#ifndef RB_HPP
#define RB_HPP

#include <unordered_set>
#include <distrifein/beb.hpp>
#include <distrifein/fd.hpp>
#include <distrifein/event.hpp>
#include <distrifein/eventbus.hpp>
#include <distrifein/logger.hpp>
#include <distrifein/message.hpp>

class ReliableBroadcaster
{
public:
    ReliableBroadcaster(BestEffortBroadcaster &beb, FailureDetector &fd, std::vector<int> peers, int self_port, EventBus &eventBus, bool deliveryOnReceive);
    void broadcast(ReliableBroadcastMessage &message);
    void deliver(int src, ReliableBroadcastMessage &message);

    TcpServer& getServer();
private:
    int self_port;
    std::vector<int> peers;
    BestEffortBroadcaster &beb;
    FailureDetector &fd;
    Logger &logger = Logger::getInstance();
    EventBus &eventBus;
    bool deliveryOnReceive = false; // Set to true to deliver on receive

    std::unordered_set<ReliableBroadcastMessage> delivered; // Set of delivered messages
    std::unordered_map<int, std::unordered_set<ReliableBroadcastMessage>> from; // from[pi] := ∅
    std::unordered_set<int> correct; // Set of correct nodes
};
#endif