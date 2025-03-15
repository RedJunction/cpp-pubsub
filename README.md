# C++ PubSub Library

一个高性能、现代化的C++17发布-订阅库，用于构建事件驱动型应用程序。

## 功能特点

- **高性能**：专为低延迟、高吞吐量消息传递设计
- **线程安全**：支持多线程并发使用
- **灵活的主题匹配**：支持通配符和模式匹配
- **消息保留**：可选的消息保留功能，支持延迟订阅者
- **消息优先级**：支持消息优先级处理
- **消息头部**：灵活的消息头部用于元数据
- **序列化**：可扩展的序列化接口
- **统计信息**：内置性能指标监控

## 系统架构

### 架构图

```
+----------------------------------------------------------+
|                      应用程序                             |
+----------------------------------------------------------+
                |                  ^
                | 发布消息         | 接收消息
                v                  |
+----------------------------------------------------------+
|                        Broker                            |
|                                                          |
|  +----------------+      +------------------------+      |
|  |                |      |                        |      |
|  |  消息队列       | ---> |      工作线程池        |      |
|  |                |      |                        |      |
|  +----------------+      +------------------------+      |
|          |                          |                    |
|          v                          v                    |
|  +----------------+      +------------------------+      |
|  |                |      |                        |      |
|  |    主题管理     | <--> |      订阅管理          |      |
|  |                |      |                        |      |
|  +----------------+      +------------------------+      |
|                                                          |
+----------------------------------------------------------+
                |                  ^
                | 消息分发         | 消息过滤
                v                  |
+----------------------------------------------------------+
|                      消息处理回调                         |
+----------------------------------------------------------+
```

### 核心组件

1. **Message（消息）**：
   - 消息是系统中的基本数据单元
   - 包含主题、负载、头部、优先级和时间戳等属性
   - 支持任意类型的负载通过std::any存储

2. **Topic（主题）**：
   - 主题是消息的分类标识
   - 支持层次结构（如"sensors/temperature"）
   - 可以关联多个订阅者
   - 可选地保留最近的消息

3. **Subscription（订阅）**：
   - 表示客户端对特定主题模式的兴趣
   - 包含消息处理回调函数
   - 支持消息过滤
   - 跟踪已接收的消息数量

4. **Broker（代理）**：
   - 系统的核心组件，管理所有主题和订阅
   - 维护消息队列和工作线程池
   - 处理消息的路由和分发
   - 提供统计信息和监控功能

5. **TopicFilter（主题过滤器）**：
   - 支持精确匹配和通配符匹配
   - 通配符包括"+"（单级）和"#"（多级）

## 通信原理

### 发布-订阅模式

C++ PubSub库实现了发布-订阅（Pub/Sub）设计模式，这是一种消息传递模式，其中：

1. **发布者**不直接将消息发送给特定的接收者，而是将消息分类为不同的主题
2. **订阅者**表达对一个或多个主题的兴趣，并且只接收感兴趣的消息
3. **代理**负责接收所有消息，并将其路由到相关的订阅者

这种解耦合的通信模式有以下优势：
- 发布者和订阅者不需要相互了解
- 系统组件可以独立演化
- 提高了系统的可扩展性和灵活性

### 消息流程

1. **消息发布**：
   - 客户端创建一个消息对象，设置主题和负载
   - 客户端调用Broker的publish方法发布消息
   - 消息被添加到优先级队列中

2. **消息处理**：
   - 工作线程从队列中获取消息
   - Broker查找与消息主题匹配的所有订阅
   - 消息被分发到匹配的订阅者

3. **消息订阅**：
   - 客户端通过提供主题模式和回调函数创建订阅
   - 当匹配的消息到达时，回调函数被调用
   - 订阅可以设置过滤器进一步筛选消息

4. **消息保留**：
   - 如果启用了消息保留功能，主题会保存最近的消息
   - 新的订阅者会立即收到这些保留的消息
   - 可以配置每个主题保留的最大消息数量

### 线程模型

Broker使用线程池来处理消息：

1. 主线程负责接收发布请求和创建订阅
2. 工作线程负责从队列中获取消息并分发给订阅者
3. 回调函数在工作线程上下文中执行
4. 所有共享数据结构都使用互斥锁保护，确保线程安全

## 要求

- C++17兼容的编译器
- CMake 3.14或更高版本
- 线程支持

## 构建

```bash
mkdir build && cd build
cmake ..
make
```

## 安装

```bash
make install
```

## 使用示例

```cpp
#include "pubsub/pubsub.hpp"
#include "pubsub/broker.hpp"
#include "pubsub/message.hpp"
#include <iostream>

using namespace pubsub;

int main() {
    // 初始化PubSub库
    if (!initialize(2)) {
        std::cerr << "Failed to initialize PubSub library" << std::endl;
        return 1;
    }
    
    // 获取代理实例
    Broker& broker = Broker::instance();
    
    // 订阅主题
    auto subscription = broker.subscribe("sensors/#", [](const std::shared_ptr<Message>& msg) {
        try {
            std::string payload = msg->payload<std::string>();
            std::cout << "Received: " << payload << std::endl;
        } catch (const std::bad_any_cast&) {
            std::cout << "Received non-string payload" << std::endl;
        }
    });
    
    // 发布消息
    auto msg = Message::create("sensors/temperature", std::string("Temperature: 25°C"));
    broker.publish(msg->topic(), msg);
    
    // 清理
    broker.unsubscribe(subscription);
    shutdown();
    
    return 0;
}
```

## 高级功能

### 主题通配符

- `+`：单级通配符（匹配恰好一个层级）
- `#`：多级通配符（匹配任意数量的层级）

示例：`sensors/+/temperature`匹配`sensors/living_room/temperature`但不匹配`sensors/outdoor/humidity`。

### 消息优先级

```cpp
auto msg = Message::create("important/alerts", payload);
msg->set_priority(Priority::High);
```

### 消息头部

```cpp
auto msg = Message::create("data/stream", payload);
msg->set_header("content-type", "application/json");
msg->set_header("timestamp", "2023-05-01T12:34:56Z");
```

### 消息保留

```cpp
BrokerConfig config;
config.retain_messages = true;
config.max_retained_messages = 100;

// 使用自定义配置初始化
Broker::instance().initialize(config);
```

## 性能考虑

- **消息队列**：使用优先级队列确保高优先级消息先处理
- **线程池**：可配置的工作线程数量，默认使用硬件并发数
- **内存管理**：使用智能指针（shared_ptr）管理消息和订阅的生命周期
- **锁粒度**：细粒度锁设计，最小化线程竞争

## 扩展点

1. **自定义序列化**：
   - 实现MessageSerializer接口来支持自定义序列化格式
   - 可以轻松集成JSON、Protocol Buffers、MessagePack等

2. **消息过滤**：
   - 可以为订阅添加自定义过滤器
   - 基于消息内容、头部或其他属性进行过滤

3. **持久化**：
   - 可以扩展Broker实现消息持久化
   - 支持断电恢复和消息历史查询

## 许可证

本库基于MIT许可证发布。详情请参阅LICENSE文件。 