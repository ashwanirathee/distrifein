#include <distrifein/beb.hpp>
#include <iostream>

BestEffortBroadcaster::BestEffortBroadcaster(TcpServer &server, EventBus &eventBus, bool deliveryOnReceive) : server(server), eventBus(eventBus), deliveryOnReceive(deliveryOnReceive)
{
    eventBus.subscribe(EventType::P2P_MESSAGE_RECEIVED, [this](const Event &event)
                       {
        if(this->deliveryOnReceive && event.payload.size() == sizeof(BestEffortBroadcastMessage))
        {
            BestEffortBroadcastMessage bebm;
            std::memcpy(&bebm, event.payload.data(), sizeof(BestEffortBroadcastMessage));
            this->logger.log("[BEB] Received message from P2P at BEB: " + std::string(bebm.message));
            this->deliver(bebm);
        } 

        Event event_beb(EventType::BEB_MESSAGE_RECEIVED, event.payload);
        this->eventBus.publish(event_beb); });
}

void BestEffortBroadcaster::deliver(BestEffortBroadcastMessage &message)
{
    logger.log("[BEB] Delivering-> msg: " + std::string(message.message));
}