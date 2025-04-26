#ifndef EVENT_HPP
#define EVENT_HPP

#pragma once
#include <vector>
#include <string>

enum class EventType {
    MESSAGE_RECEIVED,
    P2P_MESSAGE_RECEIVED,
    P2P_MESSAGE_SENT,
    BEB_MESSAGE_RECEIVED,
    BEB_MESSAGE_SENT,
    RB_MESSAGE_RECEIVED,
    RB_MESSAGE_SENT,
    CLIENT_CONNECTED,
    CLIENT_DISCONNECTED
};

struct Event {
    EventType type;              // e.g., "MESSAGE_RECEIVED", "CLIENT_CONNECTED"
    std::vector<uint8_t> payload;   // raw bytes for maximum flexibility

    Event(EventType type, const std::vector<uint8_t>& payload)
        : type(type), payload(payload) {}
};

#endif