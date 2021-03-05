//
// Created by hsyuan on 2021-03-05.
//

#include "osu.h"

int main(int argc, char *argv[])
{
    uint64_t timer_id;
    osu::TimerQueuePtr timerQueuePtr = osu::TimerQueue::create();
    timerQueuePtr->create_timer(1000, []{
        std::cout << "triggerred:" << osu::gettime_sec() << std::endl;
        std::cout << osu::format("time:%d", osu::gettime_sec()) << std::endl;
    }, 1, &timer_id);

    timerQueuePtr->run_loop();

}