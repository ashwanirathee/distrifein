#ifndef FD_HPP
#define FD_HPP

#include <distrifein/network.hpp>
#include <unordered_map>
#include <chrono>
#include <thread>
#include <mutex>

class FailureDetector
{
public:
    FailureDetector(TcpServer &server, EventBus &eventBus, int timeoutMs = 5000, int intervalMs = 2000);

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

    void sendHeartbeats();
    void monitorHeartbeats();
    void handleMessage(const std::string &message);
};

#endif