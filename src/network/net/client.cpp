#include "client.h"
#include "common/endpoint_help.h"
namespace gb
{




 ClientImpl::ClientImpl(const ClientOptions& options) 
	 : _options(options)
	 , _is_runing(false)
	 , _next_request_id(0)
	 , _epoch_time(std::chrono::steady_clock::now())
	 , _ticks_per_second(std::chrono::seconds(1).count())
	 , _last_maintain_ticks(0)
	 , _last_print_connection_ticks(0)
{
	 _slice_count = std::max(1, 1000 / MAINTAIN_INTERVAL_IN_MS);
	 _slice_quota_in = _options.max_throughput_in == -1 ? -1 : std::max(0L, _options.max_throughput_in * 1024L * 1024L) / _slice_count;
	 _slice_quota_out = _options.max_throughput_out == -1 ? -1 : std::max(0L, _options.max_throughput_out * 1024L * 1024L) / _slice_count;
	 _max_pending_buffer_size = std::max(0L, _options.max_pending_buffer_size * 1024L * 1024L);
	 _keep_alive_ticks = _options.keep_alive_time == -1 ? -1 : std::max(1, _options.keep_alive_time) * _ticks_per_second;
	
	 NETWORK_LOG("RpcClientImpl(): quota_in={}MB/s, quota_out={}MB/s, max_pending_buffer_size={}MB, keep_alive_time={}seconds",
            _slice_quota_in == -1 ? -1 : _slice_quota_in * _slice_count / (1024L * 1024L),
            _slice_quota_out == -1 ? -1 : _slice_quota_out * _slice_count / (1024L * 1024L),
            _max_pending_buffer_size / (1024L * 1024L),
            _keep_alive_ticks == -1 ? -1 : _keep_alive_ticks / _ticks_per_second);
}

