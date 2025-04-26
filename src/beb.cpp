#include <distrifein/beb.hpp>
#include <iostream>

BestEffortBroadcaster::BestEffortBroadcaster(TcpServer &server) : server(server)
{
    setupCallbacks();
}

void BestEffortBroadcaster::broadcast(const std::string &message)
{
    std::cout << "Broadcasting-> msg: " << message << std::endl;

    // send to itself
    server.sendMessage("127.0.0.1", server.getSelfPort(), message);

    for (int peerPort : server.getPeers())
    {
        server.sendMessage("127.0.0.1", peerPort, message);
    }
}

void BestEffortBroadcaster::deliver(int src, const std::string &message)
{
    std::cout << "Deliver-> src: " << src << ", msg: " << message << std::endl;
}

void BestEffortBroadcaster::setupCallbacks()
{
    server.setClientCallback([](int clientSocket)
                             { std::cout << "New client connected: " << clientSocket << std::endl; });

    server.setMessageCallback([this](const std::string &message)
                              {
                                  this->deliver(0, message); // or extract real source if needed
                              });
}