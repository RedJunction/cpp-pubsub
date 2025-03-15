#ifndef CPP_PUBSUB_MESSAGE_HPP
#define CPP_PUBSUB_MESSAGE_HPP

#include <any>
#include <chrono>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace pubsub {

/**
 * @brief Message priority levels
 */
enum class Priority {
    Low,
    Normal,
    High,
    Critical
};

/**
 * @brief Base class for message serialization
 */
class MessageSerializer {
public:
    virtual ~MessageSerializer() = default;
    
    /**
     * @brief Serialize a message to binary data
     * @param data The data to serialize
     * @return Serialized binary data
     */
    virtual std::vector<uint8_t> serialize(const std::any& data) const = 0;
    
    /**
     * @brief Deserialize binary data to a message
     * @param data The binary data to deserialize
     * @return Deserialized message
     */
    virtual std::any deserialize(const std::vector<uint8_t>& data) const = 0;
};

/**
 * @brief A message in the PubSub system
 */
class Message {
public:
    using Headers = std::unordered_map<std::string, std::string>;
    using TimePoint = std::chrono::system_clock::time_point;
    
    /**
     * @brief Create a new message with payload
     * @tparam T Type of the payload
     * @param topic Topic name
     * @param payload Message payload
     * @param priority Message priority
     * @return Shared pointer to the new message
     */
    template<typename T>
    static std::shared_ptr<Message> create(std::string_view topic, T&& payload, 
                                          Priority priority = Priority::Normal) {
        auto msg = std::make_shared<Message>(topic);
        msg->set_payload(std::forward<T>(payload));
        msg->set_priority(priority);
        return msg;
    }
    
    /**
     * @brief Constructor
     * @param topic Topic name
     */
    explicit Message(std::string_view topic);
    
    /**
     * @brief Get the message ID
     * @return Message ID
     */
    const std::string& id() const;
    
    /**
     * @brief Get the topic name
     * @return Topic name
     */
    const std::string& topic() const;
    
    /**
     * @brief Get the message creation timestamp
     * @return Creation timestamp
     */
    TimePoint timestamp() const;
    
    /**
     * @brief Get the message priority
     * @return Message priority
     */
    Priority priority() const;
    
    /**
     * @brief Set the message priority
     * @param priority New priority
     */
    void set_priority(Priority priority);
    
    /**
     * @brief Get the message headers
     * @return Reference to headers map
     */
    Headers& headers();
    
    /**
     * @brief Get the message headers (const)
     * @return Const reference to headers map
     */
    const Headers& headers() const;
    
    /**
     * @brief Set a header value
     * @param key Header key
     * @param value Header value
     */
    void set_header(const std::string& key, const std::string& value);
    
    /**
     * @brief Get a header value
     * @param key Header key
     * @param default_value Default value if header doesn't exist
     * @return Header value or default
     */
    std::string get_header(const std::string& key, const std::string& default_value = "") const;
    
    /**
     * @brief Set the message payload
     * @tparam T Type of the payload
     * @param payload Message payload
     */
    template<typename T>
    void set_payload(T&& payload) {
        payload_ = std::forward<T>(payload);
    }
    
    /**
     * @brief Get the message payload
     * @tparam T Expected type of the payload
     * @return Payload cast to the expected type
     * @throws std::bad_any_cast if the payload is not of type T
     */
    template<typename T>
    const T& payload() const {
        return std::any_cast<const T&>(payload_);
    }
    
    /**
     * @brief Check if the message has a payload of type T
     * @tparam T Type to check
     * @return true if the payload is of type T
     */
    template<typename T>
    bool has_payload_type() const {
        return payload_.type() == typeid(T);
    }
    
    /**
     * @brief Serialize the message using the provided serializer
     * @param serializer The serializer to use
     * @return Serialized message data
     */
    std::vector<uint8_t> serialize(const MessageSerializer& serializer) const;
    
    /**
     * @brief Deserialize a message from binary data
     * @param data The binary data
     * @param serializer The serializer to use
     * @return Deserialized message
     */
    static std::shared_ptr<Message> deserialize(const std::vector<uint8_t>& data, 
                                               const MessageSerializer& serializer);

private:
    std::string id_;
    std::string topic_;
    TimePoint timestamp_;
    Priority priority_;
    Headers headers_;
    std::any payload_;
};

} // namespace pubsub

#endif // CPP_PUBSUB_MESSAGE_HPP 