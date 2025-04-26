#include <iostream>
#include <distrifein/fd.hpp>
#include <iostream>
#include <sstream>

FailureDetector::FailureDetector(TcpServer &server, int timeoutMs, int intervalMs)
    : server(server), timeoutMs(timeoutMs), intervalMs(intervalMs), running(false)
{
    logger.log("[FailureDetector] Initialized with timeout: " + std::to_string(timeoutMs) + "ms, interval: " + std::to_string(intervalMs) + "ms");
}

void FailureDetector::start()
{
    logger.log("[FailureDetector] Starting failure detector...");
    running = true;

    // Set message callback
    server.setMessageCallback([this](const std::string &msg)
                              { handleMessage(msg); });

    std::thread(&FailureDetector::sendHeartbeats, this).detach();
    std::thread(&FailureDetector::monitorHeartbeats, this).detach();
}

void FailureDetector::stop()
{
    running = false;
}

void FailureDetector::sendHeartbeats()
{
    while (running)
    {
        for (int peerPort : server.getPeers())
        {
            // logger.log("[FailureDetector] Sending heartbeat to peer " + std::to_string(peerPort));
            server.sendMessage("127.0.0.1", peerPort, "heartbeat:" + std::to_string(server.getSelfPort()));
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));
    }
}

void FailureDetector::monitorHeartbeats()
{
    while (running)
    {
        auto now = std::chrono::steady_clock::now();

        for (int peerPort : server.getPeers())
        {
            auto it = lastHeartbeat.find(peerPort);
            if (it == lastHeartbeat.end() || std::chrono::duration_cast<std::chrono::milliseconds>(now - it->second).count() > timeoutMs)
            {
                logger.log("[FailureDetector] Peer " + std::to_string(peerPort) + " is suspected to be down.");
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

void FailureDetector::handleMessage(const std::string &message)
{
    if (message.rfind("heartbeat:", 0) == 0)
    {

        int port = std::stoi(message.substr(10));
        // logger.log("[FailureDetector] Received heartbeat from peer " + std::to_string(port));
        lastHeartbeat[port] = std::chrono::steady_clock::now();
    }
}
