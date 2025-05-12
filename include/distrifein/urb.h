#ifndef URB_HPP
#define URB_HPP

#include <unordered_set>
#include <vector>

#include <distrifein/beb.h>
#include <distrifein/fd.h>
#include <distrifein/event.h>
#include <distrifein/eventbus.h>
#include <distrifein/logger.h>
#include <distrifein/message.h>
#include <distrifein/orderedset.h>

// Custom comparator for (originalSenderPort, message)
struct PendingComparator
{
    bool operator()(const Message& a, const Message& b) const
    {
        return (a.header.original_sender_id == b.header.original_sender_id) && a.payload.size() == b.payload.size() &&
        std::memcmp(a.payload.data(), b.payload.data(), a.payload.size()) == 0;
    }
};

// Custom hasher for (originalSenderPort, message)
struct PendingHasher
{
    std::size_t operator()(const Message& msg) const
    {
        std::size_t h1 = std::hash<int32_t>{}(msg.header.original_sender_id);

        std::size_t size_to_hash = std::min<std::size_t>(msg.payload.size(), 512);
        std::string_view payload_view(reinterpret_cast<const char*>(msg.payload.data()), size_to_hash);
        std::size_t h2 = std::hash<std::string_view>{}(payload_view);

        // Combine the two hashes using the Boost-like hash combine formula
        return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
    }
};


template<typename T>
bool is_subset(const std::unordered_set<T>& subset, const std::unordered_set<T>& superset)
{
    for (const auto& elem : subset)
    {
        if (superset.find(elem) == superset.end())
            return false;
    }
    return true;
}

class UniformReliableBroadcaster
{
public:
    UniformReliableBroadcaster(BestEffortBroadcaster &beb, FailureDetector &fd, std::vector<int> peers, int self_port, EventBus &eventBus);
    void broadcast(const Event& event);
    void deliver(const Event& event);

    void handleCrashEvent(const Event& event);
    void handleBEBDeliverEvent(const Event& event);
    
    TcpServer& getServer();
private:
    int node_id;
    std::vector<int> peerIds;
    BestEffortBroadcaster &beb;
    FailureDetector &fd;
    Logger &logger = Logger::getInstance();
    EventBus &eventBus;
    PayloadHasher hasher;
    void tryDelivery();
    std::unordered_set<Message> delivered; // Set of delivered messages
    std::unordered_set<Message, PendingHasher, PendingComparator> pending; // Set of pending messages

    std::unordered_map<std::size_t, std::unordered_set<int32_t>> ack; // from[pi] := ∅
    std::unordered_set<int> correct; // Set of correct nodes
};
#endif