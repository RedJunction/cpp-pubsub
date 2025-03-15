#include "pubsub/pubsub.hpp"
#include "pubsub/broker.hpp"
#include "pubsub/message.hpp"
#include "pubsub/topic.hpp"
#include "pubsub/subscription.hpp"

#include <iostream>
#include <string>
#include <thread>
#include <chrono>

using namespace pubsub;

// Simple message serializer implementation
class SimpleSerializer : public MessageSerializer {
public:
    std::vector<uint8_t> serialize(const std::any& payload) const override {
        try {
            const std::string& str = std::any_cast<std::string>(payload);
            std::vector<uint8_t> data(str.begin(), str.end());
            return data;
        } catch (const std::bad_any_cast&) {
            return {};
        }
    }
    
    std::any deserialize(const std::vector<uint8_t>& data) const override {
        std::string str(data.begin(), data.end());
        return str;
    }
};

int main() {
    // Initialize the PubSub library
    if (!initialize(2)) {
        std::cerr << "Failed to initialize PubSub library" << std::endl;
        return 1;
    }
    
    std::cout << "PubSub library initialized with 2 worker threads" << std::endl;
    
    // Get the broker instance
    Broker& broker = Broker::instance();
    
    // Subscribe to a topic
    auto subscription = broker.subscribe("sensors/#", [](const std::shared_ptr<Message>& msg) {
        try {
            std::string payload = msg->payload<std::string>();
            std::cout << "Received message on topic '" << msg->topic() << "': " << payload << std::endl;
        } catch (const std::bad_any_cast&) {
            std::cout << "Received message with non-string payload on topic '" << msg->topic() << "'" << std::endl;
        }
    });
    
    std::cout << "Subscribed to 'sensors/#' with ID: " << subscription->id() << std::endl;
    
    // Create a message serializer
    SimpleSerializer serializer;
    
    // Publish some messages
    for (int i = 0; i < 5; ++i) {
        auto msg = Message::create("sensors/temperature", std::string("Temperature: " + std::to_string(20 + i) + "°C"));
        
        if (broker.publish(msg->topic(), msg)) {
            std::cout << "Published message to 'sensors/temperature'" << std::endl;
        } else {
            std::cout << "Failed to publish message" << std::endl;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    
    // Publish to another topic
    for (int i = 0; i < 3; ++i) {
        auto msg = Message::create("sensors/humidity", std::string("Humidity: " + std::to_string(50 + i * 5) + "%"));
        
        if (broker.publish(msg->topic(), msg)) {
            std::cout << "Published message to 'sensors/humidity'" << std::endl;
        } else {
            std::cout << "Failed to publish message" << std::endl;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    
    // Get broker statistics
    BrokerStats stats = broker.get_stats();
    std::cout << "\nBroker Statistics:" << std::endl;
    std::cout << "Topics: " << stats.topic_count << std::endl;
    std::cout << "Subscriptions: " << stats.subscription_count << std::endl;
    std::cout << "Published messages: " << stats.published_messages << std::endl;
    std::cout << "Delivered messages: " << stats.delivered_messages << std::endl;
    
    // Unsubscribe
    if (broker.unsubscribe(subscription)) {
        std::cout << "\nUnsubscribed from 'sensors/#'" << std::endl;
    } else {
        std::cout << "\nFailed to unsubscribe" << std::endl;
    }
    
    // Publish after unsubscribing (should not be received)
    auto msg = Message::create("sensors/temperature", std::string("Temperature: 25°C (after unsubscribe)"));
    broker.publish(msg->topic(), msg);
    
    // Wait a bit to ensure all messages are processed
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // Shutdown the PubSub library
    shutdown();
    std::cout << "PubSub library shutdown" << std::endl;
    
    return 0;
} 