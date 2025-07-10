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
    return 0;
}

void App::Stop()
{
    runding_ = false;
    gb::WorkerManager* work_mng = gb::WorkerManager::Instance(4);
    if (work_mng)
    {
        for (auto& t :work_mng->GetThreads())
        {
            t.join();
        }
    }
}

void App::Run()
{

   gb::WorkerManager* work_mng = gb::WorkerManager::Instance();
   if (!work_mng)
   {
       LOG_INFO(" worker manager create fail");
       return;
   }
   for (size_t i = 0; i < work_mng->Size();i++)
   {
       if (0 != OnStartup(work_mng->GetWorker(i)))
       {
            LOG_INFO("OnStartup fail: worker index{}", i);
           return;
       }
   }

    auto last_time = std::chrono::high_resolution_clock::now();
    while (runding_)
    {
        auto                         current_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> elapsed      = current_time - last_time;
        last_time                                 = current_time;
        
	    for (size_t i = 0; i < work_mng->Size();i++)
	    {
           gb::WorkerPtr work = work_mng->GetWorker(i);
		   if (OnUpdate(work) != 0)
		   {
			   break;
		   }

		   if (OnTick(work, elapsed.count()) != 0)
		   {
			   break;
		   }
	    }

        auto frame_end_time = std::chrono::high_resolution_clock::now();
        auto frame_time     = frame_end_time - current_time;

        if (frame_time < frame_duration_)
        {
            std::this_thread::sleep_for(frame_duration_ - frame_time);
        }
    }

    for (size_t i = 0; i < work_mng->Size();i++)
    {
        if (0 != OnCleanup(work_mng->GetWorker(i)))
        {
            LOG_INFO("OnCleanup fail: worker index {}", i);
            return;
        }
    }

    OnUnInit();
}