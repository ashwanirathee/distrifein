#include <iostream>
#include <distrifein/network.hpp>
#include <netinet/in.h>
#include <arpa/inet.h> // for htons

TcpServer::TcpServer(int port)
{
    // Initialize the server
    std::cout << "Server initialized on port: " << port << std::endl;
    this->port = port;
}

void TcpServer::startServer()
{
    // Start the server
    std::cout << "Server started." << std::endl;

    // socket file descriptor, 0
    // SOCK_STREAM for TCP
    // AF_INET for IPv4
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("socket failed");
        return;
    }

    // initializes the struct
    sockaddr_in address{};

    // set the socket options
    int opt = 1;
    int addrlen = sizeof(address);

    // set the socket options, SO_REUSEADDR and SO_REUSEPORT to reuse the port better
    // and avoid the "address already in use" error
    // AF_INET for IPv4, INADDR_ANY for any address for local binding
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(this->port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        std::cout << "bind failed" << std::endl;
        return;
    }

    if (listen(server_fd, 10) < 0)
    {
        std::cout << "listen" << std::endl;
        return;
    }
}