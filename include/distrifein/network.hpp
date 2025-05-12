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
    TcpServer(int node_id, std::vector<int> peer_ids, EventBus &eventBus, std::vector<EventType> deliver_events, std::vector<EventType> send_events);
    void startServer();

    std::vector<int> getPeerPorts();
    int getSelfPort();

    std::vector<int> getPeerIds()
    {
        return this->peer_ids;
    }
    int getSelfId()
    {
        return this->node_id;
    }

    void deliver(int clientSocket);
    void broadcast(const Event &event);
    void sendMessage(const std::string &ip, int port, const Event &event);

private:
    int node_id;
    std::vector<int> peer_ids;

    int self_port;
    std::vector<int> peers;
    // std::unordered_set<int> crashedPeers;
    std::unordered_set<int> crashedPeerIds;
    Logger &logger = Logger::getInstance();
    EventBus &eventBus;
    std::atomic<bool> running;
    std::vector<EventType> deliver_events;
    std::vector<EventType> send_events;

    void network_thread();
};

#endif