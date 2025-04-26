#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h> // for htons
#include <unistd.h>    // for close
#include <cstring>     // for memset
#include <thread>
#include <atomic>
#include <thread>

#include <distrifein/network.hpp>

TcpServer::TcpServer(int port, std::vector<int> peers, EventBus &eventBus)
    : self_port(port), peers(std::move(peers)), eventBus(eventBus), running(true)
{
}

std::vector<int> TcpServer::getPeers()
{
    return this->peers;
}

int TcpServer::getSelfPort()
{
    return this->self_port;
}

void TcpServer::startServer()
{
    std::thread(&TcpServer::network_thread, this).detach();
}

void TcpServer::receiveMessage(int clientSocket)
{
    char buffer[1024] = {0};
    ssize_t bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1);
    if (bytesRead > 0 && receive_event)
    {
        std::vector<uint8_t> receivedData(buffer, buffer + bytesRead);
        Event event(EventType::P2P_MESSAGE_RECEIVED, receivedData);
        eventBus.publish(event);
    }
    else
    {
        logger.log("[P2P] Read failed or empty.");
    }
    close(clientSocket);
}

void TcpServer::network_thread()
{
    logger.log("[P2P] Thread started.");

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        logger.log("[P2P] Socket creation failed.");
        return;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(this->self_port);

    if (bind(server_fd, (sockaddr *)&address, sizeof(address)) < 0)
    {
        logger.log("[P2P] Bind failed.");
        return;
    }

    if (listen(server_fd, 10) < 0)
    {
        logger.log("[P2P] Listen failed.");
        return;
    }

    logger.log("[P2P] Listening on port " + std::to_string(this->self_port));

    int addrlen = sizeof(address);
    while (this->running)
    {
        int clientSocket = accept(server_fd, (sockaddr *)&address, (socklen_t *)&addrlen);
        if (clientSocket < 0)
        {
            continue;
        }

        // Create and publish a CLIENT_CONNECTED event
        std::vector<uint8_t> payload = {static_cast<uint8_t>(clientSocket)}; // Store the socket as part of the event
        Event event(EventType::CLIENT_CONNECTED, payload);
        eventBus.publish(event);

        std::thread([this, clientSocket]()
                    { this->receiveMessage(clientSocket); })
            .detach();
    }

    logger.log("[P2P] Thread exiting.");
    close(server_fd);
}
