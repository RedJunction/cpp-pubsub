#ifndef CPP_PUBSUB_BROKER_HPP
#define CPP_PUBSUB_BROKER_HPP

#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <vector>

#include "pubsub/subscription.hpp"

namespace pubsub {

class Message;
class Topic;
class BrokerDeleter;

/**
 * @brief Configuration options for the broker
 */
struct BrokerConfig {
    /**
     * @brief Number of worker threads (0 = use hardware concurrency)
     */
    size_t thread_count = 0;
    
    /**
     * @brief Maximum queue size (0 = unlimited)
     */
    size_t max_queue_size = 10000;
    
    /**
     * @brief Whether to retain messages for late subscribers
     */
    bool retain_messages = true;
    
    /**
     * @brief Maximum number of retained messages per topic (0 = unlimited)
     */
    size_t max_retained_messages = 100;
    
    /**
     * @brief Whether to use strict topic matching
     */
    bool strict_topic_matching = false;
};

/**
 * @brief Statistics about the broker
 */
struct BrokerStats {
    /**
     * @brief Number of active topics
     */
    size_t topic_count = 0;
    
    /**
     * @brief Number of active subscriptions
     */
    size_t subscription_count = 0;
    
    /**
     * @brief Number of messages published
     */
    size_t published_messages = 0;
    
    /**
     * @brief Number of messages delivered
     */
    size_t delivered_messages = 0;
    
    /**
     * @brief Number of messages in the queue
     */
    size_t queued_messages = 0;
    
    /**
     * @brief Number of worker threads
     */
    size_t worker_threads = 0;
};

/**
 * @brief The message broker that manages topics and subscriptions
 */
class Broker {
public:
    // Make BrokerDeleter a friend so it can access the protected destructor
    friend class BrokerDeleter;
    
    /**
     * @brief Get the singleton instance
     * @return Reference to the broker instance
     */
    static Broker& instance();
    
    /**
     * @brief Initialize the broker
     * @param config Broker configuration
     * @return true if initialization was successful
     */
    bool initialize(const BrokerConfig& config = {});
    
    /**
     * @brief Shutdown the broker
     */
    void shutdown();
    
    /**
     * @brief Check if the broker is running
     * @return true if the broker is running
     */
    bool is_running() const;
    
    /**
     * @brief Publish a message to a topic
     * @param topic Topic name
     * @param message Message to publish
     * @return true if the message was published successfully
     */
    bool publish(std::string_view topic, std::shared_ptr<Message> message);
    
    /**
     * @brief Create a subscription to a topic pattern
     * @param topic_pattern Topic pattern to subscribe to
     * @param callback Callback function for message delivery
     * @param options Subscription options
     * @return Shared pointer to the subscription
     */
    std::shared_ptr<Subscription> subscribe(
        std::string_view topic_pattern,
        std::function<void(std::shared_ptr<Message>)> callback,
        const SubscriptionOptions& options = {}
    );
    
    /**
     * @brief Unsubscribe from a topic
     * @param subscription Subscription to cancel
     * @return true if the subscription was cancelled
     */
    bool unsubscribe(std::shared_ptr<Subscription> subscription);
    
    /**
     * @brief Get statistics about the broker
     * @return Broker statistics
     */
    BrokerStats get_stats() const;
    
    /**
     * @brief Get a list of all active topics
     * @return Vector of topic names
     */
    std::vector<std::string> get_topics() const;
    
    /**
     * @brief Clear all retained messages
     */
    void clear_retained_messages();
    
protected:
    /**
     * @brief Destructor
     */
    ~Broker();
    
private:
    /**
     * @brief Constructor (private for singleton)
     */
    Broker();
    
    /**
     * @brief Get or create a topic
     * @param topic_name Topic name
     * @return Shared pointer to the topic
     */
    std::shared_ptr<Topic> get_or_create_topic(std::string_view topic_name);
    
    /**
     * @brief Worker thread function
     */
    void worker_thread();
    
    /**
     * @brief Process a message
     * @param message Message to process
     */
    void process_message(std::shared_ptr<Message> message);
    
    /**
     * @brief Find matching subscriptions for a topic
     * @param topic Topic name
     * @return Vector of matching subscriptions
     */
    std::vector<std::shared_ptr<Subscription>> find_matching_subscriptions(std::string_view topic);
    
    // Singleton instance
    static std::unique_ptr<Broker, BrokerDeleter> instance_;
    
    // Configuration
    BrokerConfig config_;
    
    // State
    std::atomic<bool> running_{false};
    std::atomic<size_t> published_messages_{0};
    std::atomic<size_t> delivered_messages_{0};
    
    // Topics and subscriptions
    mutable std::mutex topics_mutex_;
    std::unordered_map<std::string, std::shared_ptr<Topic>> topics_;
    
    mutable std::mutex subscriptions_mutex_;
    std::unordered_map<std::string, std::shared_ptr<Subscription>> subscriptions_;
    
    // Message queue
    mutable std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
    std::queue<std::shared_ptr<Message>> message_queue_;
    
    // Worker threads
    std::vector<std::thread> workers_;
};

} // namespace pubsub

#endif // CPP_PUBSUB_BROKER_HPP 