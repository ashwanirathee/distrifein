#include <distrifein/message.h>
#include <iostream>


Message deserialize_message(const std::vector<uint8_t>& raw) {
    if (raw.size() < sizeof(MessageHeader)) {
        throw std::runtime_error("Invalid message: too small for header");
    }

    Message msg;
    std::memcpy(&msg.header, raw.data(), sizeof(MessageHeader));

    if (msg.header.payload_size > 0) {
        if (raw.size() < sizeof(MessageHeader) + msg.header.payload_size) {
            throw std::runtime_error("Incomplete payload");
        }

        msg.payload.resize(msg.header.payload_size);
        std::memcpy(msg.payload.data(), raw.data() + sizeof(MessageHeader), msg.header.payload_size);
    }

    return msg;
}
