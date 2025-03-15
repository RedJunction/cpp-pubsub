#ifndef CPP_PUBSUB_TOPIC_HPP
#define CPP_PUBSUB_TOPIC_HPP

#include <functional>
#include <memory>
#include <regex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace pubsub {

class Message;
class Subscription;

/**
 * @brief Interface for topic filters
 */
class TopicFilter {
public:
    virtual ~TopicFilter() = default;
    
    /**
     * @brief Check if a topic matches this filter
     * @param topic Topic name to check
     * @return true if the topic matches
     */
    virtual bool matches(std::string_view topic) const = 0;
};

/**
 * @brief Exact match topic filter
 */
class ExactTopicFilter : public TopicFilter {
public:
    /**
     * @brief Constructor
     * @param topic Exact topic name to match
     */
    explicit ExactTopicFilter(std::string topic);
    
    /**
     * @brief Check if a topic exactly matches this filter
     * @param topic Topic name to check
     * @return true if the topic matches exactly
     */
    bool matches(std::string_view topic) const override;
    
private:
    std::string topic_;
};

/**
 * @brief Wildcard topic filter using regex
 */
class WildcardTopicFilter : public TopicFilter {
public:
    /**
     * @brief Constructor
     * @param pattern Wildcard pattern (e.g., "sensors/+/temperature")
     */
    explicit WildcardTopicFilter(std::string pattern);
    
    /**
     * @brief Check if a topic matches this wildcard pattern
     * @param topic Topic name to check
     * @return true if the topic matches the pattern
     */
    bool matches(std::string_view topic) const override;
    
private:
    std::string pattern_;
    std::regex regex_;
    
    /**
     * @brief Convert MQTT-style wildcards to regex
     * @param pattern Pattern with MQTT wildcards
     * @return Regex pattern
     */
    static std::string wildcardToRegex(const std::string& pattern);
};

/**
 * @brief A topic in the PubSub system
 */
class Topic {
public:
    /**
     * @brief Constructor
     * @param name Topic name
     */
    explicit Topic(std::string name);
    
    /**
     * @brief Get the topic name
     * @return Topic name
     */
    const std::string& name() const;
    
    /**
     * @brief Publish a message to this topic
     * @param message Message to publish
     * @return true if the message was published successfully
     */
    bool publish(std::shared_ptr<Message> message);
    
    /**
     * @brief Add a subscription to this topic
     * @param subscription Subscription to add
     */
    void add_subscription(std::shared_ptr<Subscription> subscription);
    
    /**
     * @brief Remove a subscription from this topic
     * @param subscription_id ID of the subscription to remove
     * @return true if the subscription was removed
     */
    bool remove_subscription(const std::string& subscription_id);
    
    /**
     * @brief Get the number of subscriptions to this topic
     * @return Number of subscriptions
     */
    size_t subscription_count() const;
    
private:
    std::string name_;
    std::unordered_map<std::string, std::shared_ptr<Subscription>> subscriptions_;
};

/**
 * @brief Factory for creating topic filters
 */
class TopicFilterFactory {
public:
    /**
     * @brief Create a topic filter from a pattern
     * @param pattern Topic pattern (exact or with wildcards)
     * @return Shared pointer to a topic filter
     */
    static std::shared_ptr<TopicFilter> create(std::string pattern);
    
    /**
     * @brief Check if a pattern contains wildcards
     * @param pattern Pattern to check
     * @return true if the pattern contains wildcards
     */
    static bool has_wildcards(std::string_view pattern);
};

} // namespace pubsub

#endif // CPP_PUBSUB_TOPIC_HPP 