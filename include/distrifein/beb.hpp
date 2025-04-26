#ifndef BEB_HPP
#define BEB_HPP

#include <distrifein/network.hpp>
#include <string>

class BestEffortBroadcaster
{
public:
    BestEffortBroadcaster(TcpServer &server);
    void broadcast(const std::string &message);
    void deliver(int src, const std::string &message);

    void setupCallbacks();

private:
    TcpServer &server;
};

#endif