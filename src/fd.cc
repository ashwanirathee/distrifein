#include <iostream>
#include <sstream>
#include <distrifein/fd.h>
#include <distrifein/message.h>
#include <distrifein/utils.h>

FailureDetector::FailureDetector(TcpServer &server, EventBus &eventBus, std::vector<EventType> deliver_events, std::vector<EventType> send_events, int timeoutMs, int intervalMs)
    : server(server), eventBus(eventBus), timeoutMs(timeoutMs), intervalMs(intervalMs), running(false), deliver_events(deliver_events), send_events(send_events)
{
    logger.log("[FD] Initialized with timeout: " + std::to_string(timeoutMs) + "ms, interval: " + std::to_string(intervalMs) + "ms");

    for (auto &eventType : deliver_events)
    {
        eventBus.subscribe(eventType, [this](const Event &event)
                           { this->handleMessage(event); });
    }

    for (auto &eventType : send_events)
    {
        eventBus.subscribe(eventType, [this](const Event &event) { // this->handleMessage(event);
            logger.log("[FD] unexpected subscription sending message!");
        });
    }
}

void FailureDetector::start()
{
    logger.log("[FD] Starting failure detector...");
    running = true;

    std::thread(&FailureDetector::sendHeartbeats, this).detach();
    std::thread(&FailureDetector::monitorHeartbeats, this).detach();
}

void FailureDetector::stop()
{
    running = false;
}

void FailureDetector::sendHeartbeats()
{
    std::string line = "heartbeat";
    Message msg;
    msg.header.type = MessageType::HEARTBEAT_MESSAGE;
    msg.header.sender_id = server.getSelfId();
    msg.header.recipient_id = 0;
    generate_message_id(msg.header.message_id);
    msg.header.timestamp = std::chrono::system_clock::now().time_since_epoch().count();
    msg.header.chunk_index = 0;  // Set chunk index (0 for single chunk)
    msg.header.total_chunks = 1; // Set total chunks (1 for single chunk)
    msg.header.crc32 = 0;        // Set CRC32 (0 for now, can be calculated later)
    msg.header.payload_size = line.size();
    msg.payload.assign(line.begin(), line.end());
    msg.payload.push_back('\0');
    msg.header.payload_size = msg.payload.size(); // now includes null terminator

    std::vector<uint8_t> payload;
    payload.resize(sizeof(MessageHeader) + msg.payload.size());

    std::memcpy(payload.data(), &msg.header, sizeof(MessageHeader));
    std::memcpy(payload.data() + sizeof(MessageHeader), msg.payload.data(), msg.payload.size());

    Event event(EventType::FD_SEND_EVENT, payload);
    Event event_fd(EventType::FD_SEND_EVENT, event.payload);
    while (running)
    {
        this->eventBus.publish(event_fd);
        std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));
    }
}

void FailureDetector::monitorHeartbeats()
{
    // add 10 sec delay
    std::this_thread::sleep_for(std::chrono::seconds(10));
    while (running)
    {
        auto now = std::chrono::steady_clock::now();

        for (int peerId : server.getPeerIds())
        {
            auto it = lastHeartbeat.find(peerId);
            bool heartbeatMissing = (it == lastHeartbeat.end() || std::chrono::duration_cast<std::chrono::milliseconds>(now - it->second).count() > timeoutMs);

            if (heartbeatMissing && crashedPeerIds.find(peerId) == crashedPeerIds.end())
            {
                // logger.log("[FD] Peer " + std::to_string(peerPort) + " is suspected to be down.");

                // Trigger a process crash event
                ProcessCrashEvent process_crash_event;
                process_crash_event.processId = peerId;
                std::vector<uint8_t> payload(reinterpret_cast<uint8_t *>(&process_crash_event), reinterpret_cast<uint8_t *>(&process_crash_event) + sizeof(process_crash_event));
                Event event(EventType::PROCESS_CRASH_EVENT, payload);
                eventBus.publish(event);

                crashedPeerIds.insert(peerId);
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

void FailureDetector::handleMessage(const Event &event)
{
    if (event.payload.size() < sizeof(MessageHeader))
    {
        logger.log("[FD] Error: Payload size is less than expected.");
        return;
    }
    Message heartbeat_message = deserialize_message(event.payload);
    int sender_id = heartbeat_message.header.sender_id;
    lastHeartbeat[sender_id] = std::chrono::steady_clock::now();
}
