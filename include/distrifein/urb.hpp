#ifndef URB_HPP
#define URB_HPP

#include <unordered_set>
#include <vector>

#include <distrifein/beb.hpp>
#include <distrifein/fd.hpp>
#include <distrifein/event.hpp>
#include <distrifein/eventbus.hpp>
#include <distrifein/logger.hpp>
#include <distrifein/message.hpp>
#include <distrifein/orderedset.hpp>

// Custom comparator for (originalSenderPort, message)
struct PendingComparator
{
    bool operator()(const ReliableBroadcastMessage& a, const ReliableBroadcastMessage& b) const
    {
        return (a.originalSenderPort == b.originalSenderPort) &&
               (std::memcmp(a.message, b.message, 512) == 0);
    }
};

// Custom hasher for (originalSenderPort, message)
struct PendingHasher
{
    std::size_t operator()(const ReliableBroadcastMessage& msg) const
    {
        std::size_t h1 = std::hash<int32_t>{}(msg.originalSenderPort);
        std::size_t h2 = std::hash<std::string_view>{}(std::string_view(msg.message, 512));
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
    int self_port;
    std::vector<int> peers;
    BestEffortBroadcaster &beb;
    FailureDetector &fd;
    Logger &logger = Logger::getInstance();
    EventBus &eventBus;
    PayloadHasher hasher;
    void tryDelivery();
    std::unordered_set<ReliableBroadcastMessage> delivered; // Set of delivered messages
    std::unordered_set<ReliableBroadcastMessage, PendingHasher, PendingComparator> pending; // Set of pending messages

    std::unordered_map<std::size_t, std::unordered_set<int32_t>> ack; // from[pi] := ∅
    std::unordered_set<int> correct; // Set of correct nodes
};
#endif