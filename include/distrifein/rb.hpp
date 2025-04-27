#ifndef RB_HPP
#define RB_HPP

#include <unordered_set>
#include <vector>

#include <distrifein/beb.hpp>
#include <distrifein/fd.hpp>
#include <distrifein/event.hpp>
#include <distrifein/eventbus.hpp>
#include <distrifein/logger.hpp>
#include <distrifein/message.hpp>
#include <distrifein/orderedset.hpp>


#include <iostream>



class ReliableBroadcaster
{
public:
    ReliableBroadcaster(BestEffortBroadcaster &beb, FailureDetector &fd, std::vector<int> peers, int self_port, EventBus &eventBus);
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
    std::unordered_set<ReliableBroadcastMessage> delivered; // Set of delivered messages
    std::unordered_map<int, OrderedSet<ReliableBroadcastMessage>> from; // from[pi] := ∅
    std::unordered_set<int> correct; // Set of correct nodes
};
#endif