#pragma once
#include "../singleton.h"
#include "worker.h"
#include "../../app/app.h"
namespace gb
{
	class WorkerManager :public Singleton<WorkerManager>
	{
    public:
		WorkerManager(size_t worker_num =  4);
		~WorkerManager();
        size_t Size();
        WorkerPtr GetWorker(size_t index) const;
        WorkerPtr GetCurWorker() const;
        std::vector<std::thread>& GetThreads();
        std::vector<WorkerPtr>&   GetWorkers();
    private:
        std::vector<WorkerPtr>        workers;
        std::vector<std::thread> worker_threads;
        std::mutex mutex_;
	};


}