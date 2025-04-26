#include <distrifein/rb.hpp>

ReliableBroadcaster::ReliableBroadcaster(BestEffortBroadcaster &beb, FailureDetector &fd, std::vector<int> peers, int self_port, EventBus &eventBus, bool deliveryOnReceive)
    : beb(beb), fd(fd), peers(peers), self_port(self_port), eventBus(eventBus), deliveryOnReceive(deliveryOnReceive)
{
    // delivered := ∅, make sure delivered is empty
    delivered.clear();

    // forall pi in Π do from[pi] := ∅
    for (int peerPort : peers)
    {
        from[peerPort].clear(); // Initialize from[pi] to empty set
    }

    // correct := Π
    for (int peerPort : peers)
    {
        correct.insert(peerPort); // Initialize correct set with all peers
    }

    logger.log("[RB] Initialized with self_port: " + std::to_string(self_port) + ", peers: " + std::to_string(peers.size()));
    logger.log("[RB] Subscribing to events...");
    eventBus.subscribe(EventType::BEB_MESSAGE_RECEIVED, [this](const Event &event)
                       {
                           if (event.payload.size() >= sizeof(ReliableBroadcastMessage))
                           {
                                std::string message(event.payload.begin(), event.payload.end());
                                ReliableBroadcastMessage rbm;
                                std::memcpy(&rbm, event.payload.data(), sizeof(ReliableBroadcastMessage));
                                logger.log("[RB] Received message from " + std::to_string(rbm.senderPort) + ": " + std::string(rbm.message));
                            } 
                        });
}

TcpServer &ReliableBroadcaster::getServer()
{
    return beb.getServer();
}
void ReliableBroadcaster::broadcast(ReliableBroadcastMessage &msg)
{
    logger.log("[RB] Broadcasting-> msg: " + std::string(msg.message));

    // delivered := delivered U {m}
    delivered.insert(msg); // Add to delivered set
    // trigger < deliver (self, m) >
    this->deliver(self_port, msg); // Deliver to self
    // trigger < beb, broadcast ([self, m]) >
    beb.broadcast(msg); // Broadcast to peers
}

void ReliableBroadcaster::deliver(int src, ReliableBroadcastMessage &message)
{
    // if m ∉ delivered then
    if (delivered.find(message) == delivered.end())
    {
        // delivered := delivered U {m}
        delivered.insert(message); // Add to delivered set

        // trigger <deliver (pj, m)>
        logger.log("[RB] Delivering-> src: " + std::to_string(src) + ", msg: " + std::string(message.message));

        // if pi ∉ correct then
        if (correct.find(src) == correct.end())
        {
            // trigger <beb, broadcast([pj, m])>
            beb.broadcast(message); // Broadcast to peers
        }
        else
        {
            // from[pi] := from[pi] U {[pj, m]}
            from[src].insert(message); // Add to from[pi]
        }
    }
}