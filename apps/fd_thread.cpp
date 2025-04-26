#include <iostream>
#include <chrono>
#include <thread>

#include "fd_thread.hpp"

void failure_detector_thread(std::atomic<bool> &running, Logger &logger)
{
    logger.log("Failure detector thread started.");
    while (running)
    {
        logger.log("Failure detector running...");
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    logger.log("Failure detector thread exiting...");
}