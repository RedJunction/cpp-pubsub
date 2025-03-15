#include "pubsub/message.hpp"

#include <chrono>
#include <random>
#include <sstream>
#include <iomanip>

namespace pubsub {

// Helper function to generate a unique ID
static std::string generate_id() {
    // Use high-resolution clock for timestamp part
    auto now = std::chrono::high_resolution_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
        now.time_since_epoch()).count();
    
    // Add some randomness
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dist(0, 0xFFFFFFFF);
    uint32_t random = dist(gen);
    
    // Format as hex string
    std::stringstream ss;
    ss << std::hex << std::setfill('0') << std::setw(16) << timestamp;
    ss << std::setw(8) << random;
    
    return ss.str();
}

Message::Message(std::string_view topic)
    : id_(generate_id())
    , topic_(topic)
    , timestamp_(std::chrono::system_clock::now())
    , priority_(Priority::Normal) {
}

const std::string& Message::id() const {
    return id_;
}

const std::string& Message::topic() const {
    return topic_;
}

Message::TimePoint Message::timestamp() const {
    return timestamp_;
}

Priority Message::priority() const {
    return priority_;
}

void Message::set_priority(Priority priority) {
    priority_ = priority;
}

Message::Headers& Message::headers() {
    return headers_;
}

const Message::Headers& Message::headers() const {
    return headers_;
}

void Message::set_header(const std::string& key, const std::string& value) {
    headers_[key] = value;
}

std::string Message::get_header(const std::string& key, const std::string& default_value) const {
    auto it = headers_.find(key);
    if (it != headers_.end()) {
        return it->second;
    }
    return default_value;
}

std::vector<uint8_t> Message::serialize(const MessageSerializer& serializer) const {
    // This is a simplified implementation
    // In a real-world scenario, we would serialize all message properties
    return serializer.serialize(payload_);
}

std::shared_ptr<Message> Message::deserialize(
    const std::vector<uint8_t>& data, 
    const MessageSerializer& serializer) {
    
    // This is a simplified implementation
    // In a real-world scenario, we would deserialize all message properties
    std::any payload = serializer.deserialize(data);
    
    // For now, we just create a dummy message
    auto msg = std::make_shared<Message>("deserialized");
    msg->payload_ = payload;
    
    return msg;
}

} // namespace pubsub 