#pragma once
#include "../session/session.h"
#include "../timer_worker/timer_worker.h"
#include "../io_service_pool/io_service_pool.h"
namespace gb
{
struct ClientOptions
{
    int work_thread_num;        //网络处理线程数
    int callback_thread_num;
    int keep_alive_time;        //保持连接的时间 -1没有限制
    int max_pending_buffer_size;//一个连接等待发送队列最大缓冲区 单位MB  0表示没有缓冲区 默认100MB

    //网络吞吐 -1表示没有限制
    int max_throughput_in;
    int max_throughput_out;
    
    int connect_timeout;  //
    
    bool no_delay;  //默认true  

	ClientOptions()
	: work_thread_num(4)
    , callback_thread_num(4)
	, keep_alive_time(-1)
	, max_pending_buffer_size(100)
	, max_throughput_in(-1)
	, max_throughput_out(-1)
    , connect_timeout(-1)
	, no_delay(true)
    {}
};



class ClientImpl : public std::enable_shared_from_this<ClientImpl>
{
public:
    static const int MAINTAIN_INTERVAL_IN_MS = 500;
public:
    explicit ClientImpl(const ClientOptions& options);
    virtual ~ClientImpl();

    void Start();

    void Stop();

    ClientOptions GetOptions();
    IoServicePoolPtr GetIoServicePool();

    void Connect(CONNECT_TYPE type, std::string_view address);
    const SessionPtr GetSession(CONNECT_TYPE type);

    void ResetOptions(const ClientOptions& options);

    int ConnectionCount();

    bool ResolveAddressByStr(const std::string& address, Endpoint* endpoint);

public:
    void SetReceivedCallBack(Session::session_received_callback_t callback);
    void SetConnectCallBack(Session::session_connected_callback_t callback);
    void SetCloseCallBack(Session::session_closed_callback_t callback);

private:
    SessionPtr FindOrCreateStream(const Endpoint& remote_endpoint);

	void OnClosed(const SessionPtr& session);
    void OnConnected(const SessionPtr& session);
    void OnReceived(const SessionPtr& session, const ReadBufferPtr& message, int meta_size, int64_t data_size);

    void StopSessions();

    void ClearSessions();

    void TimerMaintain(const time_point_t& now);


private:
    ClientOptions _options;
    bool          _is_runing;
    std::mutex    _start_stop_mutex;
    std::atomic<int64_t> _next_request_id;
    
    time_point_t _epoch_time;
    int64_t      _ticks_per_second;
    int64_t      _last_maintain_ticks;
    int64_t      _last_print_connection_ticks;

    int64_t _slice_count;
    int64_t _slice_quota_in;
    int64_t _slice_quota_out;
    int64_t _max_pending_buffer_size;
    int64_t _keep_alive_ticks;
    
    FlowControllerPtr _flow_controller;
    
    IoServicePoolPtr _io_service_pool;
    IoWorkerPtr         _maintain_thread;
    TimerWorkerPtr _timer_worker;
    
    std::map<Endpoint, SessionPtr> _session_map;
    std::mutex                     _session_map_mutex;

    std::map<CONNECT_TYPE, SessionPtr> _session_type_map;
    std::mutex                         _session_type_map_mutex;

private:
    Session::session_received_callback_t  _received_callback;
    Session::session_connected_callback_t _connected_callback; 
    Session::session_closed_callback_t    _close_callback;
    NON_COPYABLE(ClientImpl);
};



class Client
{
public:
    explicit Client(const ClientOptions& options = ClientOptions());
    virtual ~Client();

    ClientOptions GetOptions();

    void ResetOptions(const ClientOptions& options);

    int ConnectionCount();

    void Shutdown();

public:
    void Send(CONNECT_TYPE type, Meta* meta);
    void Send(CONNECT_TYPE type,const Meta* meta, const google::protobuf::Message* message);
    void Send(CONNECT_TYPE type,const Meta* meta, const std::vector<uint8_t>& data);
    void Send(CONNECT_TYPE type,const Meta* meta, std::string_view data);
    void Send(CONNECT_TYPE type,const Meta* meta, const char* data,std::size_t size);
    void Send(CONNECT_TYPE type,const Meta* meta, const ReadBufferPtr& buffer);

public:
    void SetReceivedCallBack(Session::session_received_callback_t callback);
    void SetConnnectCallBack(Session::session_connected_callback_t callback);
    void SetCloseCallBack(Session::session_closed_callback_t callback);

public:
    const SessionPtr GetSession(CONNECT_TYPE type);
    IoServicePoolPtr GetIoServicePool();
     void Connect(CONNECT_TYPE type, std::string_view address);
public:
    const std::shared_ptr<ClientImpl>& impl() const; 

private:

    std::shared_ptr<ClientImpl> _impl;
};

}