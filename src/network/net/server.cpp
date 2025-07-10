#include "server.h"
#include <gbnet/common/endpoint_help.h>

namespace gb
{

 ServerImpl::ServerImpl(const ServerOptions& options)
     : _options(options)
     , _is_runing(false)
     , _start_time(std::chrono::steady_clock::now())
     , _ticks_per_second(std::chrono::seconds(1).count())
     , _last_maintain_ticks(0)
     , _last_restart_listen_ticks(0)
     , _last_switch_stat_slot_ticks(0)
     , _last_print_connection_ticks(0)
{
    _slice_count = std::max(1, 1000 / MAINTAIN_INTERVAL_IN_MS);
    _slice_quota_in = _options.max_throughput_in == -1 ? -1 : std::max(0L, _options.max_throughput_in*1024L *1024L)/_slice_count;
    _slice_quota_out = _options.max_throughput_out == -1 ? -1 : std::max(0L, _options.max_throughput_out*1024L *1024L)/_slice_count;
    _max_pending_buffer_size = std::max(0L, _options.max_pending_buffer_size * 1024L * 1024L);
    _keep_alive_ticks        = _options.keep_alive_time == -1 ? -1 : std::max(1, _options.keep_alive_time) * _ticks_per_second;

    _restart_listen_interval_ticks = _ticks_per_second * 3;

    NETWORK_LOG("RpcServerImpl(): quota_in={}MB/s, quota_out={}MB/s, max_pending_buffer_size={}MB, keep_alive_time={}seconds",
                _slice_quota_in == -1 ? -1 : _slice_quota_in * _slice_count / (1024L * 1024L),
                _slice_quota_out == -1 ? -1 : _slice_quota_out * _slice_count / (1024L * 1024L),
                _max_pending_buffer_size / (1024L * 1024L),
                _keep_alive_ticks == -1 ? -1 : _keep_alive_ticks / _ticks_per_second);
}

