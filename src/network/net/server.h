#pragma once 
#include "listener.h"
#include "common/define.h"
#include "../timer_worker/timer_worker.h"
namespace gb
{

struct ServerOptions
{
    //int work_thread_num;        //网络处理线程数
    int max_connection_count;   //允许的最大连接数量  -1没有限制
    int keep_alive_time;        //保持连接的时间 -1没有限制
    int max_pending_buffer_size;//一个连接等待发送队列最大缓冲区 单位MB  0表示没有缓冲区 默认100MB

    //网络吞吐 -1表示没有限制
    int max_throughput_in;
    int max_throughput_out;
    
    size_t io_service_pool_size; //实际网络处理线程数

    //一个基本块64B  默认是4  时间内存大小  64<<4 1024B
    size_t write_buffer_base_block_factor;
    size_t read_buffer_base_block_factor;  
    
    bool no_delay;  //默认true  

	ServerOptions()
	//: work_thread_num(8)
	: max_connection_count(-1)
	, keep_alive_time(-1)
	, max_pending_buffer_size(100)
	, max_throughput_in(-1)
	, max_throughput_out(-1)
	, io_service_pool_size(4)
	, write_buffer_base_block_factor(4)
	, read_buffer_base_block_factor(9)
	, no_delay(true)
    {}
};



class ServerImpl :public std::enable_shared_from_this<ServerImpl>
{
public:
    static const int MAINTAIN_INTERVAL_IN_MS = 100;     //维护间隔 100ms

public:
    ServerImpl(const ServerOptions& options);
    virtual ~ServerImpl();

    bool Start(std::string_view server_address);
    void Stop();

    time_point_t GetStartTime();

    ServerOptions GetOptions();

    void ResetOptions(const ServerOptions& options);
    
    int ConnectionCount();

    void GetPendingStat(int64_t* pending_message_count, int64_t* pending_buffer_size, int64_t* pending_data_size);

    bool IsListening();

    bool ReStartListen();

    IoServicePoolPtr GetIoServicePool();

public:
    void SetReceivedCallBack(Session::session_received_callback_t callback);
    void SetAcceptCallBack(Session::session_connected_callback_t callback);
    void SetCloseCallBack(Session::session_closed_callback_t callback);

private:
    void OnCreated(const SessionPtr& session);

    void OnAccepted(const SessionPtr& session);

    void OnAcceptedFailed(NET_ErrorCode error_code, const std::string_view reason);

    void OnReceived(const SessionPtr& session, const ReadBufferPtr& buffer, int meta_size,int64_t data_size);

    void OnClosed(const SessionPtr& stream);

    void StopSession();
    void ClearSession();

    void TimerMaintain(const time_point_t& now);


private:
    IoServicePoolPtr _io_service_pool;
    Endpoint      _listen_endpoint;
    ListenerPtr   _listener;
    ServerOptions _options;

    bool _is_runing;
    time_point_t _start_time;
    int64_t _ticks_per_second;
    int64_t _last_maintain_ticks;
    int64_t _last_restart_listen_ticks;
    int64_t _last_switch_stat_slot_ticks;
    int64_t _last_print_connection_ticks;
    int64_t _restart_listen_interval_ticks;
    std::mutex _start_stop_mutex;

    int64_t _slice_count;
    int64_t _slice_quota_in;
    int64_t _slice_quota_out;
    int64_t _max_pending_buffer_size;
    int64_t _keep_alive_ticks;
    
    FlowControllerPtr _flow_controller;

    std::set<SessionPtr> _session_set;
    std::mutex           _session_set_mutex;
    
    TimerWorkerPtr  _timer_worker;
    IoWorkerPtr _maintain_thread;

private:
    Session::session_received_callback_t  _received_callback;
    Session::session_connected_callback_t _connected_callback; 
    Session::session_closed_callback_t    _close_callback;

    NON_COPYABLE(ServerImpl);
    
};


class Server
{

public:
    explicit Server(const ServerOptions& option = ServerOptions());
    virtual ~Server();
    
    bool Start(const std::string& server_address);
    void Stop();

    ServerOptions GetOptions();
    void ResetOptions(const ServerOptions& options);

    int ConnectionCount();
    bool IsListening();

    const std::shared_ptr<ServerImpl>& impl(); 
    
    void SetReceivedCallBack(Session::session_received_callback_t callback);
    void SetAcceptCallBack(Session::session_connected_callback_t callback);
    void SetCloseCallBack(Session::session_closed_callback_t callback);
    
    IoServicePoolPtr GetIoServicePool();

private:
    
    std::shared_ptr<ServerImpl> _impl;
};

}