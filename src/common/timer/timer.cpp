#include "timer.h"

#include <iostream>
namespace gb
{
Timer::Timer(int64_t id, bool is_loop, std::chrono::milliseconds duration, TimerCallBack func) 
    : active_(true)
    , id_(id)
    , loop_(is_loop)
    , duration_(duration)
    , call_func_(func) 
{
}

int64_t Timer::Id() const
{
    return id_;
}

bool Timer::IsLoop() const
{
    return loop_;
}

void Timer::Cancel()
{
    active_ = false;
}


int64_t Timer::DurationTime() const
{
    return duration_.count();
}

void Timer::operator()() const 
{
    if (active_ && call_func_)
    {
        call_func_();
    }
}

 SteadyTimer::SteadyTimer(std::chrono::milliseconds time, int64_t id, bool loop, std::function<void()>&& callFunc)
     : Timer(id, loop, time, std::move(callFunc))
     , end_time_point_(std::chrono::steady_clock::now() + time)
{
     std::cout << end_time_point_.time_since_epoch() << std::endl;
 }

bool SteadyTimer::IsExpired() const
{
    return std::chrono::steady_clock::now() > end_time_point_;

}

int64_t SteadyTimer::RemainingTime() const
{
	auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(end_time_point_- std::chrono::steady_clock::now()).count();
	return remaining > 0 ? remaining : 0;
}

void SteadyTimer::Reset()
{
     end_time_point_ = std::chrono::steady_clock::now() + duration_;
}

 SystemTimer::SystemTimer(std::chrono::milliseconds time, int64_t id, bool loop, std::function<void()>&& callFunc) 
     : Timer(id, loop, time, std::move(callFunc))
     , end_time_point_(std::chrono::system_clock::now() + time)
{
    
 }

bool SystemTimer::IsExpired() const
{
    return std::chrono::system_clock::now() >= end_time_point_;
}

int64_t SystemTimer::RemainingTime() const
{
    auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(end_time_point_ - std::chrono::system_clock::now()).count();
     return remaining > 0 ? remaining : 0;
}

void SystemTimer::Reset()
{
    end_time_point_ = std::chrono::system_clock::now() + duration_;
}

} // namespace gb