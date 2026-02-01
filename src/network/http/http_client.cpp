#include "http_client.h"

HttpClient::HttpClient(net::io_context& ioc) : ioc_(ioc) {}

// ---------------- 协程接口 ----------------
net::awaitable<HttpResponse> HttpClient::GetAsync(const std::string& host,
                                             const std::string& port,
                                             const std::string& target) {
    co_return co_await DoRequest(http::verb::get, host, port, target, "");
}

net::awaitable<HttpResponse> HttpClient::PostAsync(const std::string& host,
                                              const std::string& port,
                                              const std::string& target,
                                              const std::string& body,
                                              const std::string& content_type) {
    co_return co_await DoRequest(http::verb::post, host, port, target, body, content_type);
}

// ---------------- 回调接口 ----------------
void HttpClient::Get(const std::string& host, const std::string& port,
                     const std::string& target, Callback cb) {
    auto self = shared_from_this();
    net::co_spawn(ioc_,
        [self, host, port, target, cb]() -> net::awaitable<void> {
            try {
                auto res = co_await self->GetAsync(host, port, target);
                if (cb) cb(res);
            } catch (const std::exception& e) {
                if (cb) cb(HttpResponse{-1, e.what()});
            }
            co_return;
        }, net::detached);
}

void HttpClient::Post(const std::string& host, const std::string& port,
                      const std::string& target, const std::string& body,
                      Callback cb, const std::string& content_type) {
    auto self = shared_from_this();
    net::co_spawn(ioc_,
        [self, host, port, target, body, cb, content_type]() -> net::awaitable<void> {
            try {
                auto res = co_await self->PostAsync(host, port, target, body, content_type);
                if (cb) cb(res);
            } catch (const std::exception& e) {
                if (cb) cb(HttpResponse{-1, e.what()});
            }
            co_return;
        }, net::detached);
}

// ---------------- 内部实现 ----------------
net::awaitable<HttpResponse> HttpClient::DoRequest(http::verb method,
                                                   const std::string& host,
                                                   const std::string& port,
                                                   const std::string& target,
                                                   const std::string& body,
                                                   const std::string& content_type) {
    tcp::resolver resolver(ioc_);
    beast::tcp_stream stream(ioc_);

    // resolve
    auto results = co_await resolver.async_resolve(host, port, net::use_awaitable);

    // connect
    co_await stream.async_connect(results, net::use_awaitable);

    // 构造请求
    if (method == http::verb::get) {
        http::request<http::empty_body> req{method, target, 11};
        req.set(http::field::host, host);
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        co_await http::async_write(stream, req, net::use_awaitable);
    } else { // POST
        http::request<http::string_body> req{method, target, 11};
        req.set(http::field::host, host);
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        req.set(http::field::content_type, content_type);
        req.body() = body;
        req.prepare_payload();
        co_await http::async_write(stream, req, net::use_awaitable);
    }

    // 读取响应
    beast::flat_buffer buffer;
    http::response<http::string_body> res;
    co_await http::async_read(stream, buffer, res, net::use_awaitable);

    // 关闭连接
    beast::error_code ec;
    stream.socket().shutdown(tcp::socket::shutdown_both, ec);

    co_return HttpResponse{(int)res.result_int(), res.body()};
}
