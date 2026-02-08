#pragma  once
#include "common/def.h"
#include "common/define.h"
#include <functional>
#include "../../script/script.h"
#include "../singleton.h"
#include <gbnet/common/define.h>
#include "concurrentqueue/concurrentqueue.h"
#include "../timer/timer_manager.h"
namespace gb
{

class Worker : public std::enable_shared_from_this<Worker>
{
	using ScriptPtr = std::shared_ptr<Script>;
public:
	Worker();
	virtual ~Worker();
public:
    void Init(uint32_t id, size_t index);
    void InitDriving(std::atomic<uint64_t>* global_tick_id, std::mutex* global_tick_mutex, std::condition_variable* global_tick_cv);
    void OnStart();
	void Run();
	void Stop();

public:
    virtual int OnStartup();
    virtual int OnUpdate(float elapsed);
    virtual int OnTick();
    virtual int OnCleanup();
public:
	void Post(const std::function<void(void)>& handler);
    void Post(std::function<void(void)>&& handler);
public:
	ScriptPtr GetScript() { return scriptPtr_; }
	uint32_t GetWorkerId();
    uint32_t  GetIndex();

public:
    std::unique_ptr<TimerManager>& GetTimerManager();

private:
    void InitLua();

private:
	ScriptPtr	scriptPtr_;
    uint32_t      index_;
    uint32_t	thread_id_;	 
	moodycamel::ConcurrentQueue<std::function<void(void)>> events_;
    std::unique_ptr<TimerManager>                                  timer_manager_;
	std::atomic<bool> runing_ = false;
    uint64_t local_tick_id_ = 0;

    std::atomic<uint64_t>* tick_id_ = nullptr;
    std::mutex* cvMutex = nullptr;
	std::condition_variable* cv = nullptr;
};

using WorkerPtr = std::shared_ptr<Worker>;
}