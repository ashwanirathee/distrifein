#include <iostream>
#include <thread>
#include <atomic>
#include <vector>
#include <string>

#include <distrifein/beb.h>
#include <distrifein/event.h>
#include <distrifein/fd.h>
#include <distrifein/network.h>
#include <distrifein/utils.h>
#include <distrifein/logger.h>
#include <distrifein/rb.h>
#include <distrifein/eventbus.h>
#include <distrifein/event.h>
#include <distrifein/urb.h>

#include "app.h"

using namespace std;

bool debug = false; // Set to false to disable debug messages

// ./build/beb 1 8000 8001,8002,8003
int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        cout << "Usage: ./beb <node_id> <peer_node_ids_comma_separated> <test_type>\n";
        return 1;
    }

    int node_id = atoi(argv[1]);
    vector<int> peer_ids = split_peers_string(argv[2], ',');
    int test_type = atoi(argv[3]);

    if (debug)
    {
        cout << "Node ID: " << node_id << "\nPort: " << id_to_port[node_id] << "\nPeers: ";
        for (int p : peer_ids)
            cout << p << " ";
        cout << endl;
    }

    // Initialize logger
    Logger &logger = Logger::getInstance(); // Get the singleton logger
    EventBus eventBus;                      // Just create it normally (no heap allocation)

    if (test_type == 0)
    {
        TcpServer server(node_id, peer_ids, eventBus, {}, {EventType::BEB_SEND_EVENT, EventType::FD_SEND_EVENT});
        BestEffortBroadcaster beb(server, eventBus, {EventType::P2P_DELIVER_EVENT}, {EventType::APP_SEND_EVENT});
        Application app(beb, eventBus, node_id); // Create the application with the ReliableBroadcaster
        server.startServer();
        app.run();
    }
    else if (test_type == 1)
    {
        TcpServer server(node_id, peer_ids, eventBus, {}, {EventType::BEB_SEND_EVENT, EventType::FD_SEND_EVENT});
        BestEffortBroadcaster beb(server, eventBus, {EventType::P2P_DELIVER_EVENT}, {EventType::RB_SEND_EVENT});
        FailureDetector detector(server, eventBus, {EventType::P2P_DELIVER_EVENT}, {});
        ReliableBroadcaster rb(beb, detector, peer_ids, eventBus, node_id);
        Application app(rb, eventBus, node_id); // Create the application with the ReliableBroadcaster

        server.startServer();
        detector.start();
        app.run();
    } else if(test_type == 2)
    {
        TcpServer server(node_id, peer_ids, eventBus, {}, {EventType::BEB_SEND_EVENT, EventType::FD_SEND_EVENT});
        BestEffortBroadcaster beb(server, eventBus, {EventType::P2P_DELIVER_EVENT}, {EventType::URB_SEND_EVENT});
        FailureDetector detector(server, eventBus, {EventType::P2P_DELIVER_EVENT}, {});
        UniformReliableBroadcaster urb(beb, detector, peer_ids, node_id, eventBus);
        Application app(urb, eventBus, node_id); // Create the application with the ReliableBroadcaster

        server.startServer();
        detector.start();
        app.run();
    }
    else
    {
        cout << "Invalid test type. Use 0 for BEB or 1 for RB.\n";
        return 1;

    }

    return 0;
}
