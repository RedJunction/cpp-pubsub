#include "pubsub/subscription.hpp"
#include "pubsub/message.hpp"
#include "pubsub/topic.hpp"
#include <mutex>
#include <vector>

namespace pubsub {

std::shared_ptr<Subscription> Subscription::create(
    std::string_view topic_pattern,
    MessageCallback callback,
    const SubscriptionOptions& options) {
    
    // Create a topic filter based on the pattern
    auto filter = TopicFilterFactory::create(std::string(topic_pattern));
    
    // Generate a unique ID
    static std::atomic<uint64_t> next_id{0};
    std::string id = "sub_" + std::to_string(next_id++);
    
    return std::make_shared<Subscription>(
        std::move(id),
        std::move(filter),
        std::move(callback),
        options
    );
}

Subscription::Subscription(
    std::string id,
    std::shared_ptr<TopicFilter> filter,
    MessageCallback callback,
    SubscriptionOptions options)
    : id_(std::move(id))
    , filter_(std::move(filter))
    , callback_(std::move(callback))
    , options_(std::move(options))
    , message_count_(0)
    , active_(true) {
}

const std::string& Subscription::id() const {
    return id_;
}

const std::shared_ptr<TopicFilter>& Subscription::filter() const {
    return filter_;
}

bool Subscription::matches(std::string_view topic) const {
    return filter_->matches(topic);
}

DeliveryResult Subscription::deliver(std::shared_ptr<Message> message) {
    // Check if subscription is active
    if (!is_active()) {
        return DeliveryResult::Rejected;
    }
    
    // Check if the message topic matches the subscription filter
    if (!matches(message->topic())) {
        return DeliveryResult::Filtered;
    }
    
    // Check if we've reached the maximum number of messages
    if (options_.max_messages > 0 && message_count_.load() >= options_.max_messages) {
        return DeliveryResult::Rejected;
    }
    
    // Increment message count
    message_count_++;
    
    // Deliver the message to the callback
    try {
        callback_(message);
        
        // Auto-acknowledge if configured
        if (options_.auto_acknowledge) {
            acknowledge(message->id());
        }
        
        return DeliveryResult::Success;
    } catch (...) {
        return DeliveryResult::Error;
    }
}

bool Subscription::acknowledge(const std::string& message_id) {
    // This is a simplified implementation
    // In a real-world scenario, we would track which messages have been acknowledged
    (void)message_id; // Suppress unused parameter warning
    return true;
}

void Subscription::cancel() {
    active_.store(false);
}

bool Subscription::is_active() const {
    return active_.load();
}

size_t Subscription::message_count() const {
    return message_count_.load();
}

} // namespace pubsub 