 ClientImpl::~ClientImpl()
{
     Stop();
 }

void ClientImpl::Start()
{
    std::lock_guard<std::mutex> _lock(_start_stop_mutex);
    if (_is_runing)
        return;


    _flow_controller.reset(new FlowController(_slice_quota_in == -1, _slice_quota_in,_slice_quota_out == -1, _slice_quota_out));

    _io_service_pool.reset(new IoServicePool(_options.work_thread_num));
    _io_service_pool->Run();

    _maintain_thread.reset(new IoWorker());
    _maintain_thread->Run();  //²»¼ÓÔØ½Å±¾

	_timer_worker.reset(new TimerWorker(_maintain_thread->GetIoContext()));
    _timer_worker->set_time_duration(std::chrono::milliseconds(MAINTAIN_INTERVAL_IN_MS));
    _timer_worker->set_work_routine(asio_bind(&ClientImpl::TimerMaintain, shared_from_this(), _(1)));
    _timer_worker->start();

    _is_runing = true;
    NETWORK_LOG("Start(): client started");
}

void ClientImpl::Stop()
{
    std::lock_guard<std::mutex> _lock(_start_stop_mutex);
    if (!_is_runing)
        return;
    
    _timer_worker->stop();
    StopSessions();
    _io_service_pool->Stop();
    
    _timer_worker.reset();
    ClearSessions();

    _maintain_thread->Stop();
    _io_service_pool.reset();
    _maintain_thread.reset();
    _flow_controller.reset();
   

    NETWORK_LOG("Stop(): client stop");
}

gb::ClientOptions ClientImpl::GetOptions()
{
    return _options;
}

gb::IoServicePoolPtr ClientImpl::GetIoServicePool()
{
    return _io_service_pool;
}

void ClientImpl::Connect(CONNECT_TYPE type, std::string_view address)
{
    Endpoint endpoint;
    ResolveAddressByStr(std::string(address), &endpoint);
    SessionPtr session = FindOrCreateStream(endpoint);
    if (session)
    {
		std::lock_guard<std::mutex> _lock(_session_type_map_mutex);
		_session_type_map[type] = session;
    }
}

const gb::SessionPtr ClientImpl::GetSession(CONNECT_TYPE type)
{
    auto it = _session_type_map.find(type);
    if (it != _session_type_map.end())
        return it->second;
    return nullptr;
}

void ClientImpl::ResetOptions(const ClientOptions& options)
{
    int64_t old_slice_quota_in = _slice_quota_in;
    int64_t old_slice_quota_out = _slice_quota_out;
    int64_t old_max_pending_buffer_size = _max_pending_buffer_size;
    int64_t old_keep_alive_ticks = _keep_alive_ticks;

    _options.max_throughput_in = options.max_throughput_in;
    _options.max_throughput_out = options.max_throughput_out;
    _options.max_pending_buffer_size = options.max_pending_buffer_size;
    _options.keep_alive_time = options.keep_alive_time;

    _slice_quota_in = _options.max_throughput_in == -1 ? -1 : std::max(0L, _options.max_throughput_in * 1024L * 1024L) / _slice_count;
    _slice_quota_out = _options.max_throughput_out == -1 ? -1 : std::max(0L, _options.max_throughput_out * 1024L * 1024L) / _slice_count;
    _max_pending_buffer_size = std::max(0L, options.max_pending_buffer_size * 1024L * 1024L);
    _keep_alive_ticks = _options.keep_alive_time == -1 ? -1 : std::max(1, _options.keep_alive_time) * _ticks_per_second;

    if (_max_pending_buffer_size != old_max_pending_buffer_size)
    {
        std::lock_guard<std::mutex> _lock(_session_map_mutex);
        for (auto& [endpoint,session] : _session_map)
        {
            session->set_max_pending_buffer_size(_max_pending_buffer_size);
        }
    }
    NETWORK_LOG("ResetOptions(): quota_in={}MB/s(old {}MB/s), quota_out={}MB/s(old {}MB/s), max_pending_buffer_size={}MB(old {}MB), keep_alive_time={}seconds(old {}seconds)",
            _slice_quota_in == -1 ? -1 : _slice_quota_in * _slice_count / (1024L * 1024L),
            old_slice_quota_in == -1 ? -1 : old_slice_quota_in * _slice_count / (1024L * 1024L),
            _slice_quota_out == -1 ? -1 : _slice_quota_out * _slice_count / (1024L * 1024L),
            old_slice_quota_out == -1 ? -1 : old_slice_quota_out * _slice_count / (1024L * 1024L),
            _max_pending_buffer_size / (1024L * 1024L),
            old_max_pending_buffer_size / (1024L * 1024L),
            _keep_alive_ticks == -1 ? -1 : _keep_alive_ticks / _ticks_per_second,
            old_keep_alive_ticks == -1 ? -1 : old_keep_alive_ticks / _ticks_per_second);
}

int ClientImpl::ConnectionCount()
{
	std::lock_guard<std::mutex> _lock(_session_map_mutex);
    return _session_map.size();
}

bool ClientImpl::ResolveAddressByStr(const std::string& address, Endpoint* endpoint)
{
    return ResolveAddress(_io_service_pool->GetIoService().second, address, endpoint);
}

gb::SessionPtr ClientImpl::FindOrCreateStream(const Endpoint& remote_endpoint)
{
    SessionPtr session;
    bool       create = false;
    {
        std::lock_guard<std::mutex> _lock(_session_map_mutex);
        auto itr = _session_map.find(remote_endpoint);
        if (itr != _session_map.end() && !itr->second->is_closed())
        {
            session = itr->second;
        }
        else
        {
			auto [io_service_index, ios] = _io_service_pool->GetIoService();
            session.reset(new Session(NET_TYPE::NT_CLIENT,ios, remote_endpoint));
            //session->SetIoServicePoolIndex(io_service_index);
            session->set_flow_controller(_flow_controller);
            session->set_max_pending_buffer_size(_max_pending_buffer_size);
            session->reset_ticks(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - _epoch_time).count(), true);
            session->set_connect_timeout(_options.connect_timeout);
            session->set_closed_callback(asio_bind(&ClientImpl::OnClosed, shared_from_this(), _(1)));
            session->set_connected_callback(asio_bind(&ClientImpl::OnConnected, shared_from_this(), _(1)));
            session->set_received_callback(asio_bind(&ClientImpl::OnReceived, shared_from_this(), _(1),_(2),_(3),_(4)));
            session->set_no_delay(_options.no_delay);
            
            _session_map[remote_endpoint] = session;
            create                        = true;
        }
    }

    if (create)
    {
        session->async_connect();
    }
    return session;
}

void ClientImpl::OnClosed(const SessionPtr& session)
{
    if (!_is_runing)
        return;

    {
        std::lock_guard<std::mutex> _lock(_session_map_mutex);
        auto                        itr = _session_map.find(session->remote_endpoint());
        if (itr != _session_map.end() && !itr->second->is_closed())
        {
            return;
        }

        _session_map.erase(session->remote_endpoint());
    }

    {
		std::lock_guard<std::mutex> _lock(_session_type_map_mutex);
		auto it = std::find_if(_session_type_map.begin(), _session_type_map.end(), [&session](auto& pair) {
			return pair.second->remote_endpoint() == session->remote_endpoint();
		});
		if (it != _session_type_map.end())
		{
			_session_type_map.erase(it);
		}
    }
    
    if (_close_callback)
        _close_callback(session);
}

void ClientImpl::OnConnected(const SessionPtr& session)
{
  //  if (session->is_ssl_socket())
  //  {
  //      auto ssl_sock = session->ssl_socket();
  //      if (ssl_sock)
  //      {
  //          ssl_sock->async_handshake(Asio::ssl::stream_base::client, [this,session](const Error_code& error) {
  //              if (!error && _connected_callback)
  //              {
  //                  _connected_callback(session);
  //              }
  //              else
  //                  NETWORK_LOG("handshake error:{}", error.message());

  //          });
  //      }
  //      else
  //      {
  //          
		//	NETWORK_LOG("ssl::sock  not find ssl::socket point");
  //      }
  //  }
  //  else
  //  {
		//if (_connected_callback)
		//	_connected_callback(session);
  //  }
	if (_connected_callback)
		_connected_callback(session);
}


