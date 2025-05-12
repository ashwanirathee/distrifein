#include <distrifein/urb.hpp>

UniformReliableBroadcaster::UniformReliableBroadcaster(BestEffortBroadcaster &beb, FailureDetector &fd, std::vector<int> peers, int node_id, EventBus &eventBus)
    : beb(beb), fd(fd), peerIds(peers), node_id(node_id), eventBus(eventBus)
{
    // correct := Π
    for (int peerId : peerIds)
    {
        correct.insert(peerId); // Initialize correct set with all peers
    }
    correct.insert(node_id); // Add self to correct set

    // delivered := ∅, make sure delivered is empty
    delivered.clear();

    // pending := ∅
    pending.clear();

    eventBus.subscribe(EventType::BEB_DELIVER_EVENT, [this](const Event &event)
                       { this->handleBEBDeliverEvent(event); });

    eventBus.subscribe(EventType::APP_SEND_EVENT, [this](const Event &event)
                       { this->broadcast(event); });

    eventBus.subscribe(EventType::PROCESS_CRASH_EVENT, [this](const Event &event)
                       { this->handleCrashEvent(event); });

    logger.log("[URB] Initialized with subscriptions...");
}

TcpServer &UniformReliableBroadcaster::getServer()
{
    return beb.getServer();
}

void UniformReliableBroadcaster::broadcast(const Event &event)
{
    Message rbm = deserialize_message(event.payload);
    logger.log("[URB] Broadcasting Message!");

    // pending := pending U {[self,m]}
    pending.insert(rbm); 

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
    for (int peerId : correct)
    {
        logger.log("[URB] " + std::to_string(peerId));
    }

    this->tryDelivery();
}

void UniformReliableBroadcaster::tryDelivery()
{
    logger.log("[URB] Message delivey attempt");
    
    // foreach ([pj,m] in pending)
    for (const auto &message : pending){
        // ack[m] := ack[m] U {pi}
        std::size_t size_to_hash = std::min<std::size_t>(message.payload.size(), 512);
        std::string_view payload_view(reinterpret_cast<const char*>(message.payload.data()), size_to_hash);
        std::vector<uint8_t> payloadVector(payload_view.begin(), payload_view.end());
        std::size_t hashValue = hasher(payloadVector, message.header.original_sender_id);
        ack[hashValue].insert(message.header.sender_id); // Add to ack set

        // if (correct ack[m] and m ∉ delivered)
        if (is_subset(correct, ack[hashValue]) && delivered.find(message) == delivered.end()){
            logger.log("[URB] Message is being delivered!");
            //     delivered := delivered U {m}
            this->delivered.insert(message); // Add to delivered set
            
            //     trigger <deliver (pj, m)>
            std::vector<uint8_t> payload;
            payload.resize(sizeof(MessageHeader) + message.payload.size());
    
            std::memcpy(payload.data(), &message.header, sizeof(MessageHeader));
            std::memcpy(payload.data() + sizeof(MessageHeader), message.payload.data(), message.payload.size());
            
            Event event_urb(EventType::URB_DELIVER_EVENT, payload);
            this->deliver(event_urb); // Deliver to self
        } else {
            // logger.log("[URB] Message not delivered yet and correct is not a subset of ack[m]");
            logger.log("[URB] Message can't be delivered yet!");
        }

    }

}

void UniformReliableBroadcaster::handleBEBDeliverEvent(const Event &event)
{
    // logger.log("[URB] Handling BEB Deliver Event!");
    Message message = deserialize_message(event.payload);
    if (message.header.type == MessageType::HEARTBEAT_MESSAGE) return;

    
    // logger.log("[URB] Received message: " + std::string(message.message));
    // ack[m] := ack[m] U {pi}
    std::size_t size_to_hash = std::min<std::size_t>(message.payload.size(), 512);
    std::string_view payload_view(reinterpret_cast<const char*>(message.payload.data()), size_to_hash);
    std::vector<uint8_t> payloadVector(payload_view.begin(), payload_view.end());
    std::size_t hashValue = hasher(payloadVector, message.header.original_sender_id);
    ack[hashValue].insert(message.header.sender_id); // Add to ack set
    // for (auto  it = ack[hashValue].begin(); it != ack[hashValue].end(); ++it)
    // {
    //     logger.log("[URB] ack[m] from: " + std::to_string(*it));
    // }
    logger.log("[RB] SID: " + std::to_string(message.header.sender_id) + ", Org SID: " + std::to_string(message.header.original_sender_id));

    logger.log("[RB] Current Message ID: " + std::string(message.header.message_id)+ ",size: " + std::to_string(message.payload.size()));
    for (auto &msg_id : delivered)
    {
        logger.log("[RB] Delivered Message ID: " + std::string(msg_id.header.message_id));
    }

    for (auto &msg_id : pending)
    {
        logger.log("[RB] Pending Message ID: " + std::string(msg_id.header.message_id));
    }
    

    // logger.log("[URB] ack[m] size: " + std::to_string(ack[hashValue].size()));
    if (pending.find(message) == pending.end())
    {
        logger.log("[URB] Message not in pending {}");
        // pending := pending U {[pj,m]}
        pending.insert(message); // Add to pending set

        message.header.sender_id = this->node_id; // Set sender port to self
        std::vector<uint8_t> payload;
        payload.resize(sizeof(MessageHeader) + message.payload.size());

        std::memcpy(payload.data(), &message.header, sizeof(MessageHeader));
        std::memcpy(payload.data() + sizeof(MessageHeader), message.payload.data(), message.payload.size());
    

        // trigger < beb, broadcast ([pj,m]) >
        Event event_rbs(EventType::URB_SEND_EVENT, payload);
        this->eventBus.publish(event_rbs); // Publish the event to the event bus
    } else {
        logger.log("[URB] Message in pending {}");
        tryDelivery();
    }
}
