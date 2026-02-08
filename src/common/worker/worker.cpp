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

void Worker::InitDriving(std::atomic<uint64_t>* global_tick_id, std::mutex* global_tick_mutex, std::condition_variable* global_tick_cv)
{
    tick_id_ = global_tick_id;
    cvMutex  = global_tick_mutex;
    cv       = global_tick_cv;
    local_tick_id_ = *tick_id_;
}

void Worker::OnStart()
{
    LOG_INFO("Start");
    runing_.store(true);  //启动线程了
	//注册消息监听

	//加载脚本
    InitLua();
}   


void Worker::Run()
{
    auto last_time = std::chrono::steady_clock::now();
    while (runing_)
    {
        if (!tick_id_ || !cv || !cvMutex)
        {
            continue;
        }
        std::unique_lock<std::mutex> lk(*cvMutex);
        cv->wait(lk);
		auto                         current_time = std::chrono::steady_clock::now();
		std::chrono::duration<float> elapsed      = current_time - last_time;
		last_time                                 = current_time;
        OnUpdate(elapsed.count());
		uint64_t g_tick_id = tick_id_->load(std::memory_order_acquire);
        while(local_tick_id_ < g_tick_id)
        {
            OnTick();
            ++local_tick_id_;
        }
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

int Worker::OnUpdate(float elapsed)
{
    if (timer_manager_)
        timer_manager_->Update();
	while(events_.size_approx() > 0)
	{
		std::function<void(void)> func;
		events_.try_dequeue(func);
		func();
	}
    return 0;
}

int Worker::OnTick()
{
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

void Worker::InitLua()
{
	using sol::lib;
	if (!scriptPtr_)
		return;
	scriptPtr_->open_libraries(lib::base, lib::package,lib::string,lib::table,lib::os,lib::bit32,lib::coroutine,lib::count,lib::debug,lib::ffi,lib::io,lib::jit,lib::math,lib::utf8);
	//注册脚本 
	_lua_(scriptPtr_);
    sol::function require = (*scriptPtr_)["require"];
#ifdef MY_DEBUG_MODE
    std::string _lua_socket = ResPath::Instance()->FindResPath("../Debug/bin/");
#else 
    std::string _lua_socket = ResPath::Instance()->FindResPath("../Release/bin/");
#endif

#if ENGINE_PLATFORM != PLATFORM_WIN32	
    _lua_socket += "?.so";
#endif
    //加载luasocket
    std::string package_cpath = (*scriptPtr_)["package"]["cpath"].get<std::string>();
    (*scriptPtr_)["package"]["cpath"] = package_cpath + ";" + _lua_socket;
    require("socket.core");


	//LuaPanda
	std::string script_path =  ResPath::Instance()->FindResPath("/script");
	std::string package_path = (*scriptPtr_)["package"]["path"];
    package_path += ";" + script_path + "/?.lua";
    (*scriptPtr_)["package"]["path"] = package_path;
    //启动调试
    auto result = scriptPtr_->safe_script_file(script_path + "/start_debug.lua");
    if (!result.valid()) {
        sol::error err = result;
     
        LOG_ERROR("Start Lua Debug Fail {}",err.what());
    }

	std::string scriptRootPath = ResPath::Instance()->FindResPath("script/main.lua");
	scriptPtr_->Load(scriptRootPath);
}

}