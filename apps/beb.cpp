#include <iostream>
#include <thread>
#include <atomic>

#include <distrifein/beb.hpp>
#include <distrifein/event.hpp>
#include <distrifein/fd.hpp>
#include <distrifein/network.hpp>
#include <distrifein/utils.hpp>
#include <distrifein/logger.hpp>

#include "network_thread.hpp"
#include "fd_thread.hpp"

using namespace std;

// ./build/beb 1 8000 8001,8002,8003
int main(int argc, char *argv[])
{
    // first arg will be node's id, second will be the port used by this node, third will be list of peers in form of port1,port2,...
    if (argc < 3)
    {
        cout << "We expect at least 3 arguments: node_id, port, peers" << endl;
        return 1;
    }
    int node_id = atoi(argv[1]);
    int port = atoi(argv[2]);
    vector<int> peers = split_peers_string(argv[3], ',');

    // print all this detail
    cout << "Node ID: " << node_id << endl;
    cout << "Port: " << port << endl;
    cout << "Peers: ";
    for (size_t i = 0; i < peers.size(); i++)
    {
        cout << peers[i] << " ";
    }
    cout << endl;

    std::atomic<bool> running(true);
    
    // Initialize the logger
    Logger &logger = Logger::getInstance();
    // logger.setOutputFile("log.txt");

    std::thread serverThread(network_thread, std::ref(running), std::ref(logger));
    std::thread failureDetectorThread(failure_detector_thread, std::ref(running), std::ref(logger));

    serverThread.join();
    failureDetectorThread.join();

    return 0;
}