#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h> // for htons
#include <unistd.h>    // for close
#include <cstring>     // for memset
#include <thread>
#include <atomic>
#include <thread>

#include <distrifein/network.hpp>
#include <distrifein/message.hpp>

TcpServer::TcpServer(int port, std::vector<int> peers, EventBus &eventBus, std::vector<EventType> deliver_events, std::vector<EventType> send_events)
    : self_port(port), peers(std::move(peers)), eventBus(eventBus), running(true), deliver_events(deliver_events), send_events(send_events)
{
    logger.log("[P2P] Initialized with port: " + std::to_string(port) + ", peers: " + std::to_string(peers.size()));

    for (auto &eventType : deliver_events)
    {
        eventBus.subscribe(eventType, [this](const Event &event)
                           { // this->deliver(event);
                            logger.log("[P2P] unexpected subscription Delivering message!"); 
                            });
    }

    for (auto &eventType : send_events)
    {
        eventBus.subscribe(eventType, [this](const Event &event)
                           { this->broadcast(event); });
    }

    // this is a special case
    eventBus.subscribe(EventType::PROCESS_CRASH_EVENT, [this](const Event &event)
                       { 
                                ProcessCrashEvent processCrashEvent;
                                std::memcpy(&processCrashEvent, event.payload.data(), sizeof(ProcessCrashEvent));
                                int crashedProcessId = processCrashEvent.processId;

                                // Remove the crashed process from the correct set
                                this->crashedPeers.insert(crashedProcessId); 
                        });
}

void TcpServer::broadcast(const Event &event)
{
    if (event.type != EventType::FD_SEND_EVENT)
        logger.log("[P2P] Broadcasting Message!");
    for (int peerPort : this->getPeers())
    {
        if(this->crashedPeers.find(peerPort) != this->crashedPeers.end())
        {
            // logger.log("[P2P] Skipping crashed peer: " + std::to_string(peerPort));
            continue;
        }
        // logger.log("[BEB] Broadcasting to peer: " + std::to_string(peerPort));
        // logger.log("[BEB] Size of payload: " + std::to_string(event.payload.size()));
        this->sendMessage("127.0.0.1", peerPort, event);
    }
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

void TcpServer::deliver(int clientSocket)
{
    char buffer[1024] = {0};

    ssize_t bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1);
    if (bytesRead > 0)
    {
        // logger.log("[P2P] Received message size: " + std::to_string(bytesRead));
        std::vector<uint8_t> receivedData(buffer, buffer + bytesRead);
        Event event(EventType::P2P_DELIVER_EVENT, receivedData);
        eventBus.publish(event);
    }
    else
    {
        logger.log("[P2P] Read failed or empty.");
    }
    close(clientSocket);
}

void TcpServer::sendMessage(const std::string &ip, int port, const Event &event)
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

    // logger.log("[P2P] Sending message of size: " + std::to_string(event.payload.size()));
    // logger.log("[Send] Connected to " + ip + ":" + std::to_string(port));
    const char *data = reinterpret_cast<const char *>(event.payload.data());
    send(sock, data, event.payload.size(), 0);
    close(sock);
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

        std::thread([this, clientSocket]()
                    { this->deliver(clientSocket); })
            .detach();
    }

    logger.log("[P2P] Thread exiting.");
    close(server_fd);
}
