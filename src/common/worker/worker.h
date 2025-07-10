#pragma  once
#include "common/def.h"
#include "common/define.h"
#include <functional>
#include "../../script/script.h"
#include "../singleton.h"
#include <gbnet/common/define.h>
//#include "concurrentqueue/concurrentqueue.h"
#include "concurrentqueue/blockingconcurrentqueue.h"
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
    void OnStart();
	void Run();
	void Stop();

public:
    virtual int OnStartup();
    virtual int OnUpdate();
    virtual int OnTick(float elapsed);
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
	ScriptPtr	scriptPtr_;
    uint32_t      index_;
    uint32_t	thread_id_;	 
	moodycamel::BlockingConcurrentQueue<std::function<void(void)>> events_;
    std::unique_ptr<TimerManager>                                  timer_manager_;
	std::atomic<bool> runing_ = false;
};

using WorkerPtr = std::shared_ptr<Worker>;
}