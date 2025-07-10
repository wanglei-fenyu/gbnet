#pragma once 

#include <deque>
#include <memory>
#include <string>
#include "../common/define.h"


namespace gb
{

//using namespace Asio;
//using Asio::ip::tcp;

class SSLByteStream : public std::enable_shared_from_this<SSLByteStream>
{
public:
    SSLByteStream(NET_TYPE net_type,IoService& ios, const Endpoint& endpoint);
    virtual ~SSLByteStream();

    bool no_delay();
    void   set_no_delay(bool no_delay);
    void set_read_buffer_base_block_factor(size_t factor);
    size_t read_buffer_base_block_factor();
    void set_write_buffer_base_block_factor(size_t factor);
    size_t write_buffer_base_block_factor();

    void close(const std::string& reason);

    void on_connect_timeout(const Error_code& error);
    
    void async_connect();

    void update_remote_endpoint();

    void set_socket_connected();

    Socket& socket();
    SSLSocket* ssl_socket();
    IoService& ioservice();

    const Endpoint&   local_endpoint() const;
    const Endpoint& remote_endpoint() const;

    bool is_connecting() const;
    bool is_connected() const;
    bool is_closed() const;


    void reset_ticks(int64_t ticks, bool update_last_rw_ticks);
    int64_t last_rw_ticks();

    void    set_connect_timeout(int64_t timeout /*����*/);
    int64_t connect_timeout();

    bool is_ssl_socket() { return true; }
    void set_ssl_server_file_path_impl(std::string& path,std::string& key_path);
    void set_ssl_client_file_path_impl(std::string& path);
public:
    virtual bool trigger_receive() = 0;         //���������� �����ɹ����� true
    virtual bool trigger_send() = 0;            //����д���� �����ɹ����� true
    
    void async_read_some(char* data, size_t size);           //������
    void async_write_some(const char* data, size_t size);    //д����       

    virtual bool on_connected() = 0;

    virtual void on_closed() = 0;

    virtual void on_read_some(const Error_code& error,std::size_t bytes_transferred) = 0;

    virtual void on_write_some(const Error_code& error,std::size_t bytes_transferred) = 0;
private:
    void on_connect(const Error_code& error);

protected:
    NET_TYPE _net_type;

protected:
    IoService&                            _io_service;
    SSLContext                            _ssl_context;
    Endpoint                              _local_endpoint;
    Endpoint                              _remote_endpoint;
    int64_t _ticks;
    int64_t _last_rw_ticks;
    bool                                  _no_delay;

    size_t _read_buffer_base_block_factor;
    size_t _write_buffer_base_block_factor;

private:
    Asio::steady_timer _timer;
    SSLSocket        _socket;
    
    std::chrono::steady_clock::duration _connect_timeout;       //ʹ�ú���
    std::atomic<NET_STSTUS>             _status;
};


}
