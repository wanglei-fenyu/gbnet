#include "worker_manager.h"
#include "log/log_help.h"

namespace gb
{
 WorkerManager::WorkerManager(size_t worker_num)
{
     LOG_INFO("WORKER NUM {}", worker_num);
     for (size_t i = 0; i < worker_num; i++)
     {
         worker_threads.emplace_back([this,i]() {
             WorkerPtr work = std::make_shared<Worker>();
             std::thread::id id = std::this_thread::get_id();
	         uint32_t thread_id = *((uint32_t*)&id);
             work->Init(thread_id, i);
             {
				 std::lock_guard<std::mutex> lock(mutex_);
				 workers.push_back(work);
             }
             work->OnStart();
             work->Run();
         });
     }

}

 WorkerManager::~WorkerManager()
{
}

size_t WorkerManager::Size()
{
    return workers.size();
}

gb::WorkerPtr WorkerManager::GetWorker(size_t index) const
{
     auto it = std::find_if(workers.begin(), workers.end(), [index](const WorkerPtr& work) {
         return work->GetIndex() == index;
     });
     if (it != workers.end())
     {
         return *it;

     }
	return nullptr;

}

gb::WorkerPtr WorkerManager::GetCurWorker() const
{
	 std::thread::id id = std::this_thread::get_id();
	 uint32_t thread_id = *((uint32_t*)&id);
      
     auto it = std::find_if(workers.begin(), workers.end(), [thread_id](const WorkerPtr& work) {
         return work->GetWorkerId() == thread_id;
     });

     if (it != workers.end())
     {
         return *it;

     }
     return nullptr;
}

std::vector<std::thread>& WorkerManager::GetThreads()
{
    return worker_threads;
}

std::vector<WorkerPtr>& WorkerManager::GetWorkers()
{
    return workers;
}

}