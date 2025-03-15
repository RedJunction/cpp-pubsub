set(PUBSUB_SOURCES
    broker.cpp
    topic.cpp
    subscription.cpp
    message.cpp
    pubsub.cpp
)

add_library(cpp-pubsub ${PUBSUB_SOURCES})
target_include_directories(cpp-pubsub PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    $<INSTALL_INTERFACE:include>
)

# 设置库属性
set_target_properties(cpp-pubsub PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    EXPORT_NAME CPPPubSub
)

# 安装库
install(
    TARGETS cpp-pubsub
    EXPORT cpp-pubsub-targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

# 导出目标
install(
    EXPORT cpp-pubsub-targets
    FILE cpp-pubsub-targets.cmake
    NAMESPACE CPPPubSub::
    DESTINATION lib/cmake/cpp-pubsub
) 