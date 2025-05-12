#ifndef BEB_HPP
#define BEB_HPP

#include <string>
#include <distrifein/network.h>
#include <distrifein/event.h>
#include <distrifein/eventbus.h>
#include <distrifein/message.h>

class BestEffortBroadcaster
{
public:
    BestEffortBroadcaster(TcpServer &server, EventBus &eventBus, std::vector<EventType> deliver_events, std::vector<EventType> send_events);
    void broadcast(const Event& event);
    void deliver(const Event &event);
    TcpServer &getServer();

private:
    TcpServer &server;
    EventBus &eventBus;
    std::vector<EventType> deliver_events;
    std::vector<EventType> send_events;

    Logger &logger = Logger::getInstance();
};

#endif