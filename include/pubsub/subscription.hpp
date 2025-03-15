#ifndef CPP_PUBSUB_SUBSCRIPTION_HPP
#define CPP_PUBSUB_SUBSCRIPTION_HPP

#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <string_view>

namespace pubsub {

class Message;
class TopicFilter;

/**
 * @brief Options for a subscription
 */
struct SubscriptionOptions {
    /**
     * @brief Maximum number of messages to receive (0 = unlimited)
     */
    size_t max_messages = 0;
    
    /**
     * @brief Whether to automatically acknowledge messages
     */
    bool auto_acknowledge = true;
    
    /**
     * @brief Whether to receive messages published before the subscription was created
     */
    bool receive_existing_messages = false;
    
    /**
     * @brief Maximum time to wait for a message in milliseconds (0 = no timeout)
     */
    uint64_t timeout_ms = 0;
};

/**
 * @brief Result of a message delivery
 */
enum class DeliveryResult {
    Success,
    Filtered,
    Rejected,
    Timeout,
    Error
};

/**
 * @brief A subscription to one or more topics
 */
class Subscription : public std::enable_shared_from_this<Subscription> {
public:
    /**
     * @brief Callback type for message delivery
     */
    using MessageCallback = std::function<void(std::shared_ptr<Message>)>;
    
    /**
     * @brief Create a new subscription
     * @param topic_pattern Topic pattern to subscribe to
     * @param callback Callback function for message delivery
     * @param options Subscription options
     * @return Shared pointer to the new subscription
     */
    static std::shared_ptr<Subscription> create(
        std::string_view topic_pattern,
        MessageCallback callback,
        const SubscriptionOptions& options = {}
    );
    
    /**
     * @brief Constructor
     * @param id Subscription ID
     * @param filter Topic filter
     * @param callback Callback function
     * @param options Subscription options
     */
    Subscription(
        std::string id,
        std::shared_ptr<TopicFilter> filter,
        MessageCallback callback,
        SubscriptionOptions options
    );
    
    /**
     * @brief Get the subscription ID
     * @return Subscription ID
     */
    const std::string& id() const;
    
    /**
     * @brief Get the topic filter
     * @return Topic filter
     */
    const std::shared_ptr<TopicFilter>& filter() const;
    
    /**
     * @brief Check if a topic matches this subscription's filter
     * @param topic Topic name
     * @return true if the topic matches
     */
    bool matches(std::string_view topic) const;
    
    /**
     * @brief Deliver a message to this subscription
     * @param message Message to deliver
     * @return Delivery result
     */
    DeliveryResult deliver(std::shared_ptr<Message> message);
    
    /**
     * @brief Acknowledge a message
     * @param message_id ID of the message to acknowledge
     * @return true if the message was acknowledged
     */
    bool acknowledge(const std::string& message_id);
    
    /**
     * @brief Cancel the subscription
     */
    void cancel();
    
    /**
     * @brief Check if the subscription is active
     * @return true if the subscription is active
     */
    bool is_active() const;
    
    /**
     * @brief Get the number of messages received
     * @return Number of messages received
     */
    size_t message_count() const;
    
private:
    std::string id_;
    std::shared_ptr<TopicFilter> filter_;
    MessageCallback callback_;
    SubscriptionOptions options_;
    std::atomic<size_t> message_count_{0};
    std::atomic<bool> active_{true};
};

} // namespace pubsub

#endif // CPP_PUBSUB_SUBSCRIPTION_HPP 