#include <distrifein/rb.hpp>
#include <distrifein/utils.hpp>

ReliableBroadcaster::ReliableBroadcaster(BestEffortBroadcaster &beb, FailureDetector &fd, std::vector<int> peer_ids, EventBus &eventBus, int node_id)
    : beb(beb), fd(fd), peer_ids(peer_ids), eventBus(eventBus), node_id(node_id)
{
    // delivered := ∅, make sure delivered is empty
    delivered.clear();

    // forall pi in Π do from[pi] := ∅
    for (int peer_id : peer_ids)
    {
        from[peer_id].clear(); // Initialize from[pi] to empty set
    }

    // correct := Π
    for (int peer_id : peer_ids)
    {
        correct.insert(peer_id); // Initialize correct set with all peers
    }
    correct.insert(node_id); // Add self to correct set

    eventBus.subscribe(EventType::BEB_DELIVER_EVENT, [this](const Event &event)
                       { this->handleBEBDeliverEvent(event); });

    eventBus.subscribe(EventType::APP_SEND_EVENT, [this](const Event &event)
                       { this->broadcast(event); });

    eventBus.subscribe(EventType::PROCESS_CRASH_EVENT, [this](const Event &event)
                       { this->handleCrashEvent(event); });

    logger.log("[RB] Initialized with subscriptions...");
}

TcpServer &ReliableBroadcaster::getServer()
{
    return beb.getServer();
}
void ReliableBroadcaster::broadcast(const Event &event)
{
    logger.log("[RB] Broadcasting Message!");
    Message message = deserialize_message(event.payload);

    // delivered := delivered U {m}
    delivered.insert(std::string(message.header.message_id)); // Add to delivered set

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
    for (int peer_id : correct)
    {
        logger.log("[RB] " + std::to_string(peer_id));
    }

    // print all the messages in from[pi]
    logger.log("[RB] Messages from crashed process:" + std::to_string(from[crashedProcessId].size()));

    // forall [pj, m] in from[pi] do
    //     trigger <beb, broadcast([pj, m])>
    for (auto &message : from[crashedProcessId])
    {
        Message rbm = message;
        rbm.header.sender_id = this->node_id;

        std::vector<uint8_t> payload;
        payload.resize(sizeof(MessageHeader) + message.payload.size());

        std::memcpy(payload.data(), &rbm.header, sizeof(MessageHeader));
        std::memcpy(payload.data() + sizeof(MessageHeader), message.payload.data(), message.payload.size());
    
        logger.log("[RB] Broadcasting message from crashed process " + std::to_string(crashedProcessId) + ": " + std::string(reinterpret_cast<char *>(rbm.payload.data())));    

        // trigger <beb, broadcast([pj, m])>
        Event event_rbs(EventType::RB_SEND_EVENT, payload);
        this->eventBus.publish(event_rbs);
    }
}

void ReliableBroadcaster::handleBEBDeliverEvent(const Event &event)
{
    // logger.log("[RB] Handling BEB Deliver Event");

    if (event.payload.size() < sizeof(MessageHeader)) return;
    Message message = deserialize_message(event.payload);
    if (message.header.type == MessageType::HEARTBEAT_MESSAGE)
    {
        return;
    }
    logger.log("[RB] SID: " + std::to_string(message.header.sender_id) + ", Org SID: " + std::to_string(message.header.original_sender_id));
    logger.log("[RB] Current Message ID: " + std::string(message.header.message_id)+ ",size: " + std::to_string(message.payload.size()));
    for (auto &msg_id : delivered)
    {
        logger.log("[RB] Delivered Message ID: " + std::string(msg_id));
    }
    
    // if m ∉ delivered then
    if (delivered.find(message.header.message_id) == delivered.end())
    {
        logger.log("[RB] Message not delivered, processing...");

        // delivered := delivered U {m}
        delivered.insert(std::string(message.header.message_id)); // Add to delivered set

        // trigger <deliver (pj, m)>
        this->deliver(event); // Deliver to self

        // if pi ∉ correct then
        if (correct.find(message.header.sender_id) == correct.end())
        {
            // trigger <beb, broadcast([pj, m])>
            Event event_beb(EventType::RB_SEND_EVENT, event.payload);
            this->eventBus.publish(event_beb); // Publish the event to the event bus
        }
        else
        {
            // from[pi] := from[pi] U {[pj, m]}
            from[message.header.sender_id].insert(message); // Add to from[pi]
        }
        // logger.log("[RB] Delivering Message: pi: " + std::to_string(message.senderPort) + ", pj: " + std::to_string(message.originalSenderPort) + ", msg: " + std::string(message.message));
    }
    else
    {
        logger.log("[RB] Message already delivered, ignoring.");
    }
}
