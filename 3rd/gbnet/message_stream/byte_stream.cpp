#include "byte_stream.h"
#include "../buffer/buffer.h"
#include"../buffer/tran_buf_pool.h"
#include "../common/atomic.h"
#include "../common/endpoint_help.h"
namespace gb
{



 ByteStream::ByteStream(NET_TYPE net_type,IoService& ios, const Endpoint& endpoint)
	 : _io_service(ios)
     , _net_type(net_type)
	 , _remote_endpoint(endpoint)
	 , _ticks(0)
	 , _last_rw_ticks(0)
	 , _no_delay(true)
	 , _read_buffer_base_block_factor(TRAN_BUF_BLOCK_MAX_FACTOR)
	 , _write_buffer_base_block_factor(4)
	 , _timer(ios)
	 , _socket(ios)
	 , _connect_timeout(std::chrono::duration<int64_t, std::ratio<1>>(-1))
	 , _status(NET_STSTUS::STATUS_INIT)
{
     RESOURCE_COUNTER_INC(ByteStream);
 }

 ByteStream::~ByteStream()
{
     Error_code ec;
     _socket.close(ec);
     RESOURCE_COUNTER_DEC(ByteStream);
 }

bool ByteStream::no_delay()
{
    return _no_delay;
}

void ByteStream::set_no_delay(bool no_delay)
{
	_no_delay = no_delay;
}

void ByteStream::set_read_buffer_base_block_factor(size_t factor)
{
    _read_buffer_base_block_factor = factor;
}

size_t ByteStream::read_buffer_base_block_factor()
{
    return _read_buffer_base_block_factor;
}

void ByteStream::set_write_buffer_base_block_factor(size_t factor)
{
    _write_buffer_base_block_factor = factor;
}

size_t ByteStream::write_buffer_base_block_factor()
{
    return _write_buffer_base_block_factor;
}

void ByteStream::close(const std::string& reason)
{
	if (atomic_swap(&_status, NET_STSTUS::STATUS_CLOSED) != NET_STSTUS::STATUS_CLOSED)
	{
        Error_code ec;
        _socket.shutdown(Socket::shutdown_both, ec);
        on_closed();
        if (_remote_endpoint != Endpoint())
		{
            NETWORK_LOG("connection closed: {}:{}", EndpointToString(_remote_endpoint), reason);
		}
	}

}

void ByteStream::on_connect_timeout(const Error_code& error)
{
    if (_status != NET_STSTUS::STATUS_CONNECTING)
        return;
    if (error == Asio::error::operation_aborted)
    {
        return;
    }
    close("connect timeout");
}

void ByteStream::async_connect()
{
     _last_rw_ticks = _ticks;
    
     _status = NET_STSTUS::STATUS_CONNECTING;
     _socket.async_connect(_remote_endpoint, asio_bind(&ByteStream::on_connect,shared_from_this(),_(1)));

     if (_connect_timeout.count() > 0)
     {
         _timer.expires_after(std::chrono::duration_cast<Asio::steady_timer::duration>(_connect_timeout));
         _timer.async_wait(asio_bind(&ByteStream::on_connect_timeout,shared_from_this(),_(1)));
     }

}

void ByteStream::update_remote_endpoint()
{
    Error_code ec;
    _remote_endpoint = _socket.remote_endpoint(ec);
    if (ec)
    {
        NETWORK_LOG("get remote endpoint failed : {} ",ec.message().c_str());
        close("update remote endpoint failed: " + ec.message());
    }
}

void ByteStream::set_socket_connected()
{
    _last_rw_ticks = _ticks;
    Error_code ec;
    _socket.set_option(Asio::ip::tcp::no_delay(_no_delay), ec);
    if (ec)
    {
        NETWORK_LOG("set no_delay option failed: {} ",ec.message().c_str());
        close("set no_delay option failed: " + ec.message());
        return;
    }

    _local_endpoint = _socket.local_endpoint(ec);
    if (ec)
    {
        NETWORK_LOG("get local endpoint failed: {} ",ec.message().c_str());
        close("get local endpoint failed: " + ec.message());
        return;
    }

    if (!on_connected())
    {
        NETWORK_LOG("call on_connected() failed: {} ",ec.message().c_str());
        close("call on_connected() failed: " + ec.message());
        return;
    }

    _status = NET_STSTUS::STATUS_CONNECTED;
    trigger_receive();
    trigger_send();

}

Socket& ByteStream::socket()
{
    return _socket;
}

SSLSocket* ByteStream::ssl_socket()
{
    return nullptr;
}

IoService& ByteStream::ioservice()
{
    return _io_service;
}

const Endpoint& ByteStream::local_endpoint() const
{
    return _local_endpoint;
}

const Endpoint& ByteStream::remote_endpoint() const
{
    return _remote_endpoint;
}

bool ByteStream::is_connecting() const
{
    return _status == NET_STSTUS::STATUS_CONNECTING;
}

bool ByteStream::is_connected() const
{
    return _status == NET_STSTUS::STATUS_CONNECTED;
}

bool ByteStream::is_closed() const
{
    return _status == NET_STSTUS::STATUS_CLOSED;
}

void ByteStream::reset_ticks(int64_t ticks, bool update_last_rw_ticks)
{
    _ticks = ticks;
    if (update_last_rw_ticks)
    {
        _last_rw_ticks = ticks;
    }
}

int64_t ByteStream::last_rw_ticks()
{
    return _last_rw_ticks;
}

void ByteStream::set_connect_timeout(int64_t timeout)
{
    auto duration_ms = std::chrono::milliseconds(timeout);
    _connect_timeout = std::chrono::duration_cast<std::chrono::steady_clock::duration>(duration_ms);
}

int64_t ByteStream::connect_timeout()
{
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(_connect_timeout);
    return milliseconds.count();
}


void ByteStream::async_read_some(char* data, size_t size)
{
    _socket.async_read_some(Asio::buffer(data,size),asio_bind(&ByteStream::on_read_some,shared_from_this(),_(1),_(2)));
}

void ByteStream::async_write_some(const char* data, size_t size)
{
    _socket.async_write_some(Asio::buffer(data,size),asio_bind(&ByteStream::on_write_some,shared_from_this(),_(1),_(2)));
}

void ByteStream::on_connect(const Error_code& error)
{
    if (_status != NET_STSTUS::STATUS_CONNECTING)
    {
        return;
    }
    if (error)
    {
        NETWORK_LOG("on_connect(): connect error: {} ",error.message());
        close("on_connect(): connect error:"  + error.message());
        return;
    }

    Error_code ec;
    _socket.set_option(Asio::ip::tcp::no_delay(_no_delay), ec);
    if (ec)
    {
        NETWORK_LOG("on_connect(): set no_delay option failed: {} ",error.message());
        close("on_connect(): set no_delay option failed:"  + error.message());
        return;
    }

    _local_endpoint = _socket.local_endpoint(ec);
    if (ec)
    {
        NETWORK_LOG("get local endpoint failed: {} ",error.message());
        close("get local endpoint failed:"  + error.message());
        return;
    }
    
    if (!on_connected())
    {
        NETWORK_LOG("on_connect(): call on_connected() failed: {} ",error.message());
        close("on_connect(): call on_connected() failed:"  + error.message());
        return;
    }

    _status = NET_STSTUS::STATUS_CONNECTED;
    _timer.cancel();
    
    trigger_receive();
    trigger_send();
}

}