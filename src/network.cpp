#include <iostream>
#include <distrifein/network.hpp>
#include <netinet/in.h>
#include <arpa/inet.h> // for htons
#include <unistd.h>    // for close
#include <cstring>     // for memset
#include <thread>
#include <atomic>
#include <thread>

TcpServer::TcpServer(int port, std::vector<int> peers)
    : self_port(port), peers(std::move(peers)), running(true)
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

void TcpServer::setClientCallback(ClientCallback callback)
{
    onClientConnected = std::move(callback);
}

void TcpServer::setMessageCallback(MessageCallback callback)
{
    onMessageReceived = std::move(callback);
}

void TcpServer::startServer()
{
    std::thread netThread(&TcpServer::network_thread, this);
    netThread.join();
}

void TcpServer::receiveMessage(int clientSocket)
{
    char buffer[1024] = {0};
    ssize_t bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1);
    if (bytesRead > 0)
    {
        std::string message(buffer, bytesRead);
        // this->logger.log("[Received] " + message);

        // Call the user-defined message received callback
        if (onMessageReceived)
        {
            onMessageReceived(message); // Trigger the callback
        }
    }
    else
    {
        logger.log("[Client] Read failed or empty.");
    }
    close(clientSocket);
}

void TcpServer::network_thread()
{
    logger.log("[Network] Thread started.");

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        logger.log("[Network] Socket creation failed.");
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
        logger.log("[Network] Bind failed.");
        return;
    }

    if (listen(server_fd, 10) < 0)
    {
        logger.log("[Network] Listen failed.");
        return;
    }

    logger.log("[Network] Listening on port " + std::to_string(this->self_port));

    int addrlen = sizeof(address);
    while (this->running)
    {
        int clientSocket = accept(server_fd, (sockaddr *)&address, (socklen_t *)&addrlen);
        if (clientSocket < 0)
        {
            continue;
        }

        // Call the user-defined callback when a client connects
        if (onClientConnected)
        {
            onClientConnected(clientSocket); // Trigger the callback
        }

        std::thread([this, clientSocket]()
                    { this->receiveMessage(clientSocket); })
            .detach();
    }

    logger.log("[Network] Thread exiting.");
    close(server_fd);
}

void TcpServer::sendMessage(const std::string &ip, int port, const std::string &message)
{
    if (port == this->self_port)
    {
        // logger.log("[Send] Sending to self: " + message);

        // Call the user-defined message received callback
        if (onMessageReceived)
        {
            onMessageReceived(message); // Trigger the callback
        }
        return;
    }
    else
    {

        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0)
        {
            logger.log("[Send] Socket creation failed.");
            return;
        }

        sockaddr_in serv_addr{};
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);

        if (inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr) <= 0)
        {
            logger.log("[Send] Invalid address or address not supported.");
            close(sock);
            return;
        }

        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
            logger.log("[Send] Connection Failed to " + ip + ":" + std::to_string(port));
            close(sock);
            return;
        }

        // logger.log("[Send] Connected to " + ip + ":" + std::to_string(port));
        send(sock, message.c_str(), message.length(), 0);
        // logger.log("[Send] Message sent to " + ip + ":" + std::to_string(port));
        close(sock);
    }
}