 ServerImpl::~ServerImpl()
{
     Stop();
 }

bool ServerImpl::Start(std::string_view server_address)
{
    std::lock_guard<std::mutex> _lock(_start_stop_mutex);
    if (_is_runing)
        return false;
    _flow_controller.reset(new FlowController(_slice_quota_in == -1, _slice_quota_in,_slice_quota_out == -1, _slice_quota_out));
    
    _io_service_pool.reset(new IoServicePool(_options.io_service_pool_size));
    _io_service_pool->Run();

    _maintain_thread.reset(new IoWorker());
    _maintain_thread->Run();  //²»¼ÓÔØ½Å±¾

    
    if (!ResolveAddress(_maintain_thread->GetIoContext(), std::string(server_address), &_listen_endpoint))
    {
        NETWORK_LOG("Start(): resolve server address failed: {}", server_address);
        _maintain_thread.reset();
        _io_service_pool.reset();
        _flow_controller.reset();
        return false;
    }

    _listener.reset(new Listener(_maintain_thread->GetIoContext(),_io_service_pool,_listen_endpoint));
    _listener->set_create_callback(asio_bind(&ServerImpl::OnCreated,shared_from_this(),_(1)));
    _listener->set_accept_callback(asio_bind(&ServerImpl::OnAccepted,shared_from_this(),_(1)));
    _listener->set_accept_fail_callback(asio_bind(&ServerImpl::OnAcceptedFailed,shared_from_this(),_(1),_(2)));
    if (!_listener->start_listen())
    {
        NETWORK_LOG("Start(): listen failed: {}", server_address);
        _listener.reset();
        _maintain_thread.reset();
        _io_service_pool.reset();
        _flow_controller.reset();
        return false;
    }

    NETWORK_LOG("Start(): listen succeed:: {}", server_address);

    _timer_worker.reset(new TimerWorker(_maintain_thread->GetIoContext()));
    _timer_worker->set_time_duration(std::chrono::milliseconds(MAINTAIN_INTERVAL_IN_MS));
    _timer_worker->set_work_routine(asio_bind(&ServerImpl::TimerMaintain, shared_from_this(), _(1)));
    _timer_worker->start();
    
    _is_runing = true;

    return true;
}

void ServerImpl::Stop()
{
    std::lock_guard<std::mutex> _lock(_start_stop_mutex);
    if (!_is_runing)
        return;
    _is_runing = false;
    _timer_worker->stop();
    _listener->close();
    StopSession();

    _timer_worker.reset();
    _listener.reset();
    ClearSession();
    _io_service_pool->Stop();
    _maintain_thread->Stop();
    _io_service_pool.reset();
    _maintain_thread.reset();
    _flow_controller.reset();
}

gb::time_point_t ServerImpl::GetStartTime()
{
    return _start_time;
}

gb::ServerOptions ServerImpl::GetOptions()
{
    return _options;
}

void ServerImpl::ResetOptions(const ServerOptions& options)
{
    int64_t old_slice_quota_in = _slice_quota_in;
    int64_t old_slice_quota_out = _slice_quota_out;
    int64_t old_max_pending_buffer_size = _max_pending_buffer_size;
    int64_t old_keep_alive_ticks = _keep_alive_ticks;
    int64_t old_max_connection_count = _options.max_connection_count;

	_options.max_throughput_in = options.max_throughput_in;
    _options.max_throughput_out = options.max_throughput_out;
    _options.max_pending_buffer_size = options.max_pending_buffer_size;
    _options.keep_alive_time = options.keep_alive_time;
    _options.max_connection_count = options.max_connection_count;


    _slice_quota_in = _options.max_throughput_in == -1 ? -1 : std::max(0L, _options.max_throughput_in * 1024L * 1024L) / _slice_count;
    _slice_quota_out = _options.max_throughput_out == -1 ? -1 : std::max(0L, _options.max_throughput_out * 1024L * 1024L) / _slice_count;
    _max_pending_buffer_size = std::max(0L, _options.max_pending_buffer_size * 1024L * 1024L);
    _keep_alive_ticks = _options.keep_alive_time == -1 ? -1 : std::max(1, _options.keep_alive_time) * _ticks_per_second;

    if (_max_pending_buffer_size != old_max_pending_buffer_size)
    {
        std::lock_guard<std::mutex> _lock(_session_set_mutex);
        for (const SessionPtr& e : _session_set)
        {
            e->set_max_pending_buffer_size(_max_pending_buffer_size);
        }
    }

    if (_slice_quota_in != old_slice_quota_in)
    {
        _flow_controller->reset_read_quota(_slice_quota_in == -1, _slice_quota_in);
    }

    if (_slice_quota_out != old_slice_quota_out)
    {
        _flow_controller->reset_write_quota(_slice_quota_out == -1, _slice_quota_out);
    }

    NETWORK_LOG("ResetOptions(): quota_in={}MB/s(old {}MB/s),\n quota_out={}MB/s(old {}MB/s),\n max_pending_buffer_size={}MB(old {}MB),\n keep_alive_time={}seconds(old {}seconds),\n max_connection_count={}(old {})",
         _slice_quota_in == -1 ? -1 : _slice_quota_in * _slice_count / (1024L * 1024L),
         old_slice_quota_in == -1 ? -1 : old_slice_quota_in * _slice_count / (1024L * 1024L),
         _slice_quota_out == -1 ? -1 : _slice_quota_out * _slice_count / (1024L * 1024L),
         old_slice_quota_out == -1 ? -1 : old_slice_quota_out * _slice_count / (1024L * 1024L),
         _max_pending_buffer_size / (1024L * 1024L),
         old_max_pending_buffer_size / (1024L * 1024L),
         _keep_alive_ticks == -1 ? -1 : _keep_alive_ticks / _ticks_per_second,
         old_keep_alive_ticks == -1 ? -1 : old_keep_alive_ticks / _ticks_per_second,
         _options.max_connection_count,
         old_max_connection_count);
}

int ServerImpl::ConnectionCount()
{
    std::lock_guard<std::mutex> _lock(_session_set_mutex);
    return _session_set.size();
}

void ServerImpl::GetPendingStat(int64_t* pending_message_count, int64_t* pending_buffer_size, int64_t* pending_data_size)
{
    std::lock_guard<std::mutex> _lock(_session_set_mutex);
    int64_t message_count = 0;
    int64_t buffer_size = 0;
    int64_t data_size = 0;
	for (const SessionPtr& e : _session_set)
    {
        message_count += e->pending_message_count();
        buffer_size += e->pending_buffer_size();
        data_size += e->pending_data_size();
    }
    *pending_message_count = message_count;
    *pending_buffer_size = buffer_size;
    *pending_data_size = data_size;
}

bool ServerImpl::IsListening()
{
    std::lock_guard<std::mutex> _lock(_start_stop_mutex);
    return _is_runing && !_listener->is_close();
}

bool ServerImpl::ReStartListen()
{
    std::lock_guard<std::mutex> _lock(_start_stop_mutex);
    if (!_is_runing)
        return false;
    
    _listener->close();
    _listener.reset(new Listener(_io_service_pool,_listen_endpoint));
    _listener->set_create_callback(asio_bind(&ServerImpl::OnCreated,shared_from_this(),_(1)));
    _listener->set_accept_callback(asio_bind(&ServerImpl::OnAccepted,shared_from_this(),_(1)));
    _listener->set_accept_fail_callback(asio_bind(&ServerImpl::OnAcceptedFailed,shared_from_this(),_(1),_(2)));
    if (!_listener->start_listen())
    {
        NETWORK_LOG("Start(): listen failed");
        return false;
    }
	NETWORK_LOG("Start(): listen succeed");
    return true;

}

gb::IoServicePoolPtr ServerImpl::GetIoServicePool()
{
    return _io_service_pool;
}

void ServerImpl::SetReceivedCallBack(Session::session_received_callback_t callback)
{
    _received_callback = callback;
}

void ServerImpl::SetAcceptCallBack(Session::session_connected_callback_t callback)
{
    _connected_callback = callback;
}

void ServerImpl::SetCloseCallBack(Session::session_closed_callback_t callback)
{
    _close_callback = callback;
}

void ServerImpl::OnCreated(const SessionPtr& session)
{
    session->set_flow_controller(_flow_controller);
    session->set_received_callback(asio_bind(&ServerImpl::OnReceived, shared_from_this(), _(1), _(2), _(3), _(4)));
    session->set_closed_callback(asio_bind(&ServerImpl::OnClosed, shared_from_this(), _(1)));
}

void ServerImpl::OnAccepted(const SessionPtr& session)
{
    if (!_is_runing)
    {
        session->close("server not runing");
        return;
    }
    
    size_t read_base_block_factor = std::min(_options.read_buffer_base_block_factor,(size_t)TRAN_BUF_BLOCK_MAX_FACTOR);
    session->set_read_buffer_base_block_factor(read_base_block_factor);
    size_t write_base_block_factor = std::min(_options.write_buffer_base_block_factor,(size_t)TRAN_BUF_BLOCK_MAX_FACTOR);
    session->set_write_buffer_base_block_factor(write_base_block_factor);

    session->set_no_delay(_options.no_delay);
    if (_options.max_connection_count != -1 && ConnectionCount() >= _options.max_connection_count)
    {
        session->close("exceed max connection count");
        return;
    }

    session->set_max_pending_buffer_size(_max_pending_buffer_size);
    session->reset_ticks(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - _start_time).count(), true);

