#pragma once
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <string>
#include <functional>
#include <memory>
#include <coroutine>

namespace net  = boost::asio;
using tcp = net::ip::tcp;
namespace beast = boost::beast;
namespace http  = beast::http;

struct HttpResponse {
    int status;
    std::string body;
};

class HttpClient : public std::enable_shared_from_this<HttpClient> {
public:
    using Callback = std::function<void(HttpResponse)>;

    explicit HttpClient(net::io_context& ioc);

    // 协程接口
    net::awaitable<HttpResponse> GetAsync(const std::string& host,
                                     const std::string& port,
                                     const std::string& target);

    net::awaitable<HttpResponse> PostAsync(const std::string& host,
                                      const std::string& port,
                                      const std::string& target,
                                      const std::string& body,
                                      const std::string& content_type = "application/json");

    // 回调接口
    void Get(const std::string& host, const std::string& port,
             const std::string& target, Callback cb);

    void Post(const std::string& host, const std::string& port,
              const std::string& target, const std::string& body,
              Callback cb, const std::string& content_type = "application/json");

private:
    net::io_context& ioc_;

    net::awaitable<HttpResponse> DoRequest(http::verb method,
                                           const std::string& host,
                                           const std::string& port,
                                           const std::string& target,
                                           const std::string& body = "",
                                           const std::string& content_type = "application/json");
};
