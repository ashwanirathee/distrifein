#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h> // for htons
#include <unistd.h>    // for close
#include <cstring>     // for memset
#include <thread>
#include <atomic>
#include <thread>

#include <distrifein/network.h>
#include <distrifein/message.h>
#include <distrifein/utils.h>
TcpServer::TcpServer(int node_id, std::vector<int> peer_ids, EventBus &eventBus, std::vector<EventType> deliver_events, std::vector<EventType> send_events, std::string peer_list_path)
    : node_id(node_id), peer_ids(peer_ids), eventBus(eventBus), running(true), deliver_events(deliver_events), send_events(send_events)
{
    this->process_peer_list(peer_list_path);
    this->self_port = peer_info[node_id].second;
    // logger.log("[P2P] Initialized with port: " + std::to_string(this->self_port) + ", peers: " + std::to_string(this->peer_ids.size()));
    if (peer_info[node_id].first == "127.0.0.1"){
        logger.log("[P2P] Self Id: " + std::to_string(this->node_id) + "Self Ip: localhost"  + peer_info[node_id].first +  ", Self Port: " + std::to_string(this->self_port));
    } else {
        logger.log("[P2P] Id: " + std::to_string(this->node_id) + ", Ip: "  + peer_info[node_id].first +  ", Port: " + std::to_string(this->self_port));
    }

    for (int peer_id : peer_ids)
    {
        this->peers.push_back(peer_info[peer_id].second);
        // logger.log("[P2P] Peer:" + std::to_string(peer_id) + " at port: " + std::to_string(id_to_port[peer_id]));
    }

    for (auto &eventType : deliver_events)
    {
        eventBus.subscribe(eventType, [this](const Event &event) { // this->deliver(event);
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
                                this->crashedPeerIds.insert(crashedProcessId); });
}

void TcpServer::process_peer_list(std::string peer_list_path){
    std::ifstream peer_list(peer_list_path);
    std::string line;
    int node_id, port;
    std::string ip;
    while (std::getline(peer_list, line))
    {
        // std::cout << line << std::endl;
        if (line[0] == '#'){
            // std::cout << "Skipping comment" << std::endl;
            continue;
        }
        std::stringstream line_content(line);
        line_content >> node_id >> ip >> port;
        // std::cout << "Node ID: " << node_id << " IP: " << ip << " Port: " << port << std::endl;
        this->peer_info[node_id] = std::make_pair(ip, port);
    }
    peer_list.close();
}

void TcpServer::broadcast(const Event &event)
{
    if (event.type != EventType::FD_SEND_EVENT)
        logger.log("[P2P] Broadcasting Message!");
    for (int peerId : this->getPeerIds())
    {
        if (this->crashedPeerIds.find(peerId) != this->crashedPeerIds.end())
        {
            logger.log("[P2P] Skipping crashed peer: " + std::to_string(peerId));
            continue;
        }
        // logger.log("[BEB] Broadcasting to peer: " + std::to_string(peerId));
        // logger.log("[BEB] Size of payload: " + std::to_string(event.payload.size()));
        this->sendMessage(peer_info[peerId].first, peer_info[peerId].second, event);
    }
}

std::vector<int> TcpServer::getPeerPorts()
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
    std::vector<uint8_t> receivedData;
    char buffer[1024];

    ssize_t bytesRead;
    while ((bytesRead = read(clientSocket, buffer, sizeof(buffer))) > 0)
    {
        receivedData.insert(receivedData.end(), buffer, buffer + bytesRead);
    }

    if (!receivedData.empty())
    {
        // if (receivedData.size() > 85)
        //     logger.log("[P2P] Received total size: " + std::to_string(receivedData.size()));
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
    // logger.log("[P2P] Sending message to " + ip + ":" + std::to_string(port));
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
    logger.log("[P2P] Starting tcp server...");

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
