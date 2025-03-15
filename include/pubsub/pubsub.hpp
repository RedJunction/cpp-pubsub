#ifndef CPP_PUBSUB_HPP
#define CPP_PUBSUB_HPP

#include "pubsub/broker.hpp"
#include "pubsub/message.hpp"
#include "pubsub/subscription.hpp"
#include "pubsub/topic.hpp"
#include "pubsub/version.hpp"

/**
 * @namespace pubsub
 * @brief Main namespace for the C++ PubSub library
 */
namespace pubsub {

/**
 * @brief Initialize the PubSub library
 * @param thread_count Number of worker threads to use (0 = auto)
 * @return true if initialization was successful
 */
bool initialize(size_t thread_count = 0);

/**
 * @brief Shutdown the PubSub library and release resources
 */
void shutdown();

} // namespace pubsub

#endif // CPP_PUBSUB_HPP 