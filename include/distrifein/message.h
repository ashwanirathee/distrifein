#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <vector>
#include <cstdint>
#include <functional>

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

struct PayloadHasher {
    std::size_t operator()(const std::vector<uint8_t>& payload, int32_t originalSenderPort) const {
        std::size_t hash = 0;

        // First, hash the payload
        for (auto byte : payload) {
            hash ^= std::hash<uint8_t>{}(byte) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        }

        // Then, hash the original sender port into it
        hash ^= std::hash<int32_t>{}(originalSenderPort) + 0x9e3779b9 + (hash << 6) + (hash >> 2);

        return hash;
    }
};

enum class MessageType {
    TEXT_MESSAGE,
    IMAGE_MESSAGE,
    VIDEO_MESSAGE,
    AUDIO_MESSAGE,
    HEARTBEAT_MESSAGE,
};

#pragma pack(push, 1)
struct MessageHeader {
    MessageType type;
    uint8_t sender_id;
    uint8_t recipient_id;
    char message_id[41];  // "msg-..." + null terminator
    uint64_t timestamp;
    uint32_t chunk_index;
    uint32_t total_chunks;
    uint32_t crc32;
    uint64_t payload_size;
    uint8_t original_sender_id;
};
#pragma pack(pop)

struct Message {
    MessageHeader header;
    std::vector<uint8_t> payload;

    bool operator==(const Message &other) const
    {
        return payload.size() == other.payload.size() &&
        std::memcmp(payload.data(), other.payload.data(), payload.size()) == 0;
     }
};

namespace std {
    template <>
    struct hash<Message> {
        std::size_t operator()(const Message &msg) const {
            size_t size_to_hash = std::min<size_t>(msg.payload.size(), 512);
            return std::hash<std::string_view>()(
                std::string_view(reinterpret_cast<const char*>(msg.payload.data()), size_to_hash)
            );
        }
    };
}


Message deserialize_message(const std::vector<uint8_t>& raw);

#endif // !1
