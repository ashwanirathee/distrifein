#include <iostream>
#include <chrono>
#include <thread>

#include "network_thread.hpp"

void network_thread(std::atomic<bool>& running, Logger &logger)
{
    logger.log("Network thread started.");
    while (running) {
        logger.log("Server running...");
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    logger.log("Network thread exiting...");
}