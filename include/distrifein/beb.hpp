#ifndef BEB_HPP
#define BEB_HPP

#include <string>
#include <distrifein/network.hpp>

class BestEffortBroadcaster
{
public:
    BestEffortBroadcaster(TcpServer &server);
    void broadcast(const std::string &message);
    void deliver(int src, const std::string &message);

    void setupCallbacks();

private:
    TcpServer &server;
    Logger &logger = Logger::getInstance();
};

#endif