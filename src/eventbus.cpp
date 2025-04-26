#include <iostream>
#include <distrifein/event.hpp>
#include <distrifein/eventbus.hpp>

void EventBus::subscribe(EventType eventType, EventCallback callback)
{
    std::lock_guard<std::recursive_mutex> lock(mtx);
    subscribers[eventType].push_back(std::move(callback));
}

void EventBus::publish(const Event &event)
{
    std::lock_guard<std::recursive_mutex> lock(mtx);
    auto it = subscribers.find(event.type);
    if (it != subscribers.end())
    {
        for (auto &callback : it->second)
        {
            callback(event);
        }
    }
}

void EventBus::rebroadcast(EventBus &bus, EventType from, EventType to)
{
    bus.subscribe(from, [&bus, to](const Event &event)
                  {
        Event new_event(to, event.payload);
        bus.publish(new_event); });
}