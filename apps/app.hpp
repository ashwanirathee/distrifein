#ifndef APP_HPP
#define APP_HPP

#include <iostream>
#include <thread>
#include <atomic>
#include <vector>

template <typename T>
class Application{
public:
    Application(T& broadcaster);
    void run();
private:
    std::atomic<bool> running;
    Logger &logger = Logger::getInstance();
    T& broadcaster;
};


template <typename T>
Application<T>::Application(T& broadcaster)
    : broadcaster(broadcaster), running(true) {}

template <typename T>
void Application<T>::run()
{
    std::string line;
    logger.log("[App Layer] Entering input loop. Type 'exit' to quit.");
    while (running)
    {
        std::getline(std::cin, line);
        if (line.empty())
            continue;

        logger.log("[User] Input: " + line);

        if (line == "exit")
        {
            logger.log("[User] Exit requested.");
            running = false;
            break;
        }

        if constexpr (std::is_same<T, ReliableBroadcaster>::value)
        {
            // For ReliableBroadcaster, we need to create a ReliableBroadcastMessage
            ReliableBroadcastMessage msg;
            msg.senderPort = broadcaster.getServer().getSelfPort(); // Set sender port
            std::strncpy(msg.message, line.c_str(), sizeof(msg.message) - 1);
            broadcaster.broadcast(msg); // Broadcast the message using RB layer
            continue;
        }
        else if constexpr (std::is_same<T, BestEffortBroadcaster>::value)
        {
            // For BestEffortBroadcaster, we need to create a BestEffortBroadcastMessage
            BestEffortBroadcastMessage msg;
            std::strncpy(msg.message, line.c_str(), sizeof(msg.message) - 1);
            broadcaster.broadcast(msg); // Broadcast the message using BEB layer
            continue;
        }
        else
        {
            logger.log("[Error] Unknown broadcaster type.");
            continue;
        }
    }
}

#endif