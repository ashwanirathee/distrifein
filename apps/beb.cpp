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

using namespace std;

void application(std::atomic<bool> &running, Logger &logger, BestEffortBroadcaster &beb)
{
    std::string line;
    logger.log("[App Layer] Entering input loop. Type 'exit' to quit.");
    while (running)
    {
        std::getline(std::cin, line);
        if (line.empty())
            continue;

        logger.log("[User] Input: " + line);

        if (line == "exit")
        {
            logger.log("[User] Exit requested.");
            running = false;
            break;
        }

        beb.broadcast(line); // Broadcast the message using BEB layer
    }
}

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

    // Initialize the TcpServer and BestEffortBroadcaster
    TcpServer server(port, peers);
    BestEffortBroadcaster beb(server);
    FailureDetector detector(server);

    // Start the input thread for user interaction
    std::atomic<bool> running(true);
    std::thread inputThread(application, std::ref(running), std::ref(logger), std::ref(beb));

    // Start the server
    server.startServer();
    detector.start();
    inputThread.join();

    return 0;
}
