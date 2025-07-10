
#include "timer_manager.h"

namespace gb
{

void TimerManager::Update()
{    
     while (!steady_timers_.empty()) 
     {
        auto timer = steady_timers_.top();
        if (!timer->active_) {
            steady_timers_.pop();
            all_timers_.erase(timer->Id());  //删除
            continue;
        }

        if (timer->IsExpired()) {
            steady_timers_.pop();
            (*timer)();
            if (timer->IsLoop()) {
                timer->Reset();
                steady_timers_.push(timer); // 重新加入循环定时器
            } else {
                all_timers_.erase(timer->Id()); // 从全局管理中删除
            }
        } 
        else {
            break;
        }
     }

     while (!system_timers_.empty()) 
     {
        auto timer = system_timers_.top();
        if (!timer->active_) {
            system_timers_.pop();
            all_timers_.erase(timer->Id());
            continue;
        }

        if (timer->IsExpired()) {
            system_timers_.pop();
            (*timer)();
            if (timer->IsLoop()) {
                timer->Reset();
                system_timers_.push(timer); // 重新加入循环定时器
            } else {
                all_timers_.erase(timer->Id()); // 从全局管理中删除
            }
        } 
        else {
            break;
        }
     }
}

int64_t TimerManager::RegisterTimer(int64_t milliseconds, std::function<void()>&& callFunc, bool loop /*= false*/)
{
    return RegisterTimer(std::chrono::milliseconds(milliseconds), std::move(callFunc), loop);
}

int64_t TimerManager::RegisterTimer(std::chrono::milliseconds time, std::function<void()>&& callFunc, bool loop /*= false*/)
{
	auto id = ++generate_timer_id_;
	auto timer = std::make_unique<SteadyTimer>(time, id, loop, std::move(callFunc));
	steady_timers_.push(timer.get());
	all_timers_.emplace(id, std::move(timer));
	return id;
}

int64_t TimerManager::RegisterSystemTimer(int64_t milliseconds, std::function<void()>&& callFunc, bool loop /*= false*/)
{
	return RegisterSystemTimer(std::chrono::milliseconds(milliseconds), std::move(callFunc), loop);
}

int64_t TimerManager::RegisterSystemTimer(std::chrono::milliseconds time, std::function<void()>&& callFunc, bool loop /*= false*/)
{
	
	auto id = ++generate_timer_id_;
	auto timer = std::make_unique<SystemTimer>(time, id, loop, std::move(callFunc));
	system_timers_.push(timer.get());
	all_timers_.emplace(id, std::move(timer));
	return id;
}

void TimerManager::UnRegisterTimer(int64_t timerId)
{
	 auto it = all_timers_.find(timerId);
        if (it != all_timers_.end()) {
            it->second->Cancel(); //先标记为失效
            //all_timers_.erase(it);
        }
}

Timer* TimerManager::GetTimer(int64_t timerId)
{
	 auto it = all_timers_.find(timerId);
     return (it != all_timers_.end()) ? it->second.get() : nullptr;
}




}
