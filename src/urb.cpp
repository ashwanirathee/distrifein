#include <distrifein/urb.hpp>

UniformReliableBroadcaster::UniformReliableBroadcaster(BestEffortBroadcaster &beb, FailureDetector &fd, std::vector<int> peers, int self_port, EventBus &eventBus)
    : beb(beb), fd(fd), peers(peers), self_port(self_port), eventBus(eventBus)
{
    // correct := Π
    for (int peerPort : peers)
    {
        correct.insert(peerPort); // Initialize correct set with all peers
    }
    correct.insert(self_port); // Add self to correct set

    // delivered := ∅, make sure delivered is empty
    delivered.clear();

    // pending := ∅
    pending.clear();

    logger.log("[RB] Initialized with self_port: " + std::to_string(self_port) + ", peers: " + std::to_string(peers.size()));
    logger.log("[RB] Subscribing to events...");

    eventBus.subscribe(EventType::BEB_DELIVER_EVENT, [this](const Event &event)
                       { this->handleBEBDeliverEvent(event); });

    eventBus.subscribe(EventType::APP_SEND_EVENT, [this](const Event &event)
                       { this->broadcast(event); });

    eventBus.subscribe(EventType::PROCESS_CRASH_EVENT, [this](const Event &event)
                       { this->handleCrashEvent(event); });
}

TcpServer &UniformReliableBroadcaster::getServer()
{
    return beb.getServer();
}

void UniformReliableBroadcaster::broadcast(const Event &event)
{
    ReliableBroadcastMessage rbm;
    std::memcpy(&rbm, event.payload.data(), sizeof(ReliableBroadcastMessage));

    logger.log("[URB] Broadcasting Message!");

    // pending := pending U {[self,m]}
    pending.insert(rbm); // Add to delivered set

    // trigger < beb, broadcast ([self, m]) >
    Event event_rbs(EventType::URB_SEND_EVENT, event.payload);
    this->eventBus.publish(event_rbs);
}

void UniformReliableBroadcaster::deliver(const Event &event)
{
    logger.log("[URB] Delivering Message!");
    Event event_beb(EventType::URB_DELIVER_EVENT, event.payload);
    this->eventBus.publish(event_beb);
}

void UniformReliableBroadcaster::handleCrashEvent(const Event &event)
{

    ProcessCrashEvent processCrashEvent;
    std::memcpy(&processCrashEvent, event.payload.data(), sizeof(ProcessCrashEvent));
    int crashedProcessId = processCrashEvent.processId;

    // Remove the crashed process from the correct set
    correct.erase(crashedProcessId);
    logger.log("[URB] Process " + std::to_string(crashedProcessId) + " has crashed.");
    logger.log("[URB] Correct set: ");
    for (int peerPort : correct)
    {
        logger.log("[URB] " + std::to_string(peerPort));
    }

    this->tryDelivery();
}

void UniformReliableBroadcaster::tryDelivery()
{
    // logger.log("[URB] Trying to deliver messages...");
    // foreach ([pj,m] in pending)
    for (const auto &message : pending){
        // ack[m] := ack[m] U {pi}
        std::vector<uint8_t> payloadVector(message.message, message.message + 512);
        std::size_t hashValue = hasher(payloadVector, message.originalSenderPort);
        ack[hashValue].insert(message.senderPort); // Add to ack set
        // if (correct ack[m] and m ∉ delivered)
        if (is_subset(correct, ack[hashValue]) && delivered.find(message) == delivered.end()){
            logger.log("[URB] Message can be delivered!");
            //     delivered := delivered U {m}
            this->delivered.insert(message); // Add to delivered set
            
            //     trigger <deliver (pj, m)>
            std::vector<uint8_t> payload(sizeof(message));
            std::memcpy(payload.data(), &message, sizeof(message));
            Event event_urb(EventType::URB_DELIVER_EVENT, payload);
            this->deliver(event_urb); // Deliver to self
        } else {
            logger.log("[URB] Message not delivered yet and correct is not a subset of ack[m]");
        }

    }

}

void UniformReliableBroadcaster::handleBEBDeliverEvent(const Event &event)
{
    // logger.log("[URB] Handling BEB Deliver Event!");
    ReliableBroadcastMessage message;
    if (event.payload.size() != sizeof(ReliableBroadcastMessage))
        return;
    std::memcpy(&message, event.payload.data(), sizeof(ReliableBroadcastMessage));

    // Ensure null termination for message field
    message.message[sizeof(message.message) - 1] = '\0'; // Null-terminate the string

    // logger.log("[URB] Received message: " + std::string(message.message));
    // ack[m] := ack[m] U {pi}
    std::vector<uint8_t> payloadVector(message.message, message.message + 512);
    std::size_t hashValue = hasher(payloadVector, message.originalSenderPort);
    ack[hashValue].insert(message.senderPort); // Add to ack set
    // for (auto  it = ack[hashValue].begin(); it != ack[hashValue].end(); ++it)
    // {
    //     logger.log("[URB] ack[m] from: " + std::to_string(*it));
    // }

    // logger.log("[URB] ack[m] size: " + std::to_string(ack[hashValue].size()));
    if (pending.find(message) == pending.end())
    {
        logger.log("[URB] Message not found in pending set, adding it.");
        // pending := pending U {[pj,m]}
        pending.insert(message); // Add to pending set

        message.senderPort = this->self_port; // Set sender port to self
        message.message[sizeof(message.message) - 1] = '\0'; // Ensure null termination
        std::vector<uint8_t> payload(sizeof(message));
        std::memcpy(payload.data(), &message, sizeof(message));

        // trigger < beb, broadcast ([pj,m]) >
        Event event_rbs(EventType::URB_SEND_EVENT, payload);
        this->eventBus.publish(event_rbs); // Publish the event to the event bus
    } else {
        logger.log("[URB] Message already in pending set, not adding it.");
        tryDelivery();
    }
}
