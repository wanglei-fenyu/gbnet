#pragma once

#include <string>
#include <chrono>
#include <format>
#include <stdexcept>
#include <cstdint>
#include <ctime>

namespace netlog
{

//------------------------------------------------------------
// 日志级别
//------------------------------------------------------------
enum class Level : uint8_t
{
    Trace,
    Debug,
    Info,
    Warning,
    Error,
    Fatal
};

//------------------------------------------------------------
// 日志 Sink（可注入）
// ⚠️ 函数指针，保证 DLL ABI 安全
//------------------------------------------------------------
using LogSink = void (*)(Level level, const char* msg, size_t len);

//------------------------------------------------------------
// 默认 Sink（兜底，不建议生产用）
//------------------------------------------------------------
inline void DefaultLogSink(Level, const char* msg, size_t len)
{
    fwrite(msg, 1, len, stderr);
    fputc('\n', stderr);
}

//------------------------------------------------------------
// 当前 Sink（进程级）
//------------------------------------------------------------
inline LogSink& GetLogSink()
{
    static LogSink sink = &DefaultLogSink;
    return sink;
}

//------------------------------------------------------------
// 对外注入接口（EXE / 宿主调用）
//------------------------------------------------------------
inline void SetLogSink(LogSink sink)
{
    GetLogSink() = sink ? sink : &DefaultLogSink;
}

//------------------------------------------------------------
// 时间格式化：YYYY-MM-DD HH:MM:SS.mmm
//------------------------------------------------------------
inline std::string GetCurrentTime()
{
    using namespace std::chrono;

    auto now = system_clock::now();
    auto ms  = duration_cast<milliseconds>(
                  now.time_since_epoch()) %
        1000;
    auto tt = system_clock::to_time_t(now);

    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &tt);
#else
    localtime_r(&tt, &tm);
#endif

    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);

    return std::format("{}.{:03d}", buf, ms.count());
}

//------------------------------------------------------------
// 日志实现（核心）
//------------------------------------------------------------
template <typename... Args>
inline void logImpl(
    Level                       level,
    std::format_string<Args...> fmt,
    Args&&... args)
{
    static constexpr const char* kLevelName[] = {
        "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"};

    // 格式化正文
    std::string body = std::format(
        fmt, std::forward<Args>(args)...);

    // 拼完整日志行
    std::string line = std::format(
        //"[{}][{}] {}",
        //GetCurrentTime(),
        //kLevelName[static_cast<uint8_t>(level)],
        "{}",
        body);

    // 输出（由宿主决定）
    auto sink = GetLogSink();
    sink(level, line.c_str(), line.size());

    if (level == Level::Fatal)
        throw std::runtime_error(body);
}

} // namespace netlog

//------------------------------------------------------------
// 日志宏
// //------------------------------------------------------------
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

//------------------------------------------------------------
// CHECK 宏
//------------------------------------------------------------
#define NET_LOG_IF(cond, fmt, ...)                                           \
    do {                                                                     \
        if (cond) netlog::logImpl(netlog::Level::Error, fmt, ##__VA_ARGS__); \
    } while (0)

#define NET_CHECK(expr) \
    NET_LOG_IF(!(expr), "CHECK failed: {}", #expr)

#define NET_CHECK_EQ(a, b) NET_CHECK((a) == (b))
#define NET_CHECK_NE(a, b) NET_CHECK((a) != (b))
#define NET_CHECK_LT(a, b) NET_CHECK((a) < (b))
#define NET_CHECK_LE(a, b) NET_CHECK((a) <= (b))
#define NET_CHECK_GT(a, b) NET_CHECK((a) > (b))
#define NET_CHECK_GE(a, b) NET_CHECK((a) >= (b))
