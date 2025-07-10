#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <chrono>
#include <iomanip>
#include <mutex>

namespace netlog
{

// ʹ�� enum class ����������ͻ
enum class Level : uint8_t
{
    Trace,
    Debug,
    Info,
    Warning,
    Error,
    Fatal // ��� CRITICAL ����Ǳ�ڳ�ͻ
};

// �̰߳�ȫ����־���
inline std::ostream& getLogStream()
{
    static std::mutex           logMutex;
    std::lock_guard<std::mutex> lock(logMutex);
    return std::cerr;
}

// ��ȡ������ĵ�ǰʱ���ַ���
inline std::string getCurrentTime()
{
    auto now = std::chrono::system_clock::now();
    auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(
                  now.time_since_epoch()) %
        1000;
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S")
       << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

// ��־ʵ��
inline void logImpl(Level level, const std::string& message)
{
    static const char* levelNames[] = {
        "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"};

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

// ��־�꣨��NETǰ׺��
#define NET_LOG_TRACE(...)                               \
    do {                                                 \
        std::stringstream ss;                            \
        ss << __VA_ARGS__;                               \
        netlog::logImpl(netlog::Level::Trace, ss.str()); \
    } while (0)

#define NET_LOG_DEBUG(...)                               \
    do {                                                 \
        std::stringstream ss;                            \
        ss << __VA_ARGS__;                               \
        netlog::logImpl(netlog::Level::Debug, ss.str()); \
    } while (0)

#define NET_LOG_INFO(...)                               \
    do {                                                \
        std::stringstream ss;                           \
        ss << __VA_ARGS__;                              \
        netlog::logImpl(netlog::Level::Info, ss.str()); \
    } while (0)

#define NET_LOG_WARN(...)                                  \
    do {                                                   \
        std::stringstream ss;                              \
        ss << __VA_ARGS__;                                 \
        netlog::logImpl(netlog::Level::Warning, ss.str()); \
    } while (0)

#define NET_LOG_ERROR(...)                               \
    do {                                                 \
        std::stringstream ss;                            \
        ss << __VA_ARGS__;                               \
        netlog::logImpl(netlog::Level::Error, ss.str()); \
    } while (0)

#define NET_LOG_FATAL(...)                               \
    do {                                                 \
        std::stringstream ss;                            \
        ss << __VA_ARGS__;                               \
        netlog::logImpl(netlog::Level::Fatal, ss.str()); \
    } while (0)

// ������־
#define NET_LOG_IF(condition, ...)                           \
    do {                                                     \
        if (condition)                                       \
        {                                                    \
            std::stringstream ss;                            \
            ss << __VA_ARGS__;                               \
            netlog::logImpl(netlog::Level::Error, ss.str()); \
        }                                                    \
    } while (0)

// ����
#define NET_CHECK(expression) \
    NET_LOG_IF(!(expression), "CHECK failed: " #expression)

#define NET_CHECK_EQ(a, b) NET_CHECK((a) == (b))
#define NET_CHECK_NE(a, b) NET_CHECK((a) != (b))
#define NET_CHECK_LT(a, b) NET_CHECK((a) < (b))
#define NET_CHECK_LE(a, b) NET_CHECK((a) <= (b))
#define NET_CHECK_GT(a, b) NET_CHECK((a) > (b))
#define NET_CHECK_GE(a, b) NET_CHECK((a) >= (b))