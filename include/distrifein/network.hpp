#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <atomic>
#include <netinet/in.h>
#include <arpa/inet.h> // for htons
#include <unistd.h>    // for close
#include <cstring>     // for memset
#include <thread>
#include <atomic>
#include <thread>
#include <unordered_set>

#include <distrifein/logger.hpp>
#include <distrifein/event.hpp>
#include <distrifein/eventbus.hpp>

class TcpServer
{
public:
    TcpServer(int port, std::vector<int> peers, EventBus &eventBus, std::vector<EventType> deliver_events, std::vector<EventType> send_events);
    void startServer();

    std::vector<int> getPeers();
    int getSelfPort();

    void deliver(int clientSocket);
    void broadcast(const Event &event);
    void sendMessage(const std::string &ip, int port, const Event &event);

private:
    int self_port;
    std::vector<int> peers;
    std::unordered_set<int> crashedPeers;
    Logger &logger = Logger::getInstance();
    EventBus &eventBus;
    std::atomic<bool> running;
    std::vector<EventType> deliver_events;
    std::vector<EventType> send_events;

    void network_thread();
};

#endif