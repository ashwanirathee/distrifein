#include <functional>
#include <unordered_map>
#include <vector>
#include <mutex>

// EventBus.hpp
#pragma once
#include <distrifein/event.h>
#include <distrifein/eventbus.h>
#include <distrifein/logger.h>

class EventBus
{
public:
    using EventCallback = std::function<void(const Event &)>;

    void subscribe(EventType eventType, EventCallback callback);
    void publish(const Event &event);
    void rebroadcast(EventBus &bus, EventType from, EventType to);

private:
    std::unordered_map<EventType, std::vector<EventCallback>> subscribers;
    std::recursive_mutex mtx;
    Logger &logger = Logger::getInstance();
};
