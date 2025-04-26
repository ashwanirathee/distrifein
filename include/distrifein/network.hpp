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

#include <distrifein/logger.hpp>
#include <distrifein/event.hpp>
#include <distrifein/eventbus.hpp>

class TcpServer
{
public:
    TcpServer(int port, std::vector<int> peers, EventBus &eventBus);
    void startServer();

    std::vector<int> getPeers();
    int getSelfPort();

    void receiveMessage(int clientSocket);
    template <typename T>
    void sendMessage(const std::string &ip, int port, const T &message);

private:
    int self_port;
    std::vector<int> peers;
    Logger &logger = Logger::getInstance();
    EventBus &eventBus;
    std::atomic<bool> running;

    bool send_event = false;
    bool receive_event = true;

    void network_thread();
};

template <typename T>
void TcpServer::sendMessage(const std::string &ip, int port, const T &message)
{
    if (port == this->self_port)
    {
        logger.log("[P2P] Message sent to self.");
        if (receive_event)
        {
            std::vector<uint8_t> payload(reinterpret_cast<const uint8_t*>(&message), reinterpret_cast<const uint8_t*>(&message) + sizeof(message));
            Event event(EventType::P2P_MESSAGE_RECEIVED, payload);
            eventBus.publish(event);
        }
        return;
    }
    else
    {

        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0)
        {
            logger.log("[P2P] Socket creation failed.");
            return;
        }

        sockaddr_in serv_addr{};
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);

        if (inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr) <= 0)
        {
            logger.log("[P2P] Invalid address or address not supported.");
            close(sock);
            return;
        }

        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
            logger.log("[P2P] Connection Failed to " + ip + ":" + std::to_string(port));
            close(sock);
            return;
        }

        // logger.log("[Send] Connected to " + ip + ":" + std::to_string(port));
        const char *data = reinterpret_cast<const char *>(&message);
        send(sock, data, sizeof(T), 0);

        // logger.log("[P2P] Message sent to " + ip + ":" + std::to_string(port) + " with size: " + std::to_string(sizeof(T)));
        
        if (send_event)
        {   
            std::vector<uint8_t> payload;
            Event event(EventType::P2P_MESSAGE_SENT, payload);
            eventBus.publish(event);
        }
        close(sock);
    }
}

#endif