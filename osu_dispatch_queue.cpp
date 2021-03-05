//
// Created by hsyuan on 2021-03-05.
//

#include "osu_dispatch_queue.h"

namespace osu {
    using time_point = std::chrono::steady_clock::time_point;

    struct dispatch_que_work_entry {
        explicit dispatch_que_work_entry(std::function<void()> func_)
                : func(std::move(func_)), expiry(time_point()), from_timer(false) {
        }

        dispatch_que_work_entry(std::function<void()> func_, time_point expiry_)
                : func(std::move(func_)), expiry(expiry_), from_timer(true) {
        }

        std::function<void()> func;
        time_point expiry;
        bool from_timer;
    };

    bool operator>(dispatch_que_work_entry const &lhs, dispatch_que_work_entry const &rhs) {
        return lhs.expiry > rhs.expiry;
    }

    struct DispatchQueue::impl {
        impl();

        static void dispatch_thread_proc(impl *self);

        static void timer_thread_proc(impl *self);

        std::mutex work_queue_mtx;
        std::condition_variable work_queue_cond;
        std::deque<dispatch_que_work_entry> work_queue;

        std::mutex timer_mtx;
        std::condition_variable timer_cond;
        std::priority_queue<dispatch_que_work_entry, std::vector<dispatch_que_work_entry>, std::greater<dispatch_que_work_entry> > timers;

        std::thread work_queue_thread;
        std::thread timer_thread;

        std::atomic<bool> quit;
        std::atomic<bool> work_queue_thread_started;
        std::atomic<bool> timer_thread_started;

        using work_queue_lock = std::unique_lock<decltype(work_queue_mtx)>;
        using timer_lock = std::unique_lock<decltype(timer_mtx)>;
    };

    void DispatchQueue::impl::dispatch_thread_proc(DispatchQueue::impl *self) {
        work_queue_lock work_queue_lock(self->work_queue_mtx);
        self->work_queue_cond.notify_one();
        self->work_queue_thread_started = true;

        while (self->quit == false) {
            self->work_queue_cond.wait(work_queue_lock, [&] { return !self->work_queue.empty(); });

            while (!self->work_queue.empty()) {
                auto work = self->work_queue.back();
                self->work_queue.pop_back();

                work_queue_lock.unlock();
                work.func();
                work_queue_lock.lock();
            }
        }
    }

    void DispatchQueue::impl::timer_thread_proc(DispatchQueue::impl *self) {
        timer_lock timer_lock(self->timer_mtx);
        self->timer_cond.notify_one();
        self->timer_thread_started = true;

        while (self->quit == false) {
            if (self->timers.empty()) {
                self->timer_cond.wait(timer_lock, [&] { return self->quit || !self->timers.empty(); });
            }

            while (!self->timers.empty()) {
                auto const &work = self->timers.top();
                if (self->timer_cond.wait_until(timer_lock, work.expiry, [&] { return self->quit.load(); })) {
                    break;
                }

                {
                    work_queue_lock _(self->work_queue_mtx);
                    auto where = std::find_if(self->work_queue.rbegin(),
                                              self->work_queue.rend(),
                                              [](dispatch_que_work_entry const &w) { return !w.from_timer; });
                    self->work_queue.insert(where.base(), work);
                    self->timers.pop();
                    self->work_queue_cond.notify_one();
                }
            }
        }
    }

    DispatchQueue::impl::impl()
            : quit(false), work_queue_thread_started(false), timer_thread_started(false) {
        work_queue_lock work_queue_lock(work_queue_mtx);
        timer_lock timer_lock(timer_mtx);

        work_queue_thread = std::thread(dispatch_thread_proc, this);
        timer_thread = std::thread(timer_thread_proc, this);

        work_queue_cond.wait(work_queue_lock, [this] { return work_queue_thread_started.load(); });
        timer_cond.wait(timer_lock, [this] { return timer_thread_started.load(); });
    }

    DispatchQueue::DispatchQueue() : m(new impl) {}

