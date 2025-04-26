#include <iostream>
#include <thread>
#include <atomic>
#include <vector>
#include <string>

#include <distrifein/beb.hpp>
#include <distrifein/event.hpp>
#include <distrifein/fd.hpp>
#include <distrifein/network.hpp>
#include <distrifein/utils.hpp>
#include <distrifein/logger.hpp>
#include <distrifein/rb.hpp>
#include <distrifein/eventbus.hpp>
#include <distrifein/event.hpp>

#include "app.hpp"

using namespace std;

bool debug = false; // Set to false to disable debug messages

// ./build/beb 1 8000 8001,8002,8003
int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        cout << "Usage: ./beb <node_id> <port> <peer_ports_comma_separated>\n";
        return 1;
    }

    int node_id = atoi(argv[1]);
    int port = atoi(argv[2]);
    vector<int> peers = split_peers_string(argv[3], ',');

    if (debug)
    {
        cout << "Node ID: " << node_id << "\nPort: " << port << "\nPeers: ";
        for (int p : peers)
            cout << p << " ";
        cout << endl;
    }

    // Initialize logger
    Logger &logger = Logger::getInstance(); // Get the singleton logger
    EventBus eventBus;                      // Just create it normally (no heap allocation)
    TcpServer server(port, peers, eventBus);
    BestEffortBroadcaster beb(server, eventBus);
    FailureDetector detector(server, eventBus);
    
    bool deliveryOnReceive = true;
    ReliableBroadcaster rb(beb, detector, peers, port, eventBus, deliveryOnReceive);
    Application app(rb); // Create the application with the ReliableBroadcaster

    server.startServer();
    detector.start();
    app.run(); 

    return 0;
}
