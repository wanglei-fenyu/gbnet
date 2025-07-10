#include "worker.h"
#include "log/log_help.h"
#include "../../script/register_script.h"
#include "../res_path.h"

namespace gb
{
Worker::Worker() 
{
	scriptPtr_ = std::make_shared<Script>();
    timer_manager_ = std::make_unique<TimerManager>();
}

Worker::~Worker() 
{
	Stop();
}


void Worker::Init(uint32_t id, size_t index)
{
    thread_id_ = id;
    index_     = index;
}

void Worker::OnStart()
{
    LOG_INFO("Start");
    runing_.store(true);  //启动线程了
	//注册消息监听

	//加载脚本
	using sol::lib;
	if (!scriptPtr_)
		return;
	scriptPtr_->open_libraries(lib::base, lib::package,lib::string,lib::table,lib::os,lib::bit32,lib::coroutine,lib::count,lib::debug,lib::ffi,lib::io,lib::jit,lib::math,lib::utf8);
	//注册脚本 
	register_script(scriptPtr_);
	std::string scriptRootPath;

	scriptRootPath = ResPath::Instance()->FindResPath("script/main.lua");
	scriptPtr_->Load(scriptRootPath);
}


void Worker::Run()
{
    while (runing_)
    {
        std::function<void(void)> func;
        events_.wait_dequeue(func);
        func();
    }
}

void Worker::Stop()
{
     runing_.store(false);

}

int Worker::OnStartup()
{
    return 0;
}

int Worker::OnUpdate()
{
    return 0;
}

int Worker::OnTick(float elapsed)
{
    if (timer_manager_)
        timer_manager_->Update();
    return 0;
}

int Worker::OnCleanup()
{
    Stop();
	return 0;
}

void Worker::Post(const std::function<void(void)>& handler)
{
    if (runing_.load())
		events_.enqueue(handler);
}

void Worker::Post(std::function<void(void)>&& handler)
{
    if (runing_.load())
		events_.enqueue(std::move(handler));
}

uint32_t Worker::GetWorkerId()
{
	return thread_id_;
}


uint32_t Worker::GetIndex()
{
    return index_;
}

std::unique_ptr<TimerManager>& Worker::GetTimerManager()
{
    return timer_manager_;
}

}