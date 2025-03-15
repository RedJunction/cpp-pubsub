#ifndef CPP_PUBSUB_VERSION_HPP
#define CPP_PUBSUB_VERSION_HPP

#include <string>
#include <string_view>

namespace pubsub {

/**
 * @brief Version information for the PubSub library
 */
struct Version {
    static constexpr int major = 0;
    static constexpr int minor = 1;
    static constexpr int patch = 0;
    
    /**
     * @brief Get the version as a string (e.g., "0.1.0")
     * @return Version string
     */
    static std::string_view as_string() {
        static const std::string version = std::to_string(major) + "." + 
                                          std::to_string(minor) + "." + 
                                          std::to_string(patch);
        return version;
    }
};

} // namespace pubsub

#endif // CPP_PUBSUB_VERSION_HPP 