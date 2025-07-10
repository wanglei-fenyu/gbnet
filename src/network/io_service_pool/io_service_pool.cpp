#include "io_service_pool.h"
#include "log/log_help.h"
#include "../../script/register_script.h"
#include <memory>

namespace gb
{
IoWorker::IoWorker(): m_ioContextStrand_(m_ioContext_)
{
}

 IoWorker::~IoWorker()
{
     Stop();
}

void IoWorker::OnStart()
{
}

void IoWorker::Run()
{
	m_threadPtr_ = std::make_shared<std::thread>([this]() {
		OnStart();
        m_ioContext_.get_executor().on_work_started();
		m_ioContext_.run();
	});
}

void IoWorker::Stop()
{
    m_ioContext_.stop();
    m_threadPtr_->join();
}

uint32_t IoWorker::GetWorkerId()
{
	if (!m_threadPtr_)
	return 0;
	auto id = m_threadPtr_->get_id();
	uint32_t thread_id = *((uint32_t*)&id);

	return thread_id;
}



IoServicePool::IoServicePool(int workerNum) :
    _next_service(0)
{
    for (int i = 0; i < workerNum; i++)
    {
        auto worker = IoWorkerPtr(new IoWorker());
        m_workers.push_back(worker);
    }
}

void IoServicePool::Stop()
{
    for (auto& worker : m_workers)
    {
        worker->Stop();
    }
}



IoWorkerPtr IoServicePool::GetWorker(int index)
{
    if (index > GetWorkerCount() || index < 0)
        return nullptr;
    return m_workers.at(index);
}

IoWorkerPtr IoServicePool::GetWorkerById(uint32_t id)
{
    auto worker_it = m_worker_map.find(id);
    if (worker_it == m_worker_map.end())
        return nullptr;
    return worker_it->second;
}

std::pair<int, IoService&> IoServicePool::GetIoService()
{
    int        cur_service = _next_service;
    IoService& ios         = m_workers[_next_service]->GetIoContext();
    ++_next_service;
    if (_next_service == m_workers.size())
        _next_service = 0;
    return {cur_service, ios};
}

const std::vector<IoWorkerPtr>& IoServicePool::Workers() const
{
    return m_workers;
}

void IoServicePool::Run()
{
    for (const IoWorkerPtr e : m_workers)
    {
        e->Run();
        uint32_t id = e->GetWorkerId();
        m_worker_map.insert({id, e});
    }
}


} // namespace gb