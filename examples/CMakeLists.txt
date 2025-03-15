# 添加示例程序
add_executable(simple_example simple_example.cpp)

# 链接库
target_link_libraries(simple_example PRIVATE cpp-pubsub)

# 设置包含目录
target_include_directories(simple_example PRIVATE ${CMAKE_SOURCE_DIR}/include)

# 安装示例程序
install(TARGETS simple_example
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/examples
) 