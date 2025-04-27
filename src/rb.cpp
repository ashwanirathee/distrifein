#include <distrifein/rb.hpp>

ReliableBroadcaster::ReliableBroadcaster(BestEffortBroadcaster &beb, FailureDetector &fd, std::vector<int> peers, int self_port, EventBus &eventBus)
    : beb(beb), fd(fd), peers(peers), self_port(self_port), eventBus(eventBus)
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
    correct.insert(self_port); // Add self to correct set

    logger.log("[RB] Initialized with self_port: " + std::to_string(self_port) + ", peers: " + std::to_string(peers.size()));
    logger.log("[RB] Subscribing to events...");

    eventBus.subscribe(EventType::BEB_DELIVER_EVENT, [this](const Event &event)
                       { this->handleBEBDeliverEvent(event); });

    eventBus.subscribe(EventType::APP_SEND_EVENT, [this](const Event &event)
                       { this->broadcast(event); });

    eventBus.subscribe(EventType::PROCESS_CRASH_EVENT, [this](const Event &event)
                       { this->handleCrashEvent(event); });
}

TcpServer &ReliableBroadcaster::getServer()
{
    return beb.getServer();
}
void ReliableBroadcaster::broadcast(const Event &event)
{
    ReliableBroadcastMessage rbm;
    std::memcpy(&rbm, event.payload.data(), sizeof(ReliableBroadcastMessage));

    logger.log("[RB] Broadcasting Message!");

    // delivered := delivered U {m}
    delivered.insert(rbm); // Add to delivered set

    // trigger < deliver (self, m) >
    this->deliver(event); // Deliver to self

    // trigger < beb, broadcast ([self, m]) >
    Event event_rbs(EventType::RB_SEND_EVENT, event.payload);
    this->eventBus.publish(event_rbs);
}

void ReliableBroadcaster::deliver(const Event &event)
{
    Event event_beb(EventType::RB_DELIVER_EVENT, event.payload);
    this->eventBus.publish(event_beb);
}

void ReliableBroadcaster::handleCrashEvent(const Event &event)
{

    ProcessCrashEvent processCrashEvent;
    std::memcpy(&processCrashEvent, event.payload.data(), sizeof(ProcessCrashEvent));
    int crashedProcessId = processCrashEvent.processId;

    // Remove the crashed process from the correct set
    correct.erase(crashedProcessId);
    logger.log("[RB] Process " + std::to_string(crashedProcessId) + " has crashed.");
    logger.log("[RB] Correct set: ");
    for (int peerPort : correct)
    {
        logger.log("[RB] " + std::to_string(peerPort));
    }

    // print all the messages in from[pi]
    logger.log("[RB] Messages from crashed process:" + std::to_string(from[crashedProcessId].size()));

    // forall [pj, m] in from[pi] do
    //     trigger <beb, broadcast([pj, m])>
    for (auto &message : from[crashedProcessId])
    {
        ReliableBroadcastMessage rbm = message;
        rbm.senderPort = this->self_port;
        rbm.message[sizeof(rbm.message) - 1] = '\0'; // Ensure null termination

        std::vector<uint8_t> payload(sizeof(rbm));
        std::memcpy(payload.data(), &rbm, sizeof(rbm));

        // trigger <beb, broadcast([pj, m])>
        Event event_rbs(EventType::RB_SEND_EVENT, payload);
        this->eventBus.publish(event_rbs);
        logger.log("[RB] Broadcasting message from crashed process " + std::to_string(crashedProcessId) + ": " + std::string(message.message));
    }
}

void ReliableBroadcaster::handleBEBDeliverEvent(const Event &event)
{
    ReliableBroadcastMessage message;
    if (event.payload.size() != sizeof(ReliableBroadcastMessage)) return;
    std::memcpy(&message, event.payload.data(), sizeof(ReliableBroadcastMessage));

    // Ensure null termination for message field
    message.message[sizeof(message.message) - 1] = '\0'; // Null-terminate the string
    
    // if m ∉ delivered then
    if (delivered.find(message) == delivered.end())
    {
        // delivered := delivered U {m}
        delivered.insert(message); // Add to delivered set

        // trigger <deliver (pj, m)>
        this->deliver(event); // Deliver to self

        // if pi ∉ correct then
        if (correct.find(message.senderPort) == correct.end())
        {
            // trigger <beb, broadcast([pj, m])>
            Event event_beb(EventType::RB_SEND_EVENT, event.payload);
            this->eventBus.publish(event_beb); // Publish the event to the event bus
        }
        else
        {
            // from[pi] := from[pi] U {[pj, m]}
            from[message.senderPort].insert(message); // Add to from[pi]
        }
        // logger.log("[RB] Delivering Message: pi: " + std::to_string(message.senderPort) + ", pj: " + std::to_string(message.originalSenderPort) + ", msg: " + std::string(message.message));
    }
    else
    {
        logger.log("[RB] Message already delivered, ignoring.");
    }
}
