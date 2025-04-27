#include <distrifein/beb.hpp>
#include <iostream>

BestEffortBroadcaster::BestEffortBroadcaster(TcpServer &server, EventBus &eventBus,
                                             std::vector<EventType> deliver_events, std::vector<EventType> send_events)
    : server(server), eventBus(eventBus), deliver_events(deliver_events), send_events(send_events)
{
    for (auto &eventType : deliver_events)
    {
        eventBus.subscribe(eventType, [this](const Event &event)
                           { this->deliver(event); });
    }

    for (auto &eventType : send_events)
    {
        eventBus.subscribe(eventType, [this](const Event &event)
                           { this->broadcast(event); });
    }
}

void BestEffortBroadcaster::broadcast(const Event &event)
{

    logger.log("[BEB] Broadcasting Message!");

    Event event_selfbeb(EventType::BEB_DELIVER_EVENT, event.payload);
    this->eventBus.publish(event_selfbeb);

    Event event_beb(EventType::BEB_SEND_EVENT, event.payload);
    this->eventBus.publish(event_beb);
}

void BestEffortBroadcaster::deliver(const Event &event)
{
    if (event.payload.size() < 30)
    {
        // logger.log("[BEB] Those are heartbeat messages.");
        return;
    }
    logger.log("[BEB] Delivering Message!");
    Event event_beb(EventType::BEB_DELIVER_EVENT, event.payload);
    this->eventBus.publish(event_beb);
}

TcpServer &BestEffortBroadcaster::getServer()
{
    return server;
}