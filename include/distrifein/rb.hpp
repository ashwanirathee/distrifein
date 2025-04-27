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


#include <vector>
#include <unordered_set>
#include <iostream>

template <typename T>
class OrderedSet {
private:
    std::vector<T> order;                // To keep insertion order
    std::unordered_set<T> uniqueSet;     // To track uniqueness

public:
    // Insert value if it's not already in the set
    void insert(const T& value) {
        if (uniqueSet.find(value) == uniqueSet.end()) {
            order.push_back(value);
            uniqueSet.insert(value);
        }
    }

    // Clear the OrderedSet
    void clear() {
        order.clear();
        uniqueSet.clear();
    }

    // Get the size of the OrderedSet
    size_t size() const {
        return order.size();
    }

    // Iterator support for range-based for loops
    typename std::vector<T>::iterator begin() {
        return order.begin();
    }

    typename std::vector<T>::iterator end() {
        return order.end();
    }

    typename std::vector<T>::const_iterator begin() const {
        return order.begin();
    }

    typename std::vector<T>::const_iterator end() const {
        return order.end();
    }

    // Print the elements in the OrderedSet (for debugging)
    void print() const {
        for (const auto& elem : order) {
            std::cout << elem << " ";
        }
        std::cout << std::endl;
    }
};

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