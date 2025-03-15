#include "pubsub/pubsub.hpp"
#include "pubsub/broker.hpp"
#include <thread>

namespace pubsub {

bool initialize(size_t thread_count) {
    // Create broker configuration
    BrokerConfig config;
    
    // Set thread count if specified
    if (thread_count > 0) {
        config.thread_count = thread_count;
    } else {
        // Use hardware concurrency with a minimum of 1 thread
        config.thread_count = std::max(1u, std::thread::hardware_concurrency());
    }
    
    // Initialize the broker
    return Broker::instance().initialize(config);
}

void shutdown() {
    // Shutdown the broker
    Broker::instance().shutdown();
}

} // namespace pubsub 