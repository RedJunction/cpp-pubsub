#include "pubsub/broker.hpp"
#include "pubsub/message.hpp"
#include "pubsub/subscription.hpp"
#include "pubsub/topic.hpp"
#include <algorithm>
#include <functional>
#include <stdexcept>
#include <thread>
#include <list>

namespace pubsub {

// Custom deleter for Broker that can access protected destructor
class BrokerDeleter {
public:
    void operator()(Broker* broker) const {
        delete broker;
    }
};

// Initialize the singleton instance
std::unique_ptr<Broker, BrokerDeleter> Broker::instance_;

Broker& Broker::instance() {
    if (!instance_) {
        instance_ = std::unique_ptr<Broker, BrokerDeleter>(new Broker());
    }
    return *instance_;
}

Broker::Broker()
    : running_(false) {
}

Broker::~Broker() {
    if (running_) {
        shutdown();
    }
}

bool Broker::initialize(const BrokerConfig& config) {
    std::lock_guard<std::mutex> lock(topics_mutex_);
    
    if (running_) {
        return false; // Already running
    }
    
    config_ = config;
    
    // Set thread count if not specified
    if (config_.thread_count == 0) {
        config_.thread_count = std::thread::hardware_concurrency();
        if (config_.thread_count == 0) {
            config_.thread_count = 1; // Fallback to at least one thread
        }
    }
    
    // Initialize worker threads
    running_ = true;
    workers_.reserve(config_.thread_count);
    
    for (size_t i = 0; i < config_.thread_count; ++i) {
        workers_.emplace_back([this]() {
            worker_thread();
        });
    }
    
    return true;
}

void Broker::shutdown() {
    {
        std::lock_guard<std::mutex> lock(topics_mutex_);
        
        if (!running_) {
            return;
        }
        
        running_ = false;
    }
    
    // Notify all worker threads to exit
    queue_cv_.notify_all();
    
    // Wait for all worker threads to finish
    for (auto& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    
    workers_.clear();
    
    // Clear all topics and subscriptions
    {
        std::lock_guard<std::mutex> lock(topics_mutex_);
        topics_.clear();
        
        std::lock_guard<std::mutex> sub_lock(subscriptions_mutex_);
        subscriptions_.clear();
    }
}

bool Broker::is_running() const {
    return running_;
}

bool Broker::publish(std::string_view topic_str, std::shared_ptr<Message> message) {
    if (!running_) {
        return false;
    }
    
    // Ensure the message has the correct topic
    if (message->topic() != topic_str) {
        // If the topic doesn't match, we could either update it or return an error
        // For now, we'll just log a warning (in a real implementation)
        // std::cerr << "Warning: Message topic doesn't match provided topic" << std::endl;
    }
    
    // Increment published messages count
    published_messages_++;
    
    // Add message to queue for processing by worker threads
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        
        // Check if queue is full
        if (config_.max_queue_size > 0 && message_queue_.size() >= config_.max_queue_size) {
            // Handle based on priority
            if (message->priority() <= Priority::Normal) {
                // Drop normal or low priority messages when queue is full
                return false;
            } else {
                // For high priority messages, we can't easily remove a low priority message
                // from a std::queue, so we'll just drop the new message
                return false;
            }
        }
        
        message_queue_.push(message);
    }
    
    // Notify one worker thread to process the message
    queue_cv_.notify_one();
    
    return true;
}

std::shared_ptr<Subscription> Broker::subscribe(
    std::string_view topic_pattern,
    std::function<void(std::shared_ptr<Message>)> callback,
    const SubscriptionOptions& options) {
    
    if (!running_) {
        throw std::runtime_error("Broker is not running");
    }
    
    // Create a new subscription
    auto subscription = Subscription::create(topic_pattern, callback, options);
    
    // Store the subscription
    {
        std::lock_guard<std::mutex> lock(subscriptions_mutex_);
        subscriptions_[subscription->id()] = subscription;
    }
    
    return subscription;
}

bool Broker::unsubscribe(std::shared_ptr<Subscription> subscription) {
    if (!subscription) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(subscriptions_mutex_);
    
    auto it = subscriptions_.find(subscription->id());
    if (it == subscriptions_.end()) {
        return false;
    }
    
    // Cancel the subscription
    subscription->cancel();
    
    // Remove the subscription
    subscriptions_.erase(it);
    
    return true;
}

BrokerStats Broker::get_stats() const {
    BrokerStats stats;
    
    {
        std::lock_guard<std::mutex> lock(topics_mutex_);
        stats.topic_count = topics_.size();
    }
    
    {
        std::lock_guard<std::mutex> lock(subscriptions_mutex_);
        stats.subscription_count = subscriptions_.size();
    }
    
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        stats.queued_messages = message_queue_.size();
    }
    
    stats.published_messages = published_messages_.load();
    stats.delivered_messages = delivered_messages_.load();
    stats.worker_threads = workers_.size();
    
    return stats;
}

std::vector<std::string> Broker::get_topics() const {
    std::vector<std::string> result;
    
    std::lock_guard<std::mutex> lock(topics_mutex_);
    result.reserve(topics_.size());
    
    for (const auto& pair : topics_) {
        result.push_back(pair.first);
    }
    
    return result;
}

void Broker::clear_retained_messages() {
    std::lock_guard<std::mutex> lock(topics_mutex_);
    
    // This is a placeholder - we would need to implement clear_retained_messages in Topic
    // for (auto& pair : topics_) {
    //     pair.second->clear_retained_messages();
    // }
}

std::shared_ptr<Topic> Broker::get_or_create_topic(std::string_view topic_name) {
    std::lock_guard<std::mutex> lock(topics_mutex_);
    
    std::string topic_str(topic_name);
    auto it = topics_.find(topic_str);
    
    if (it == topics_.end()) {
        // Create a new topic
        auto topic = std::make_shared<Topic>(topic_str);
        
        // Store the topic
        topics_[topic_str] = topic;
        
        return topic;
    }
    
    return it->second;
}

void Broker::worker_thread() {
    while (running_) {
        std::shared_ptr<Message> message;
        
        // Wait for a message to process
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            
            queue_cv_.wait(lock, [this]() {
                return !running_ || !message_queue_.empty();
            });
            
            if (!running_ && message_queue_.empty()) {
                return;
            }
            
            if (!message_queue_.empty()) {
                message = message_queue_.front();
                message_queue_.pop();
            }
        }
        
        if (message) {
            process_message(message);
        }
    }
}

void Broker::process_message(std::shared_ptr<Message> message) {
    // Find matching subscriptions
    std::vector<std::shared_ptr<Subscription>> matching_subs;
    
    {
        std::lock_guard<std::mutex> lock(subscriptions_mutex_);
        
        for (auto& pair : subscriptions_) {
            if (pair.second->matches(message->topic())) {
                matching_subs.push_back(pair.second);
            }
        }
    }
    
    // Deliver the message to each matching subscription
    for (auto& sub : matching_subs) {
        if (sub->deliver(message) == DeliveryResult::Success) {
            delivered_messages_++;
        }
    }
}

} // namespace pubsub 