    {
        std::lock_guard<std::mutex> _lock(_session_set_mutex);
        _session_set.insert(session);
    }

    if (_connected_callback)
        _connected_callback(session);

}

void ServerImpl::OnAcceptedFailed(NET_ErrorCode error_code, const std::string_view reason)
{
    //todo
}

void ServerImpl::OnReceived(const SessionPtr& session, const ReadBufferPtr& buffer, int meta_size, int64_t data_size)
{
    if (!_is_runing)
        return;

    if (_received_callback)
        _received_callback(session, buffer, meta_size, data_size);
}

void ServerImpl::OnClosed(const SessionPtr& session)
{
	if (!_is_runing)
		return;

    if (_close_callback)
        _close_callback(session);

    std::lock_guard<std::mutex> _lock(_session_set_mutex);
    _session_set.erase(session);
}

void ServerImpl::StopSession()
{
    std::lock_guard<std::mutex> _lock(_session_set_mutex);
    for (const auto& e : _session_set)
    {
        e->close("server stop");
    }
}

void ServerImpl::ClearSession()
{
    std::lock_guard<std::mutex> _lock(_session_set_mutex);
    _session_set.clear();
}

void ServerImpl::TimerMaintain(const time_point_t& now)
{
    int64_t now_ticks =  std::chrono::duration_cast<std::chrono::seconds>(now - _start_time).count();
    if (_listener->is_close() && now_ticks - _last_restart_listen_ticks >= _restart_listen_interval_ticks)
    {
        _last_restart_listen_ticks = now_ticks;
        ReStartListen();
    }

    if (_keep_alive_ticks != -1 || _slice_quota_in != -1 || _slice_quota_out != -1)
    {
        std::set<SessionPtr> sessions;
        {
			std::lock_guard<std::mutex> _lock(_session_set_mutex);
            sessions = _session_set;
        }
        if (_keep_alive_ticks != -1)
        {
            for (const SessionPtr& session : sessions)
            {
                if (session->is_closed())
                    continue;

                if (now_ticks - session->last_rw_ticks() >= _keep_alive_ticks)
                {
                    session->close("keep alive timeout");
                }
                else
                {
                    session->reset_ticks(now_ticks, false);
                }
            }
        }

        if (_slice_quota_in != -1)
        {
            _flow_controller->recharge_read_quota(_slice_quota_in);
            
            std::vector<FlowControlItem> trigger_list;
            trigger_list.reserve(sessions.size());

            for (const SessionPtr& session : sessions)
            {
                if (session->is_closed())
                    continue;

                int token = session->read_quota_token();
                if (token <= 0)
                {
                    trigger_list.push_back(FlowControlItem(token, session.get()));
                }
            }

            std::sort(trigger_list.begin(), trigger_list.end());
            for (const auto& item : trigger_list)
            {
                item.session->trigger_receive();
            }
        }

        if (_slice_quota_out != -1)
        {
            _flow_controller->recharge_write_quota(_slice_quota_out);
            std::vector<FlowControlItem> trigger_list;
            trigger_list.reserve(sessions.size());

            for (const SessionPtr& session : sessions)
            {
                if (session->is_closed())
                    continue;

                int token = session->write_quota_token();
                if (token <= 0)
                {
                    trigger_list.push_back(FlowControlItem(token, session.get()));
                }
            }

            std::sort(trigger_list.begin(), trigger_list.end());
            for (const auto& item : trigger_list)
            {
                item.session->trigger_send();
            }
        }
    }
    _last_maintain_ticks = now_ticks;
}



