#include "pubsub/topic.hpp"
#include "pubsub/message.hpp"
#include "pubsub/subscription.hpp"
#include <algorithm>
#include <regex>
#include <mutex>
#include <deque>

namespace pubsub {

// ExactTopicFilter implementation
ExactTopicFilter::ExactTopicFilter(std::string topic)
    : topic_(std::move(topic)) {
}

bool ExactTopicFilter::matches(std::string_view topic) const {
    return topic_ == topic;
}

// WildcardTopicFilter implementation
WildcardTopicFilter::WildcardTopicFilter(std::string pattern)
    : pattern_(std::move(pattern))
    , regex_(wildcardToRegex(pattern_)) {
}

bool WildcardTopicFilter::matches(std::string_view topic) const {
    return std::regex_match(std::string(topic), regex_);
}

std::string WildcardTopicFilter::wildcardToRegex(const std::string& pattern) {
    std::string result;
    result.reserve(pattern.size() * 2);
    
    for (size_t i = 0; i < pattern.size(); ++i) {
        if (pattern[i] == '+') {
            result += "[^/]+";
        } else if (pattern[i] == '#') {
            result += ".*";
        } else if (pattern[i] == '.' || pattern[i] == '*' || 
                  pattern[i] == '[' || pattern[i] == ']' || 
                  pattern[i] == '(' || pattern[i] == ')' || 
                  pattern[i] == '\\' || pattern[i] == '^' || 
                  pattern[i] == '$') {
            result += '\\';
            result += pattern[i];
        } else {
            result += pattern[i];
        }
    }
    
    return result;
}

// Topic implementation
Topic::Topic(std::string name)
    : name_(std::move(name)) {
}

const std::string& Topic::name() const {
    return name_;
}

bool Topic::publish(std::shared_ptr<Message> message) {
    // Find all subscriptions and deliver the message
    for (auto& [id, subscription] : subscriptions_) {
        subscription->deliver(message);
    }
    return true;
}

void Topic::add_subscription(std::shared_ptr<Subscription> subscription) {
    subscriptions_[subscription->id()] = subscription;
}

bool Topic::remove_subscription(const std::string& subscription_id) {
    auto it = subscriptions_.find(subscription_id);
    if (it != subscriptions_.end()) {
        subscriptions_.erase(it);
        return true;
    }
    return false;
}

size_t Topic::subscription_count() const {
    return subscriptions_.size();
}

// TopicFilterFactory implementation
std::shared_ptr<TopicFilter> TopicFilterFactory::create(std::string pattern) {
    if (has_wildcards(pattern)) {
        return std::make_shared<WildcardTopicFilter>(std::move(pattern));
    } else {
        return std::make_shared<ExactTopicFilter>(std::move(pattern));
    }
}

bool TopicFilterFactory::has_wildcards(std::string_view pattern) {
    return pattern.find('+') != std::string_view::npos || 
           pattern.find('#') != std::string_view::npos;
}

} // namespace pubsub 