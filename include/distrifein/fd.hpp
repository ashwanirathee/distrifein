#ifndef FD_HPP
#define FD_HPP

#include <distrifein/network.hpp>
#include <unordered_map>
#include <chrono>
#include <thread>
#include <mutex>
#include <unordered_set>

class FailureDetector
{
public:
    FailureDetector(TcpServer &server, EventBus &eventBus,  std::vector<EventType> deliver_events, std::vector<EventType> send_events, int timeoutMs = 5000, int intervalMs = 2000);

    void start();
    void stop();

private:
    TcpServer &server;
    EventBus &eventBus;
    int timeoutMs;
    int intervalMs;
    std::atomic<bool> running;
    std::unordered_map<int, std::chrono::steady_clock::time_point> lastHeartbeat;
    Logger &logger = Logger::getInstance();
    std::unordered_set<int> crashedPeerIds;
    std::vector<EventType> deliver_events;
    std::vector<EventType> send_events;

    void sendHeartbeats();
    void monitorHeartbeats();
    void handleMessage(const Event& event);
};

#endif