#include <iostream>
#include <distrifein/fd.hpp>
#include <distrifein/message.hpp>

#include <iostream>
#include <sstream>

FailureDetector::FailureDetector(TcpServer &server, EventBus &eventBus,  std::vector<EventType> deliver_events, std::vector<EventType> send_events,int timeoutMs, int intervalMs)
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
        eventBus.subscribe(eventType, [this](const Event &event)
                           { //this->handleMessage(event); 
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
    HeartbeatMessage heartbeatMessage;
    heartbeatMessage.senderPort = server.getSelfPort();
    std::vector<uint8_t> payload(reinterpret_cast<uint8_t *>(&heartbeatMessage), reinterpret_cast<uint8_t *>(&heartbeatMessage) + sizeof(heartbeatMessage));
    Event event(EventType::FD_SEND_EVENT, payload);
    Event event_fd(EventType::FD_SEND_EVENT, event.payload);
    while (running)
    {
        // for (int peerPort : server.getPeers())
        // {
        //     server.sendMessage("127.0.0.1", peerPort, event)D;
        // }
        
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

        for (int peerPort : server.getPeers())
        {
            auto it = lastHeartbeat.find(peerPort);
            bool heartbeatMissing = (it == lastHeartbeat.end() || std::chrono::duration_cast<std::chrono::milliseconds>(now - it->second).count() > timeoutMs);

            if (heartbeatMissing && crashedPeers.find(peerPort) == crashedPeers.end())
            {
                // logger.log("[FD] Peer " + std::to_string(peerPort) + " is suspected to be down.");
                
                // Trigger a process crash event
                ProcessCrashEvent processCrashEvent;
                processCrashEvent.processId = peerPort;
                std::vector<uint8_t> payload(reinterpret_cast<uint8_t *>(&processCrashEvent), reinterpret_cast<uint8_t *>(&processCrashEvent) + sizeof(processCrashEvent));
                Event event(EventType::PROCESS_CRASH_EVENT, payload);
                eventBus.publish(event);

                crashedPeers.insert(peerPort);
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

void FailureDetector::handleMessage(const Event& event)
{
    if (event.payload.size() < sizeof(HeartbeatMessage))
    {
        logger.log("[FD] Error: Payload size is less than expected.");
        return;
    }
    HeartbeatMessage heartbeatMessage;
    std::memcpy(&heartbeatMessage, event.payload.data(), sizeof(HeartbeatMessage));
    int senderPort = heartbeatMessage.senderPort;
    lastHeartbeat[senderPort] = std::chrono::steady_clock::now();
}
