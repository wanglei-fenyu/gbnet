#pragma once
#include <string.h>
#include "define.h"
namespace gb
{

	std::string EndpointToString(const Endpoint& endpoint);

	std::string HostOfEndpoint(const Endpoint& endpoint);

	uint32_t PortOfEndpoint(const Endpoint& endpoint);


	// 将服务器地址解析为 endpoint_t，仅选择第一个。
	// @param io_service 是用于解析的 IO 服务。
	// @param host 可以是 IP 或主机名，例如 "127.0.0.1" 或 "baidu.com"。
	// @param svc 可以是端口或服务名称，例如 "21" 或 "ftp"。
	// @param endpoints 是输出参数，如果解析成功，则存储解析的 endpoint_t，但可能为空。
	// @return 如果解析成功，则返回 true。
	// @return 如果解析失败，则返回 false。
    bool ResolveAddress(IoService& io_service, const std::string& host, const std::string& svc, Endpoint* endpoint);

	// 将服务器地址解析为 endpoint_t，仅选择第一个。
	// @param io_service 是用于解析的 IO 服务。
	// @param server address 应为 "host:port" 格式。
	// @param endpoint 是输出参数，如果解析成功，则存储解析的 endpoint_t。
	// @return 如果解析成功，则返回 true。
	// @return 如果解析失败或未找到地址，则返回 false。
    bool ResolveAddress(IoService& io_service, const std::string& address, Endpoint* endpoint);

   } // namespace gb