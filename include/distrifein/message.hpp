#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <cstdint>
#pragma pack(push, 1)
struct ReliableBroadcastMessage
{
    int32_t senderPort; // sender ID (can also be int ID, IP, etc.)
    char message[512];  // fixed-size message buffer

    bool operator==(const ReliableBroadcastMessage &other) const
    {
        return senderPort == other.senderPort && std::memcmp(message, other.message, 512) == 0;
    }
};

// Now specialize std::hash:
namespace std
{
    template <>
    struct hash<ReliableBroadcastMessage>
    {
        std::size_t operator()(const ReliableBroadcastMessage &msg) const
        {
            std::size_t h1 = std::hash<int32_t>()(msg.senderPort);
            std::size_t h2 = std::hash<std::string_view>()(std::string_view(msg.message, 512));
            return h1 ^ (h2 << 1); // or any good hash combine
        }
    };
}

#pragma pack(pop)

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
