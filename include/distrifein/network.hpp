#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <distrifein/logger.hpp>
#include <atomic>

class TcpServer
{
public:
    using ClientCallback = std::function<void(int clientSocket)>;
    using MessageCallback = std::function<void(const std::string &message)>;

    TcpServer(int port, std::vector<int> peers);
    void startServer();

    void setClientCallback(ClientCallback callback);
    void setMessageCallback(MessageCallback callback);

    std::vector<int> getPeers();
    int getSelfPort();

    void receiveMessage(int clientSocket);
    void sendMessage(const std::string &ip, int port, const std::string &message);

private:
    int self_port;
    std::vector<int> peers;
    Logger &logger = Logger::getInstance();
    std::atomic<bool> running;

    ClientCallback onClientConnected;  // Hook for client connections
    MessageCallback onMessageReceived; // Hook for message received

    void network_thread();
};

#endif