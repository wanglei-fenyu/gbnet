#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <chrono>
#include <iomanip>
#include <mutex>
#include <format>
#include <source_location>

namespace netlog
{

// 使用 enum class 避免名称冲突
enum class Level : uint8_t
{
    Trace,
    Debug,
    Info,
    Warning,
    Error,
    Fatal
};

// 线程安全的日志流
inline std::ostream& getLogStream()
{
    static std::mutex logMutex;
    std::lock_guard<std::mutex> lock(logMutex);
    return std::cerr;
}

// 获取格式化的当前时间字符串
inline std::string getCurrentTime()
{
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  now.time_since_epoch()) % 1000;
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::tm time_info;
    #ifdef _WIN32
    localtime_s(&time_info, &in_time_t);
    #else
    localtime_r(&in_time_t, &time_info);
    #endif

    char buffer[20];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &time_info);

    return std::format("{}.{:03d}", buffer, ms.count());
}

// 日志实现
template<typename... Args>
inline void logImpl(Level level, std::format_string<Args...> fmt, Args&&... args)
{
    static const char* levelNames[] = {
        "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"};

    // 使用 std::format 进行格式化
    std::string message = std::format(fmt, std::forward<Args>(args)...);
    
    auto& stream = getLogStream();
    stream << "[" << getCurrentTime() << "] "
           << "[" << levelNames[static_cast<uint8_t>(level)] << "] "
           << message << std::endl;

    if (level == Level::Fatal)
    {
        throw std::runtime_error(message);
    }
}

} // namespace netlog

// 使用 C++20 特性的日志宏
#define NET_LOG_TRACE(fmt, ...) \
    netlog::logImpl(netlog::Level::Trace, fmt, ##__VA_ARGS__)

#define NET_LOG_DEBUG(fmt, ...) \
    netlog::logImpl(netlog::Level::Debug, fmt, ##__VA_ARGS__)

#define NET_LOG_INFO(fmt, ...) \
    netlog::logImpl(netlog::Level::Info, fmt, ##__VA_ARGS__)

#define NET_LOG_WARN(fmt, ...) \
    netlog::logImpl(netlog::Level::Warning, fmt, ##__VA_ARGS__)

#define NET_LOG_ERROR(fmt, ...) \
    netlog::logImpl(netlog::Level::Error, fmt, ##__VA_ARGS__)

#define NET_LOG_FATAL(fmt, ...) \
    netlog::logImpl(netlog::Level::Fatal, fmt, ##__VA_ARGS__)

// 条件日志
#define NET_LOG_IF(condition, fmt, ...) \
    do { \
        if (condition) \
            netlog::logImpl(netlog::Level::Error, fmt, ##__VA_ARGS__); \
    } while (0)

// 检查宏
#define NET_CHECK(expression) \
    NET_LOG_IF(!(expression), "CHECK failed: {}", #expression)

#define NET_CHECK_EQ(a, b) NET_CHECK((a) == (b))
#define NET_CHECK_NE(a, b) NET_CHECK((a) != (b))
#define NET_CHECK_LT(a, b) NET_CHECK((a) < (b))
#define NET_CHECK_LE(a, b) NET_CHECK((a) <= (b))
#define NET_CHECK_GT(a, b) NET_CHECK((a) > (b))
#define NET_CHECK_GE(a, b) NET_CHECK((a) >= (b))