void ClientImpl::OnReceived(const SessionPtr& session, const ReadBufferPtr& message, int meta_size, int64_t data_size)
{
    if (_received_callback)
        _received_callback(session, message, meta_size, data_size);
}

void ClientImpl::StopSessions()
{
	std::lock_guard<std::mutex> _lock(_session_map_mutex);
    for (auto& [endpoint,session] : _session_map)
	{
        session->close("client stop");
	}
}

void ClientImpl::ClearSessions()
{
    {
		std::lock_guard<std::mutex> _lock(_session_map_mutex);
		_session_map.clear();
    }

    {
        std::lock_guard<std::mutex> _lock(_session_type_map_mutex);
        _session_type_map.clear();
    }
}

void ClientImpl::SetReceivedCallBack(Session::session_received_callback_t callback)
{
    _received_callback = callback;
}

void ClientImpl::SetConnectCallBack(Session::session_connected_callback_t callback)
{
    _connected_callback = callback;
}

void ClientImpl::SetCloseCallBack(Session::session_closed_callback_t callback)
{
    _close_callback = callback;
}

void ClientImpl::TimerMaintain(const time_point_t& now)
{
    int64_t now_ticks = std::chrono::duration_cast<std::chrono::seconds>(now - _epoch_time).count();
    if (_keep_alive_ticks != -1 || _slice_quota_in != -1 || _slice_quota_out != -1)
    {
        std::map<Endpoint, SessionPtr> sessions;
        {
			std::lock_guard<std::mutex> _lock(_session_map_mutex);
            sessions = _session_map;
        }

        if (_keep_alive_ticks != -1)
        {
            for (const auto&[endpoint,session]: sessions)
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
                    //session->reset_ticks(std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count(), false);
                }
            }
        }

        if (_slice_quota_in != -1)
        {
            _flow_controller->recharge_read_quota(_slice_quota_in);
            
            std::vector<FlowControlItem> trigger_list;
            trigger_list.reserve(sessions.size());

            for (const auto& [endpoint, session] : sessions)
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

           for (const auto& [endpoint, session] : sessions)
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

 Client::Client(const ClientOptions& options /*= ClientOptions()*/)
{
     _impl.reset(new ClientImpl(options));
    _impl->Start();

 }


 Client::~Client()
 {
     Shutdown();
 }

gb::ClientOptions Client::GetOptions()
{
    return _impl->GetOptions();
}

void Client::ResetOptions(const ClientOptions& options)
{
    _impl->ResetOptions(options);
}

int Client::ConnectionCount()
{
    return _impl->ConnectionCount();
}

void Client::Shutdown()
{
    _impl->Stop();
}


void Client::Send(CONNECT_TYPE type, Meta* meta)
{
    SessionPtr session = GetSession(type);
    if (session)
        session->Send(meta);
    else
        NETWORK_LOG("session is null")
}

void Client::Send(CONNECT_TYPE type, const Meta* meta, const google::protobuf::Message* message)
{
    SessionPtr session = GetSession(type);
    if (session)
        session->Send(meta,message);
    else
        NETWORK_LOG("session is null")
}

void Client::Send(CONNECT_TYPE type, const Meta* meta, const std::vector<uint8_t>& data)
{
    SessionPtr session = GetSession(type);
    if (session)
        session->Send(meta,data);
    else
        NETWORK_LOG("session is null")
}

void Client::Send(CONNECT_TYPE type, const Meta* meta, std::string_view data)
{
    SessionPtr session = GetSession(type);
    if (session)
        session->Send(meta,data);
    else
        NETWORK_LOG("session is null")
}

void Client::Send(CONNECT_TYPE type, const Meta* meta, const char* data, std::size_t size)
{
    SessionPtr session = GetSession(type);
    if (session)
        session->Send(meta,data,size);
    else
        NETWORK_LOG("session is null")
}

void Client::Send(CONNECT_TYPE type, const Meta* meta, const ReadBufferPtr& buffer)
{
    SessionPtr session = GetSession(type);
    if (session)
        session->Send(meta,buffer);
    else
        NETWORK_LOG("session is null")
}

void Client::SetReceivedCallBack(Session::session_received_callback_t callback)
{
    _impl->SetReceivedCallBack(callback);
}

void Client::SetConnnectCallBack(Session::session_connected_callback_t callback)
{
    _impl->SetConnectCallBack(callback);
}

void Client::SetCloseCallBack(Session::session_closed_callback_t callback)
{
    _impl->SetCloseCallBack(callback);
}

const gb::SessionPtr Client::GetSession(CONNECT_TYPE type)
{
    return _impl->GetSession(type);
}

const std::shared_ptr<gb::ClientImpl>& Client::impl() const
{
    return _impl;
}

gb::IoServicePoolPtr Client::GetIoServicePool()
{
    return _impl->GetIoServicePool();
}

void Client::Connect(CONNECT_TYPE type, std::string_view address)
{
    _impl->Connect(type, address);
}

}