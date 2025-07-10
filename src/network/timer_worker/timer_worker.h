#pragma once 
#include <gbnet/common/define.h>

namespace gb
{
class TimerWorker : public std::enable_shared_from_this<TimerWorker>
{
	using WorkRoutine = std::function<void (const time_point_t& /*now*/)>;

public:
    TimerWorker(IoService& ios);
    ~TimerWorker();

    bool is_runing();

    void set_time_duration(const duration_t& time_duration);
    void set_work_routine(const WorkRoutine& work_routine);
    void start();
    void stop();

private:
    void on_timeout(const Error_code& ec);

private:
    IoService& _io_service;
    duration_t  _time_duration;
    WorkRoutine _work_routine;
    bool        _is_runing;
    Asio::deadline_timer _timer;
    std::mutex           _timer_mutex;
    IoServiceStrand      _strand;
};

using TimerWorkerPtr = std::shared_ptr<TimerWorker>;
}