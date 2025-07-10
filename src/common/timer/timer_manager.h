#include "timer.h"
#include <queue>
#include <unordered_map>
#include <memory>
#include <vector>
#include <memory_resource>

namespace gb
{

class TimerManager
{
public:
    TimerManager() = default;
    
    void Update();
    int64_t RegisterTimer(int64_t milliseconds, std::function<void()>&& callFunc, bool loop = false);
    int64_t RegisterTimer(std::chrono::milliseconds time, std::function<void()>&& callFunc, bool loop = false);
    int64_t RegisterSystemTimer(int64_t milliseconds, std::function<void()>&& callFunc, bool loop = false);
    int64_t RegisterSystemTimer(std::chrono::milliseconds time, std::function<void()>&& callFunc, bool loop = false);
    void    UnRegisterTimer(int64_t timerId);
    Timer*  GetTimer(int64_t timerId);

private:
    struct SteadyCompare
    {
        bool operator()(const SteadyTimer* l, const SteadyTimer* r) const 
        {
            return l->end_time_point_ > r->end_time_point_ || (l->end_time_point_ == r->end_time_point_ && l->Id() > r->Id());
        }
    };

    struct SystemCompare {
		bool operator()(const SystemTimer* l, const SystemTimer* r) const
		{
			return l->end_time_point_ > r->end_time_point_ || (l->end_time_point_ == r->end_time_point_ && l->Id() > r->Id());
		}
    };

private :
    std::priority_queue<SteadyTimer*, std::pmr::vector<SteadyTimer*>, SteadyCompare> steady_timers_{}; //´ó¶¥¶Ñ
    std::priority_queue<SystemTimer*, std::pmr::vector<SystemTimer*>, SystemCompare> system_timers_{};

    std::pmr::unordered_map<int64_t, std::unique_ptr<Timer>> all_timers_{};
	int64_t generate_timer_id_;
};

}
