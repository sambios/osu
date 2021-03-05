//
// Created by hsyuan on 2021-03-05.
//

#ifndef PROJECT_OSU_DISPATCH_QUEUE_H
#define PROJECT_OSU_DISPATCH_QUEUE_H

#include <iostream>
#include <queue>
#include <deque>
#include <thread>

namespace osu {
    class DispatchQueue {
    public:
        DispatchQueue();

        ~DispatchQueue();

        void dispatch_async(std::function<void()> func);

        void dispatch_sync(std::function<void()> func);

        void dispatch_after(int msec, std::function<void()> func);

        void dispatch_flush();

        // Disable Copy and == operations.
        DispatchQueue(DispatchQueue const &) = delete;

        DispatchQueue &operator=(DispatchQueue const &) = delete;


    private:
        struct impl;
        std::unique_ptr <impl> m;
    };


    class DispatchQueueMain {
    public:
        DispatchQueueMain();

        virtual ~DispatchQueueMain();

        // Run task in main thread
        void dispatch_sync(const std::function<void(void)> &task);

        // Run task asynchronous
        void dispatch_async(const std::function<void(void)> &task);

        // Run loop in main thread.
        void runMainLoop();

        void stop();

        // Disable Copy and == operations.
        DispatchQueueMain(DispatchQueueMain const &) = delete;

        DispatchQueueMain &operator=(DispatchQueueMain const &) = delete;

    private:
        struct impl;
        std::unique_ptr <impl> m;
    };
}


#endif //PROJECT_OSU_DISPATCH_QUEUE_H
