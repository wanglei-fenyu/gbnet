#pragma once

#include "byte_stream.h"
#include "ssl_byte_stream.h"
#include "../buffer/buffer.h"
#include "../buffer/tran_buf_pool.h"
#include "flow_controller.h"
#include "message_header.h"

#ifdef USER_SSL_SOCKET
#define MessageStreamBase SSLByteStream
#else
#define MessageStreamBase ByteStream
#endif
namespace gb
{

class MessageStream : public MessageStreamBase
{
   
private:
    struct PendingItem
    {
        ReadBufferPtr message;
        PendingItem(const ReadBufferPtr& _message) :
            message(_message) {}
    };

    struct ReceivedItem
    {
        ReadBufferPtr message;
        int           meta_size;
        int64_t       data_size;
        ReceivedItem(const ReadBufferPtr& _message, int _meta_size, int64_t _data_size) :
            message(_message), meta_size(_meta_size), data_size(_data_size) {}
    };

public:
    MessageStream(NET_TYPE net_type, IoService& io_service, const Endpoint& endpoint);
    virtual ~MessageStream();

    void async_send_message(const ReadBufferPtr& message);

    void set_flow_controller(const FlowControllerPtr& flow_controller);

    void set_max_pending_buffer_size(int64_t max_pending_buffer_size);
    int64_t max_pending_buffer_size() const;
    int64_t pending_message_count() const;
    int64_t pending_data_size() const;
    int64_t pending_buffer_size() const;

    virtual bool trigger_receive() override;
    virtual bool trigger_send() override;

    int read_quota_token() const;
    int write_quota_token() const;

    void set_ssl_server_file_path(std::string& path,std::string& key_path);
    void set_ssl_client_file_path(std::string& path);

protected:
    virtual bool on_sending(const ReadBufferPtr& message) = 0;
    virtual void on_sent(const ReadBufferPtr& message) = 0;
    virtual void on_send_failed(std::string_view peason,const ReadBufferPtr& message) = 0;
    virtual void on_received(const ReadBufferPtr& message,int meta_size,int64_t data_size) = 0;
    virtual bool on_connected() override;

private:
    virtual void on_read_some(const Error_code& error, size_t bytes_transferred) override;
    virtual void on_write_some(const Error_code& error, size_t bytes_transferred) override;

    
    void put_into_pending_queue(const ReadBufferPtr& message);  
    void insert_into_pending_queue(const ReadBufferPtr& message); 
    bool get_from_pending_queue(ReadBufferPtr* message);
    
    bool try_start_receive();
    bool try_start_send();
    
    bool split_and_process_message(char* data, int size, std::deque<ReceivedItem>* received_messages);
    int  identify_message_header(char* data, int size, int* consumed);

    void clear_sending_env();
    void reset_receiving_env();
    bool reset_tran_buf();


private:
    int _pending_message_count;
    int64_t _pending_data_size;
    int64_t _pending_buffer_size;
    std::deque<PendingItem> _pending_calls;
    std::mutex              _mutex;
    std::deque<PendingItem> _swapped_calls;
    int                     _swapped_message_count;
    int64_t                 _swapped_data_size;
    int64_t                 _swapped_buffer_size;

    //流量控制
    FlowControllerPtr _flow_controller;
    int64_t           _max_pending_buffer_size;

    /**
     * 小于0 则表示没有限制
     */
    int32_t           _read_quota_token;
    int32_t           _write_quota_token;

    /**
     * 发消息时候的临时变量
     */
    ReadBufferPtr _sending_message;
    int64_t       _sent_size;                //当前发送消息的大小 (已发)
    const char*   _sending_data;
    int           _sending_size;              //当前发送数据的大小
   
        // statistics
    int64_t _total_sent_count;
    int64_t _total_sent_size;
    int64_t _total_received_count;
    int64_t _total_received_size;

    /**
     * 接收消息时的临时变量
     */
    ReadBufferPtr _receiving_message;
    int64_t       _received_message_size;   //当前已接收的消息大小
    MessageHeader _receiving_header;
    int _received_header_size;  
    bool _receiving_header_identified;        //识别标志

    
    /**
     * 数据
     */
    char* _tran_buf;
    char* _receiving_data;   //维护 _tran_buf偏移的指针
    int64_t _receiving_size; //维护 _tran_buf空间的大小


    static const int TOKEN_FREE = 0;
    static const int TOKEN_LOCK = 1;
    std::atomic<int> _send_token;
    std::atomic<int> _receive_token;

};











}