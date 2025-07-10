#pragma once
#include <gbnet/message_stream/message_stream.h>
#include <gbnet/common/define.h>
#include "../../protobuf/meta.pb.h"
#include "../io_service_pool/io_service_pool.h"
namespace gb
{
class Session : public MessageStream 
{
public:
    using session_connected_callback_t = std::function<void(const std::shared_ptr<Session>&)>;
    using session_closed_callback_t = std::function<void(const std::shared_ptr<Session>&)>;
	using session_received_callback_t = std::function<void(const std::shared_ptr<Session>&,const ReadBufferPtr& /*buffer*/,int /*meta_size*/,int64_t /*data_size*/)>;
	using session_sent_callback_t = std::function<void(const std::shared_ptr<Session>&,const ReadBufferPtr&)>;


public:
    Session(NET_TYPE net_type, IoService& ios, const Endpoint& endpoint);
    virtual ~Session();

public:
    void Send(const Meta* meta);
    void Send(const Meta* meta, const google::protobuf::Message* message);
    void Send(const Meta* meta, const std::vector<uint8_t>& data);
    void Send(const Meta* meta, std::string_view data);
    void Send(const Meta* meta, const char* data,std::size_t size);
    void Send(const Meta* meta, const ReadBufferPtr& buffer);

public:
    void set_connected_callback(session_connected_callback_t call_bcak);
    void set_closed_callback(session_closed_callback_t call_bcak);
    void set_received_callback(session_received_callback_t call_bcak);
    void set_sent_callback(session_sent_callback_t call_bcak);
    void set_return_io_service_pool_fun(std::function<IoServicePoolPtr()> fun);
    const IoServicePoolPtr get_io_service_pool();

public:
    void ShutDown();
    void StartHeartbeat(duration_t  _time_duration);
    void OnHeartbeat(const Error_code& ec);

protected:
    virtual bool on_sending(const ReadBufferPtr& message) override;
    virtual void on_sent(const ReadBufferPtr& message) override;
    virtual void on_send_failed(std::string_view peason,const ReadBufferPtr& message) override;
    virtual void on_received(const ReadBufferPtr& message,int meta_size,int64_t data_size) override;
    virtual bool on_connected() override;
    virtual void on_closed() override;
        
public:
    

private:
    session_connected_callback_t _connected_callback;
    session_closed_callback_t    _closed_callback;
    session_received_callback_t  _received_callback;
    session_sent_callback_t      _sent_callbcak;
    std::function<IoServicePoolPtr()> _get_io_service_pool_fun;

    Asio::deadline_timer         _heartbeat_timer;
    duration_t                   _heartbeat_duration;
    std::atomic<bool>            _start_heartbeat;

};

struct FlowControlItem
{
	int token; //总是小于等于 0 
	Session* session;
	
	FlowControlItem(int t, Session* s) : token(t), session(s) {}

	//越接近0 优先级越高
	bool operator<(const FlowControlItem& o) const 
	{
		return token > o.token;
	}
};


using SessionPtr = std::shared_ptr<Session>;
using SessionWPtr = std::weak_ptr<Session>;

}