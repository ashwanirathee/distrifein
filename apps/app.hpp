#ifndef APP_HPP
#define APP_HPP

#include <iostream>
#include <thread>
#include <atomic>
#include <vector>

enum class BroadcasterType
{
    ReliableBroadcast,
    BestEffortBroadcast,
    Unknown
};

template <typename T>
class Application
{
public:
    Application(T &broadcaster, EventBus &eventBus);
    void decode(const Event &event, BroadcasterType type);
    void run();

private:
    std::atomic<bool> running;
    Logger &logger = Logger::getInstance();
    T &broadcaster;
    EventBus &eventBus;
    BroadcasterType broadcasterType; // <-- Add this
};

template <typename T>
Application<T>::Application(T &broadcaster, EventBus &eventBus)
    : broadcaster(broadcaster), eventBus(eventBus), running(true)
{
    logger.log("[App] Initialized with broadcaster type: " + std::string(typeid(T).name()));
    logger.log("[App] Subscribing to events...");
    if (std::is_same<T, ReliableBroadcaster>::value)
    {
        broadcasterType = BroadcasterType::ReliableBroadcast;
        eventBus.subscribe(EventType::RB_DELIVER_EVENT, [this](const Event &event)
                           { this->decode(event, broadcasterType); });
    }
    else if (std::is_same<T, BestEffortBroadcaster>::value)
    {
        broadcasterType = BroadcasterType::BestEffortBroadcast;
        eventBus.subscribe(EventType::BEB_DELIVER_EVENT, [this](const Event &event)
                           { this->decode(event, broadcasterType); });
    }
    else
    {
        logger.log("[Error] Unknown broadcaster type.");
    }
}

template <typename T>
void Application<T>::decode(const Event &event, BroadcasterType type)
{
    if (type == BroadcasterType::ReliableBroadcast)
    {
        ReliableBroadcastMessage message;
        std::memcpy(&message, event.payload.data(), sizeof(ReliableBroadcastMessage));
        message.message[sizeof(message.message) - 1] = '\0'; // Ensure null termination
        logger.log("[App] Received message: " + std::string(message.message));
    }
    else if (type == BroadcasterType::BestEffortBroadcast)
    {
        BestEffortBroadcastMessage message;
        std::memcpy(&message, event.payload.data(), sizeof(BestEffortBroadcastMessage));
        message.message[sizeof(message.message) - 1] = '\0'; // Ensure null termination
        logger.log("[App] Received message: " + std::string(message.message));
    }
}

template <typename T>
void Application<T>::run()
{
    std::string line;
    logger.log("[App] Entering input loop. Type 'exit' to quit.");
    while (running)
    {
        std::getline(std::cin, line);
        if (line.empty())
            continue;

        logger.log("[App] Input: " + line);

        if (line == "exit")
        {
            logger.log("[App] Exit requested.");
            running = false;
            break;
        }

        if constexpr (std::is_same<T, ReliableBroadcaster>::value)
        {
            // For ReliableBroadcaster, we need to create a ReliableBroadcastMessage
            ReliableBroadcastMessage msg;
            msg.senderPort = broadcaster.getServer().getSelfPort(); // Set sender port
            msg.originalSenderPort = msg.senderPort;                // Set original sender port
            msg.message[0] = '\0';                                  // Initialize message buffer
            std::strncpy(msg.message, line.c_str(), sizeof(msg.message) - 1);
            msg.message[sizeof(msg.message) - 1] = '\0'; // Ensure null termination

            std::vector<uint8_t> payload(sizeof(msg));
            std::memcpy(payload.data(), &msg, sizeof(msg));
            Event event(EventType::APP_SEND_EVENT, payload);
            eventBus.publish(event);
            continue;
        }
        else if constexpr (std::is_same<T, BestEffortBroadcaster>::value)
        {
            // For BestEffortBroadcaster, we need to create a BestEffortBroadcastMessage
            BestEffortBroadcastMessage msg;
            std::strncpy(msg.message, line.c_str(), sizeof(msg.message) - 1);
            std::vector<uint8_t> payload(sizeof(msg));
            std::memcpy(payload.data(), &msg, sizeof(msg));
            Event event(EventType::APP_SEND_EVENT, payload);
            eventBus.publish(event);
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