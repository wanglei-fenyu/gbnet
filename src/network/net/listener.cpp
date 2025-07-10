#include "listener.h"
#include "common/endpoint_help.h"

namespace gb
{
Listener::Listener(IoService& io, IoServicePoolPtr& io_service_pool, const Endpoint& endpoint)
	 : _io_service_pool(io_service_pool), _ios(io)
	 , _endpoint(endpoint)
	// , _ios(io_service_pool->GetIoService().second)
	 , _acceptor(_ios)
	 , _is_closed(false)
{
}
Listener::Listener(IoServicePoolPtr& io_service_pool, const Endpoint& endpoint) 
	 : _io_service_pool(io_service_pool)
	 , _endpoint(endpoint)
	 , _ios(io_service_pool->GetIoService().second)
	 , _acceptor(_ios)
	 , _is_closed(false)
{
    RESOURCE_COUNTER_INC(Listener);
}

//Listener(IoService& ios,IoServicePoolPtr& io_service_pool, const Endpoint& endpoint)
//     : _ios(ios)
//     , _io_service_pool(io_service_pool)
//	 , _endpoint(endpoint)
//	 , _acceptor(_ios)
//	 , _is_closed(false)
//{
//    RESOURCE_COUNTER_INC(Listener);
//}

 Listener::~Listener()
{
    RESOURCE_COUNTER_DEC(Listener);
}

void Listener::close()
{
	 std::lock_guard<std::mutex> _lock(_close_mutex);
    if (_is_closed.load()) return;
     _is_closed.store(true);

	 Error_code ec;
     _acceptor.cancel(ec);
     _acceptor.close(ec);
	 
	 NETWORK_LOG("close(): listener closed: {}", EndpointToString(_endpoint));
}

bool Listener::is_close()
{
	std::lock_guard<std::mutex> _lock(_close_mutex);
    return _is_closed.load();
}

void Listener::set_create_callback(const callback_t& create_callback)
{
    _create_callback = create_callback;
}

void Listener::set_accept_callback(const callback_t& accept_callback)
{
    _accept_callback = accept_callback;
}

void Listener::set_accept_fail_callback(const fail_callback_t& accept_fail_callback)
{
    _accept_fail_callback = accept_fail_callback;
}

bool Listener::start_listen()
{
    Error_code ec;
    _acceptor.open(_endpoint.protocol(),ec);
    if (ec)
    {
        NETWORK_LOG("start_listen(): open acceptor failed: {}: {}", EndpointToString(_endpoint), ec.message());
        return false;
    }

    _acceptor.set_option(Acceptor::reuse_address(true), ec);
    if (ec)
    {
        NETWORK_LOG("start_listen(): set acceptor option failed: {}: {}", EndpointToString(_endpoint), ec.message());
        return false;
    }

    _acceptor.bind(_endpoint, ec);
    if (ec)
    {
        NETWORK_LOG("start_listen(): bind acceptor failed: {}: {}", EndpointToString(_endpoint), ec.message());
        return false;
    }

    _acceptor.listen(LISTEN_MAX_CONNECTIONS, ec);
    if (ec)
    {
        NETWORK_LOG("start_listen(): listen acceptor failed: {}: {}", EndpointToString(_endpoint), ec.message());
        return false;
    }
    
	NETWORK_LOG("start_listen(): listen succeed: {}", EndpointToString(_endpoint));

    _is_closed.store(false);
    async_accept();
    return true;
    
}

void Listener::async_accept()
{
    auto [io_service_index, ios] = _io_service_pool->GetIoService();
    SessionPtr session = std::make_shared<Session>(NET_TYPE::NT_SERVER, ios, Endpoint());
    //session->SetIoServicePoolIndex(io_service_index);
    if (_create_callback)
        _create_callback(session);
    
    _acceptor.async_accept(session->socket(), asio_bind(&Listener::on_accept, shared_from_this(), session, _(1)));
}

void Listener::on_accept(const SessionPtr& session, const Error_code& ec)
{
    if (_is_closed.load())
        return;

    if (ec)
    {
		NETWORK_LOG("on_accept(): accept error at: {} : {}", EndpointToString(_endpoint),ec.message());
        close();
        if (_accept_fail_callback)
        {
            NET_ErrorCode net_ec = ec == Asio::error::no_descriptors ? NET_ERROR_TOO_MANY_OPEN_FILES : NET_ERROR_UNKNOWN;
            _accept_fail_callback(net_ec, ec.message());
        }
    }
    else
    {
        if (session->is_ssl_socket())
        {
            auto ssl_sock = session->ssl_socket();
            if (ssl_sock)
            {
				session->update_remote_endpoint();
                ssl_sock->async_handshake(Asio::ssl::stream_base::server, [this, session](const Error_code& error) {
                    if (!error)
                    {
                        if (!session->is_closed() && _accept_callback)
                        {
                            _accept_callback(session);
                        }

                        if (!session->is_closed())
                        {
                            session->set_socket_connected();
                        }

                        if (!session->is_closed())
                            NETWORK_LOG("on_accept(): accept connection at: {} : {}", EndpointToString(_endpoint), EndpointToString(session->remote_endpoint()));
                    }
                    else
                    {
                        NETWORK_LOG("handshake: {} : {} error:{}", EndpointToString(_endpoint), EndpointToString(session->remote_endpoint()), error.message());
                    }
                });
            }
            else
            {
                NETWORK_LOG("ssl::sock  not find ssl::socket point");
            }
        }
        else
        {

			if (!session->is_closed() && _accept_callback)
			{
				_accept_callback(session);
			}

			if (!session->is_closed())
			{
				session->set_socket_connected();
			}
			
			if (!session->is_closed())
				NETWORK_LOG("on_accept(): accept connection at: {} : {}", EndpointToString(_endpoint),EndpointToString(session->remote_endpoint()));
        }
		async_accept();
    
    }

}

}