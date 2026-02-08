#include "log_help.h"
#include <fstream>
#include <iostream>
#include <string>
#include "gbnet/log/net_log_help.h"
using namespace std;




static void NetlogToSpdlog(netlog::Level level, const char* msg, size_t len)
{
    auto logger = spdlog::get(LOG_NAME);
    if (!logger) return;
    spdlog::level::level_enum lv = spdlog::level::info;
    switch (level)
    {
        case netlog::Level::Trace: lv = spdlog::level::trace; break;
        case netlog::Level::Debug: lv = spdlog::level::debug; break;
        case netlog::Level::Info: lv = spdlog::level::info; break;
        case netlog::Level::Warning: lv = spdlog::level::warn; break;
        case netlog::Level::Error: lv = spdlog::level::err; break;
        case netlog::Level::Fatal: lv = spdlog::level::critical; break;
    }
    logger->log(lv, "{}", std::string_view(msg, len));
}

GbLog::GbLog()
	:m_bInit(false)
{

}

GbLog::~GbLog()
{
	if (m_bInit)
	{
		this->UnInit();
	}
}

bool GbLog::Init(const char* nFileName, const int nMaxFileSize, const int nMaxFile,
	const OutMode outMode, const OutPosition outPos, const OutLevel outLevel)
{
	if (m_bInit)
	{
		printf("It's already initialized\n");
		return false;
	}
	m_bInit = true;

	try
	{
		//sink容器
		std::vector<spdlog::sink_ptr> vecSink;

		//控制台
		if (outPos & CONSOLE)
		{
			const char* pFormat = "%^%Y-%m-%d %H:%M:%S.%e|t:%t|%s:%#|%v%$";
			auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
			console_sink->set_pattern(pFormat);
			vecSink.push_back(console_sink);
		}

		//文件
		if (outPos & FILE)
		{
			const char* file_pattern = "[%l]|%Y-%m-%d %H:%M:%S.%e|t:%t|%s:%#|%v";
            auto        file_sink    = std::make_shared<spdlog::sinks::daily_file_sink_mt>(nFileName, 0,0);
            file_sink->set_pattern(file_pattern);
			vecSink.push_back(file_sink);
		}

		//设置logger使用多个sink
		if (outMode == ASYNC)//异步
		{
			spdlog::init_thread_pool(102400, 1);
			auto tp = spdlog::thread_pool();
			m_pLogger = std::make_shared<spdlog::async_logger>(LOG_NAME, begin(vecSink), end(vecSink), tp, spdlog::async_overflow_policy::block);
		}
		else//同步
		{
			m_pLogger = std::make_shared<spdlog::logger>(LOG_NAME, begin(vecSink), end(vecSink));
		}
		m_pLogger->set_level((spdlog::level::level_enum)outLevel);

		//遇到warn级别，立即flush到文件
		m_pLogger->flush_on(spdlog::level::warn);
		//定时flush到文件，每三秒刷新一次
		spdlog::flush_every(std::chrono::seconds(3));
		spdlog::register_logger(m_pLogger);

		//注册网络日志
		netlog::SetLogSink(&NetlogToSpdlog);
	}
	catch (const spdlog::spdlog_ex& ex)
	{
		std::cout << "Log initialization failed: " << ex.what() << std::endl;
		return false;
	}
	return true;
}

void GbLog::UnInit()
{
	spdlog::drop_all();
	spdlog::shutdown();
}