    DispatchQueue::~DispatchQueue() {
        dispatch_async([this] { m->quit = true; });
        m->work_queue_thread.join();

        {
            impl::timer_lock _(m->timer_mtx);
            m->timer_cond.notify_one();
        }

        m->timer_thread.join();
    }

    void DispatchQueue::dispatch_async(std::function<void()> func) {
        impl::work_queue_lock _(m->work_queue_mtx);
        m->work_queue.push_front(dispatch_que_work_entry(func));
        m->work_queue_cond.notify_one();
    }

    void DispatchQueue::dispatch_sync(std::function<void()> func) {
        std::mutex sync_mtx;
        impl::work_queue_lock work_queue_lock(sync_mtx);
        std::condition_variable sync_cond;
        std::atomic<bool> completed(false);

        {
            impl::work_queue_lock _(m->work_queue_mtx);
            m->work_queue.push_front(dispatch_que_work_entry(func));
            m->work_queue.push_front(dispatch_que_work_entry([&] {
                std::unique_lock<std::mutex> sync_cb_lock(sync_mtx);
                completed = true;
                sync_cond.notify_one();
            }));

            m->work_queue_cond.notify_one();
        }

        sync_cond.wait(work_queue_lock, [&] { return completed.load(); });

    }

    void DispatchQueue::dispatch_after(int msec, std::function<void()> func) {
        impl::timer_lock _(m->timer_mtx);
        m->timers.push(
                dispatch_que_work_entry(func, std::chrono::steady_clock::now() + std::chrono::milliseconds(msec)));
        m->timer_cond.notify_one();
    }

    void DispatchQueue::dispatch_flush() {
        dispatch_sync([] {});
    }

///////////////////////////////////////////////////////////////////////////
// DispatchQueueMain

    struct DispatchQueueMain::impl {
        impl();

        std::mutex work_queue_mtx_;
        std::condition_variable work_queue_cond_;
        std::deque<dispatch_que_work_entry> work_queue_;

        std::atomic<bool> stopped_;
        std::atomic<bool> work_queue_started_;

        using work_queue_lock = std::unique_lock<decltype(work_queue_mtx_)>;
    };

    DispatchQueueMain::impl::impl():stopped_(false), work_queue_started_(false) {

    }

    DispatchQueueMain::DispatchQueueMain():m(new impl){
        //TODO
    }

    DispatchQueueMain::~DispatchQueueMain() {
        stop();
    }


    // Run Task in main thread
    void DispatchQueueMain::dispatch_sync(const std::function<void(void)> &task) {

        std::mutex sync_mtx;
        impl::work_queue_lock wlock(sync_mtx);
        std::condition_variable sync_cond;
        std::atomic<bool> completed(false);
        {
            impl::work_queue_lock _(m->work_queue_mtx_);
            m->work_queue_.push_front(dispatch_que_work_entry(task));
            m->work_queue_.push_front(dispatch_que_work_entry([&] {
                std::unique_lock<std::mutex> sync_cb_lock(sync_mtx);
                completed = true;
                sync_cond.notify_one();
            }));

            m->work_queue_cond_.notify_one();
        }

        sync_cond.wait(wlock, [&] { return completed.load();});
    }

    void DispatchQueueMain::dispatch_async(const std::function<void(void)> &task) {
        impl::work_queue_lock _(m->work_queue_mtx_);
        m->work_queue_.push_front(dispatch_que_work_entry(task));
        m->work_queue_cond_.notify_one();
    }

    // Run loop in main thread.
    void DispatchQueueMain::runMainLoop()
    {
        impl::work_queue_lock wlock(m->work_queue_mtx_);
        m->work_queue_cond_.notify_one();
        m->work_queue_started_ = true;

        while(!m->stopped_) {
            m->work_queue_cond_.wait(wlock, [&]{ return !m->work_queue_.empty();});
            while(!m->work_queue_.empty()) {
                auto work = m->work_queue_.back();
                m->work_queue_.pop_back();
                wlock.unlock();
                work.func();
                wlock.lock();
            }
        }
    }

    void DispatchQueueMain::stop() {
        dispatch_async([this]{
            m->stopped_ = true;});
    }
}