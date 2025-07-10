#pragma once
#include <string.h>
#include "define.h"
namespace gb
{

	std::string EndpointToString(const Endpoint& endpoint);

	std::string HostOfEndpoint(const Endpoint& endpoint);

	uint32_t PortOfEndpoint(const Endpoint& endpoint);


	// ����������ַ����Ϊ endpoint_t����ѡ���һ����
	// @param io_service �����ڽ����� IO ����
	// @param host ������ IP �������������� "127.0.0.1" �� "baidu.com"��
	// @param svc �����Ƕ˿ڻ�������ƣ����� "21" �� "ftp"��
	// @param endpoints �������������������ɹ�����洢������ endpoint_t��������Ϊ�ա�
	// @return ��������ɹ����򷵻� true��
	// @return �������ʧ�ܣ��򷵻� false��
    bool ResolveAddress(IoService& io_service, const std::string& host, const std::string& svc, Endpoint* endpoint);

	// ����������ַ����Ϊ endpoint_t����ѡ���һ����
	// @param io_service �����ڽ����� IO ����
	// @param server address ӦΪ "host:port" ��ʽ��
	// @param endpoint �������������������ɹ�����洢������ endpoint_t��
	// @return ��������ɹ����򷵻� true��
	// @return �������ʧ�ܻ�δ�ҵ���ַ���򷵻� false��
    bool ResolveAddress(IoService& io_service, const std::string& address, Endpoint* endpoint);

   } // namespace gb