#include "timer_worker.h"
#include "../../common/timer_help.h"
namespace gb
{

 TimerWorker::TimerWorker(IoService& ios) 
	 : _io_service(ios)
	 , _is_runing(false)
	 , _time_duration(std::chrono::seconds(1).count())
	 , _work_routine(nullptr)
	 , _timer(_io_service)
	 , _strand(_io_service)
{
}

 TimerWorker::~TimerWorker()
{
     stop();
 }

bool TimerWorker::is_runing()
 {
    return _is_runing;
 }

 void TimerWorker::set_time_duration(const duration_t& time_duration)
{
    _time_duration = time_duration;
}

void TimerWorker::set_work_routine(const WorkRoutine& work_routine)
{
    _work_routine = work_routine;
}

void TimerWorker::start()
{
	if(_is_runing)
		return;
	_is_runing = true;

	std::lock_guard<std::mutex> _lock(_timer_mutex);
    _timer.expires_from_now(CHRONO_MICROSECONDS(_time_duration));
    _timer.async_wait(_strand.wrap(asio_bind(&TimerWorker::on_timeout, shared_from_this(), _(1))));

}

void TimerWorker::stop()
{
    if (!_is_runing)
        return;
    _is_runing = false;
    std::lock_guard<std::mutex> _lock(_timer_mutex);
    _timer.cancel();
}

void TimerWorker::on_timeout(const Error_code& ec)
{
    if (_is_runing)
    {
        time_point_t now = std::chrono::steady_clock::now();
        if (ec != Asio::error::operation_aborted && _work_routine)
        {
            _work_routine(now);
        }
        std::lock_guard<std::mutex> _lock(_timer_mutex);
		//auto duration_since_epoch = std::chrono::duration_cast<std::chrono::microseconds>(_time_duration);
        _timer.expires_at(_timer.expires_at() + CHRONO_MICROSECONDS(_time_duration));
		_timer.async_wait(_strand.wrap(asio_bind(&TimerWorker::on_timeout, shared_from_this(), _(1))));
    }
}

}