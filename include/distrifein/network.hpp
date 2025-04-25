#ifndef NETWORK_HPP
#define NETWORK_HPP

class TcpServer
{
public:
    int port;
    TcpServer(int port);
    void startServer();
};

#endif