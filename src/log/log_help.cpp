#include "log_help.h"
#include <fstream>
#include <iostream>
#include <string>
using namespace std;

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
//		const char* pFormat = "[%Y-%m-%d %H:%M:%S.%e] <thread %t> [%^%l%$]\n[%@,%!]\n%v\n";
		const char* pFormat = "%^%Y-%m-%d %H:%M:%S.%e|t:%t|%s:%#|%v%$";
		//sink����
		std::vector<spdlog::sink_ptr> vecSink;

		//����̨
		if (outPos & CONSOLE)
		{
			auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
			//console_sink->set_level(spdlog::level::trace);
			console_sink->set_pattern(pFormat);
			vecSink.push_back(console_sink);
		}

		//�ļ�
		if (outPos & FILE)
		{
			auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(nFileName, nMaxFileSize, nMaxFile);
			//file_sink->set_level(spdlog::level::trace);
			file_sink->set_pattern(pFormat);
			vecSink.push_back(file_sink);
		}

		//����loggerʹ�ö��sink
		if (outMode == ASYNC)//�첽
		{
			spdlog::init_thread_pool(102400, 1);
			auto tp = spdlog::thread_pool();
			m_pLogger = std::make_shared<spdlog::async_logger>(LOG_NAME, begin(vecSink), end(vecSink), tp, spdlog::async_overflow_policy::block);
		}
		else//ͬ��
		{
			m_pLogger = std::make_shared<spdlog::logger>(LOG_NAME, begin(vecSink), end(vecSink));
		}
		m_pLogger->set_level((spdlog::level::level_enum)outLevel);

		//����warn��������flush���ļ�
		m_pLogger->flush_on(spdlog::level::warn);
		//��ʱflush���ļ���ÿ����ˢ��һ��
		spdlog::flush_every(std::chrono::seconds(3));
		spdlog::register_logger(m_pLogger);
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

