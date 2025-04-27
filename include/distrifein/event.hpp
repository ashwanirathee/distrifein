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

    P2P_DELIVER_EVENT,
    P2P_SEND_EVENT,

    BEB_DELIVER_EVENT,
    BEB_SEND_EVENT,

    FD_DELIVER_EVENT,
    FD_SEND_EVENT,

    RB_DELIVER_EVENT,
    RB_SEND_EVENT,

    APP_SEND_EVENT,
    
    CLIENT_CONNECTED,
    CLIENT_DISCONNECTED,
    PROCESS_CRASH_EVENT,
    PROCESS_RESTORE_EVENT,
};

struct Event {
    EventType type;              // e.g., "MESSAGE_RECEIVED", "CLIENT_CONNECTED"
    std::vector<uint8_t> payload;   // raw bytes for maximum flexibility

    Event(EventType type, const std::vector<uint8_t>& payload)
        : type(type), payload(payload) {}
};

#endif