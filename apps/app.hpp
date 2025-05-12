#ifndef APP_HPP
#define APP_HPP

#include <iostream>
#include <thread>
#include <atomic>
#include <vector>
#include <fstream>
#include <filesystem>  // C++17
#include <distrifein/utils.hpp>
#include <distrifein/message.hpp>

namespace fs = std::filesystem;

enum class BroadcasterType
{
    UniformReliableBroadcast,
    ReliableBroadcast,
    BestEffortBroadcast,
    Unknown
};

template <typename T>
class Application
{
public:
    Application(T &broadcaster, EventBus &eventBus, int node_id);
    void decode(const Event &event, BroadcasterType type);
    void run();

private:
    int node_id;
    std::atomic<bool> running;
    Logger &logger = Logger::getInstance();
    T &broadcaster;
    EventBus &eventBus;
    BroadcasterType broadcasterType; // <-- Add this

    void handle_text_message_input();
    void handle_image_message_input();
};

template <typename T>
Application<T>::Application(T &broadcaster, EventBus &eventBus, int node_id)
    : broadcaster(broadcaster), eventBus(eventBus), running(true), node_id(node_id)
{

    if (std::is_same<T, UniformReliableBroadcaster>::value)
    {
        broadcasterType = BroadcasterType::UniformReliableBroadcast;
        eventBus.subscribe(EventType::URB_DELIVER_EVENT, [this](const Event &event)
                           { this->decode(event, broadcasterType); });
    }
    else if (std::is_same<T, ReliableBroadcaster>::value)
    {
        broadcasterType = BroadcasterType::ReliableBroadcast;
        eventBus.subscribe(EventType::RB_DELIVER_EVENT, [this](const Event &event)
                           { this->decode(event, broadcasterType); });
    }
    else if (std::is_same<T, BestEffortBroadcaster>::value)
    {
        broadcasterType = BroadcasterType::BestEffortBroadcast;
        eventBus.subscribe(EventType::BEB_DELIVER_EVENT, [this](const Event &event)
                           { this->decode(event, broadcasterType); });
    }
    else
    {
        logger.log("[Error] Unknown broadcaster type.");
    }

    logger.log("[App] Initialized with " + std::string(typeid(T).name()) + "...");
}

template <typename T>
void Application<T>::decode(const Event &event, BroadcasterType type)
{
    Message message = deserialize_message(event.payload);
    if (message.header.type == MessageType::TEXT_MESSAGE){
        std::string text_message(reinterpret_cast<char *>(message.payload.data()));
        logger.log("[App] Delivering text message:" + text_message);
    } else if (message.header.type == MessageType::IMAGE_MESSAGE){
        // Construct node-specific folder path
        std::string folder_name = "node_" + std::to_string(node_id);

        // Create folder if it doesn't exist
        fs::create_directories(folder_name);

        // Construct file name using timestamp to avoid overwrite
        std::string filename = folder_name + "/received_" + std::to_string(message.header.timestamp) + ".ppm";

        // Write image data to file
        std::ofstream output(filename, std::ios::binary);
        output.write(reinterpret_cast<const char*>(message.payload.data()), message.payload.size());
        output.close();
        logger.log("[App] Delivering image message and saved to " + filename);
    } else {
        logger.log("[App] Delivering some other message type.");
    }
}

template <typename T>
void Application<T>::handle_text_message_input()
{
    std::cout << "Enter text message: ";
    std::string line;
    std::getline(std::cin, line);
    if (line.empty())
        return;

    Message msg;
    msg.header.type = MessageType::TEXT_MESSAGE;
    msg.header.sender_id = this->node_id;
    msg.header.original_sender_id = this->node_id;
    msg.header.recipient_id = 0; 
    generate_message_id(msg.header.message_id);
    msg.header.timestamp = std::chrono::system_clock::now().time_since_epoch().count();
    msg.header.chunk_index = 0;  // Set chunk index (0 for single chunk)
    msg.header.total_chunks = 1; // Set total chunks (1 for single chunk)
    msg.header.crc32 = 0;        // Set CRC32 (0 for now, can be calculated later)
    msg.header.payload_size = line.size();
    msg.payload.assign(line.begin(), line.end());
    msg.payload.push_back('\0');
    msg.header.payload_size = msg.payload.size(); // now includes null terminator

    std::vector<uint8_t> payload;
    payload.resize(sizeof(MessageHeader) + msg.payload.size());

    std::memcpy(payload.data(), &msg.header, sizeof(MessageHeader));
    std::memcpy(payload.data() + sizeof(MessageHeader), msg.payload.data(), msg.payload.size());

    Event event(EventType::APP_SEND_EVENT, payload);
    eventBus.publish(event);
}

template <typename T>
void Application<T>::handle_image_message_input()
{
    std::cout << "Enter image filename (e.g., image.ppm): ";
    std::string line;
    std::getline(std::cin, line);
    if (line.empty())
        return;

    std::cout << "PPM filename found: " << line << "\n";
    Message msg;
    std::ifstream ppm_file(line, std::ios::binary);
    if (!ppm_file)
    {
        std::cerr << "Error: Could not open file " << line << "\n";
        return;
    }

    // Read entire file into msg.payload
    msg.payload = std::vector<uint8_t>(
        std::istreambuf_iterator<char>(ppm_file),
        std::istreambuf_iterator<char>());

    msg.header.payload_size = msg.payload.size();
    msg.header.type = MessageType::IMAGE_MESSAGE;
    msg.header.sender_id = this->node_id;
    msg.header.original_sender_id = this->node_id;
    msg.header.recipient_id = 0;
    generate_message_id(msg.header.message_id);
    msg.header.timestamp = std::chrono::system_clock::now().time_since_epoch().count();
    msg.header.chunk_index = 0;
    msg.header.total_chunks = 1;
    msg.header.crc32 = 0; // Optional: implement CRC if needed

    std::vector<uint8_t> payload;
    payload.resize(sizeof(MessageHeader) + msg.payload.size());

    std::memcpy(payload.data(), &msg.header, sizeof(MessageHeader));
    std::memcpy(payload.data() + sizeof(MessageHeader), msg.payload.data(), msg.payload.size());

    Event event(EventType::APP_SEND_EVENT, payload);
    eventBus.publish(event);
}

template <typename T>
void Application<T>::run()
{
    std::string line;
    logger.log("[App] Entering input loop. Type 'exit' to quit.");
    while (running)
    {
        logger.log("[App] Select Message Type: \n1. Text Message\n2. Image Message\n3. Exit");
        std::getline(std::cin, line);
        if (line.empty())
            continue;

        if (line == "1")
        {
            handle_text_message_input();
        }
        else if (line == "2")
        {
            handle_image_message_input();
        }
        else if (line == "3")
        {
            logger.log("[App] Exit requested.");
            running = false;
            break;
        }
    }
}

#endif