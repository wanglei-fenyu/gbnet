#include "ssl_byte_stream.h"
#include "../buffer/buffer.h"
#include"../buffer/tran_buf_pool.h"
#include "../common/atomic.h"
#include "../common/endpoint_help.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <openssl/ssl.h>
namespace gb
{

 SSLByteStream::SSLByteStream(NET_TYPE net_type,IoService& ios, const Endpoint& endpoint)
     : _io_service(ios), _net_type(net_type)
     , _ssl_context(net_type == NET_TYPE::NT_SERVER?Asio::ssl::context::tlsv12_server:Asio::ssl::context::tlsv12_client)
     , _remote_endpoint(endpoint)
	 , _ticks(0)
	 , _last_rw_ticks(0)
	 , _no_delay(true)
	 , _read_buffer_base_block_factor(TRAN_BUF_BLOCK_MAX_FACTOR)
	 , _write_buffer_base_block_factor(4)
	 , _timer(ios)
	 , _socket(ios,_ssl_context)
	 , _connect_timeout(std::chrono::duration<int64_t, std::ratio<1>>(-1))
	 , _status(NET_STSTUS::STATUS_INIT)
{
     RESOURCE_COUNTER_INC(SSLByteStream);
 }

 SSLByteStream::~SSLByteStream()
{
     Error_code ec;
     _socket.lowest_layer().close(ec);
     RESOURCE_COUNTER_DEC(SSLByteStream);
 }

bool SSLByteStream::no_delay()
{
    return _no_delay;
}

void SSLByteStream::set_no_delay(bool no_delay)
{
	_no_delay = no_delay;
}

void SSLByteStream::set_read_buffer_base_block_factor(size_t factor)
{
    _read_buffer_base_block_factor = factor;
}

size_t SSLByteStream::read_buffer_base_block_factor()
{
    return _read_buffer_base_block_factor;
}

void SSLByteStream::set_write_buffer_base_block_factor(size_t factor)
{
    _write_buffer_base_block_factor = factor;
}

size_t SSLByteStream::write_buffer_base_block_factor()
{
    return _write_buffer_base_block_factor;
}

void SSLByteStream::close(const std::string& reason)
{
	if (atomic_swap(&_status, NET_STSTUS::STATUS_CLOSED) != NET_STSTUS::STATUS_CLOSED)
	{
        Error_code ec;
        _socket.lowest_layer().shutdown(Socket::shutdown_both, ec);
        on_closed();
        if (_remote_endpoint != Endpoint())
		{
            NETWORK_LOG("connection closed: {}:{}", EndpointToString(_remote_endpoint), reason);
		}
	}

}

void SSLByteStream::on_connect_timeout(const Error_code& error)
{
    if (_status != NET_STSTUS::STATUS_CONNECTING)
        return;
    if (error == Asio::error::operation_aborted)
    {
        return;
    }
    close("connect timeout");
}

void SSLByteStream::async_connect()
{
     _last_rw_ticks = _ticks;
    
     _status = NET_STSTUS::STATUS_CONNECTING;
     _socket.lowest_layer().async_connect(_remote_endpoint, asio_bind(&SSLByteStream::on_connect,shared_from_this(),_(1)));

     if (_connect_timeout.count() > 0)
     {
         _timer.expires_after(std::chrono::duration_cast<Asio::steady_timer::duration>(_connect_timeout));
         _timer.async_wait(asio_bind(&SSLByteStream::on_connect_timeout,shared_from_this(),_(1)));
     }

}

void SSLByteStream::update_remote_endpoint()
{
    Error_code ec;
    _remote_endpoint = _socket.lowest_layer().remote_endpoint(ec);
    if (ec)
    {
        NETWORK_LOG("get remote endpoint failed : {} ",ec.message().c_str());
        close("update remote endpoint failed: " + ec.message());
    }
}

void SSLByteStream::set_socket_connected()
{
    _last_rw_ticks = _ticks;
    Error_code ec;
    _socket.lowest_layer().set_option(Asio::ip::tcp::no_delay(_no_delay), ec);
    if (ec)
    {
        NETWORK_LOG("set no_delay option failed: {} ",ec.message().c_str());
        close("set no_delay option failed: " + ec.message());
        return;
    }

    _local_endpoint = _socket.lowest_layer().local_endpoint(ec);
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

Socket& SSLByteStream::socket()
{
    return _socket.next_layer();
}

SSLSocket* SSLByteStream::ssl_socket()
{
    return &_socket;
}

IoService& SSLByteStream::ioservice()
{
    return _io_service;
}

const Endpoint& SSLByteStream::local_endpoint() const
{
    return _local_endpoint;
}

const Endpoint& SSLByteStream::remote_endpoint() const
{
    return _remote_endpoint;
}

bool SSLByteStream::is_connecting() const
{
    return _status == NET_STSTUS::STATUS_CONNECTING;
}

bool SSLByteStream::is_connected() const
{
    return _status == NET_STSTUS::STATUS_CONNECTED;
}

bool SSLByteStream::is_closed() const
{
    return _status == NET_STSTUS::STATUS_CLOSED;
}

void SSLByteStream::reset_ticks(int64_t ticks, bool update_last_rw_ticks)
{
    _ticks = ticks;
    if (update_last_rw_ticks)
    {
        _last_rw_ticks = ticks;
    }
}

int64_t SSLByteStream::last_rw_ticks()
{
    return _last_rw_ticks;
}

void SSLByteStream::set_connect_timeout(int64_t timeout)
{
    auto duration_ms = std::chrono::milliseconds(timeout);
    _connect_timeout = std::chrono::duration_cast<std::chrono::steady_clock::duration>(duration_ms);
}

int64_t SSLByteStream::connect_timeout()
{
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(_connect_timeout);
    return milliseconds.count();
}

void SSLByteStream::set_ssl_server_file_path_impl(std::string& path, std::string& key_path)
{
     try
     {
		 _ssl_context.set_options(  Asio::ssl::context::default_workarounds |
								    Asio::ssl::context::no_sslv2 |
								    Asio::ssl::context::no_sslv3 |
								    Asio::ssl::context::tlsv12 );

         if (_net_type == NET_TYPE::NT_SERVER)
         {
             _ssl_context.use_certificate_chain_file(path);
             _ssl_context.use_private_key_file(key_path, Asio::ssl::context::pem);
         }
         _socket = SSLSocket(_io_service, _ssl_context);

     }
     catch(const std::exception& e) {
            NETWORK_LOG("SSL configuration error: {}", e.what());
            throw;
     }
}

void SSLByteStream::set_ssl_client_file_path_impl(std::string& path)
{
     try
     {
		 _ssl_context.set_options(  Asio::ssl::context::default_workarounds |
								    Asio::ssl::context::no_sslv2 |
								    Asio::ssl::context::no_sslv3 |
								    Asio::ssl::context::tlsv12 );

         if (_net_type == NET_TYPE::NT_CLIENT)
         {
             _ssl_context.load_verify_file(path);
             _ssl_context.set_verify_mode(Asio::ssl::verify_peer);

         }
         _socket = SSLSocket(_io_service, _ssl_context);

     }
     catch(const std::exception& e) {
            NETWORK_LOG("SSL configuration error: {}", e.what());
            throw;
     }
}


void SSLByteStream::async_read_some(char* data, size_t size)
{
    _socket.async_read_some(Asio::buffer(data,size),asio_bind(&SSLByteStream::on_read_some,shared_from_this(),_(1),_(2)));
}

void SSLByteStream::async_write_some(const char* data, size_t size)
{
    _socket.async_write_some(Asio::buffer(data,size),asio_bind(&SSLByteStream::on_write_some,shared_from_this(),_(1),_(2)));
}

void SSLByteStream::on_connect(const Error_code& error)
{
    _socket.async_handshake(Asio::ssl::stream_base::client,[this](const Error_code& error) {
        if (!error)
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
			_socket.lowest_layer().set_option(Asio::ip::tcp::no_delay(_no_delay), ec);
			if (ec)
			{
				NETWORK_LOG("on_connect(): set no_delay option failed: {} ",error.message());
				close("on_connect(): set no_delay option failed:"  + error.message());
				return;
			}

			_local_endpoint = _socket.lowest_layer().local_endpoint(ec);
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
        else
        {
			NETWORK_LOG("handshake error: {}",error.message());
		    close("handshake faileda:"  + error.message());
        }

    });
}

}