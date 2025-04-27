#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <cstdint>

// we ignore certain messages assuming they are always less than 30 bytes
// like heartbeat messages in other applications
// be careful with this

#pragma pack(push, 1)
struct ProcessCrashEvent
{
    int32_t processId; // process ID
    bool operator==(const ProcessCrashEvent &other) const
    {
        return processId == other.processId;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct HeartbeatMessage {
    int32_t senderPort; // sender ID (can also be int ID, IP, etc.)
    bool operator==(const HeartbeatMessage &other) const
    {
        return senderPort == other.senderPort;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct ReliableBroadcastMessage
{
    int32_t senderPort; // sender ID (can also be int ID, IP, etc.)
    int32_t originalSenderPort; // original sender ID (can also be int ID, IP, etc.)
    char message[512];  // fixed-size message buffer

    bool operator==(const ReliableBroadcastMessage &other) const
    {
        return std::memcmp(message, other.message, 512) == 0;
    }
};
#pragma pack(pop)

// Now specialize std::hash:
namespace std
{
    template <>
    struct hash<ReliableBroadcastMessage>
    {
        std::size_t operator()(const ReliableBroadcastMessage &msg) const
        {
            return std::hash<std::string_view>()(std::string_view(msg.message, 512));
        }
    };
}



#pragma pack(push, 1)
struct BestEffortBroadcastMessage
{
    char message[512]; // fixed-size message buffer
    bool operator==(const BestEffortBroadcastMessage &other) const
    {
        return std::memcmp(message, other.message, 512) == 0;
    }
};
#pragma pack(pop)

// Now specialize std::hash:
namespace std
{
    template <>
    struct hash<BestEffortBroadcastMessage>
    {
        std::size_t operator()(const BestEffortBroadcastMessage &msg) const
        {
            return std::hash<std::string_view>()(std::string_view(msg.message, 512));
        }
    };
}

#endif // !1
