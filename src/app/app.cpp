#include "app.h"
#include "log/log_help.h"

void App::SetFrameRate(int fps)
{
    if (fps > 0)
    {
        frame_duration_ = std::chrono::milliseconds(1000 / fps);
    }
}


int App::Init()
{
    if (OnInit() != 0) return -1;
    runding_ = true;
    tick_id_ = 0;
    return 0;
}

void App::Stop()
{
    runding_ = false;
}

void App::Run()
{

   if (0 != OnStartup())
   {
	   return;
   }

    auto last_time = std::chrono::steady_clock::now();
    while (runding_)
    {
        auto                         current_time = std::chrono::steady_clock::now();
        std::chrono::duration<float> elapsed      = current_time - last_time;
        last_time                                 = current_time;
        tick_id_.fetch_add(1, std::memory_order_release);
        cv_.notify_all();
        if (OnUpdate(elapsed.count()) != 0)
        {
            break;
        }

	    if (OnTick() != 0)
	    {
		   break;
	    }

        auto frame_end_time = std::chrono::steady_clock::now();
        auto frame_time     = frame_end_time - current_time;

        if (frame_time < frame_duration_)
        {
            std::this_thread::sleep_for(frame_duration_ - frame_time);
        }
    }

	if (0 != OnCleanup())
	{
		return;
	}

    OnUnInit();
}