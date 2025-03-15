cmake_minimum_required(VERSION 3.14)

project(cpp-pubsub 
    VERSION 0.1.0
    DESCRIPTION "High-performance C++ Publish-Subscribe Library"
    LANGUAGES CXX
)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置编译选项
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# 查找线程库
find_package(Threads REQUIRED)

# 添加源代码目录
add_subdirectory(src)

# 创建示例目录
add_subdirectory(examples)

# 安装头文件
include(GNUInstallDirs)
install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# 创建配置文件
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/cpp-pubsub-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cpp-pubsub-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cpp-pubsub-config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cpp-pubsub
)

# 安装配置文件
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/cpp-pubsub-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/cpp-pubsub-config-version.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cpp-pubsub
)

# 启用测试
option(BUILD_TESTING "Build the testing tree." OFF)
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif() 