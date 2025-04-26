#ifndef BEB_HPP
#define BEB_HPP

#include <string>
#include <distrifein/network.hpp>
#include <distrifein/event.hpp>
#include <distrifein/eventbus.hpp>
#include <distrifein/message.hpp>

class BestEffortBroadcaster
{
public:
    BestEffortBroadcaster(TcpServer &server, EventBus &eventBus, bool deliveryOnReceive = false);
    template <typename T>
    void broadcast(T &message);
    void deliver(BestEffortBroadcastMessage &message);

    void setupCallbacks();
    TcpServer &getServer()
    {
        return server;
    }

    bool deliveryOnReceive = false; // Set to true to deliver on receive

private:
    TcpServer &server;
    EventBus &eventBus;
    Logger &logger = Logger::getInstance();
};

template <typename T>
void BestEffortBroadcaster::broadcast(T &message)
{
    logger.log("[BEB] Broadcasting-> msg: " + std::string(message.message));

    // send to itself
    server.sendMessage("127.0.0.1", server.getSelfPort(), message);

    for (int peerPort : server.getPeers())
    {
        server.sendMessage("127.0.0.1", peerPort, message);
    }
}

#endif