/**
 * -------------------------------------------------------------------------------------------------------------------------------------
 * -------------------------------------------------------------------------------------------------------------------------------------
 */


 Server::Server(const ServerOptions& option /*= ServerOptions()*/)
{
     _impl.reset(new ServerImpl(option));
 }

 Server::~Server()
{
     Stop();
 }

bool Server::Start(const std::string& server_address)
{
    return _impl->Start(server_address);
}

void Server::Stop()
{
    _impl->Stop();
}


gb::ServerOptions Server::GetOptions()
{
    return _impl->GetOptions();
}

void Server::ResetOptions(const ServerOptions& options)
{
    _impl->ResetOptions(options);
}

int Server::ConnectionCount()
{
    return _impl->ConnectionCount();
}

bool Server::IsListening()
{
    return _impl->IsListening();
}

const std::shared_ptr<gb::ServerImpl>& Server::impl()
{
    return _impl;
}

void Server::SetReceivedCallBack(Session::session_received_callback_t callback)
{
    _impl->SetReceivedCallBack(callback);
}

void Server::SetAcceptCallBack(Session::session_connected_callback_t callback)
{
    _impl->SetAcceptCallBack(callback);
}

void Server::SetCloseCallBack(Session::session_closed_callback_t callback)
{
    _impl->SetCloseCallBack(callback);
}

gb::IoServicePoolPtr Server::GetIoServicePool()
{
    return _impl->GetIoServicePool();
}

}