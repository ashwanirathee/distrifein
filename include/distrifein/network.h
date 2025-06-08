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
#include <map>

#include <distrifein/logger.h>
#include <distrifein/event.h>
#include <distrifein/eventbus.h>

class TcpServer
{
public:
    TcpServer(int node_id, std::vector<int> peer_ids, EventBus &eventBus, std::vector<EventType> deliver_events, std::vector<EventType> send_events, std::string peer_list_path);
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
    void process_peer_list(std::string peer_list_path);
private:
    int node_id;
    std::vector<int> peer_ids;
    std::map<int, std::pair<std::string, int>> peer_info;

    int self_port;
    std::vector<int> peers;
    std::unordered_set<int> crashedPeerIds;
    Logger &logger = Logger::getInstance();
    EventBus &eventBus;
    std::atomic<bool> running;
    std::vector<EventType> deliver_events;
    std::vector<EventType> send_events;

    void network_thread();
};

#endif