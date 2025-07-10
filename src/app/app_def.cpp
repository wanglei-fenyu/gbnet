#include "app_def.h"
#include "../common/res_path.h"
#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_utils.hpp"

AppTypeMgr::AppTypeMgr()
{
	_names.insert(std::make_pair(APP_TYPE::APP_Global, "global"));

	_names.insert(std::make_pair(APP_TYPE::APP_DB_MGR, "dbmgr"));

	_names.insert(std::make_pair(APP_TYPE::APP_GAME_MGR, "gamemgr"));
	_names.insert(std::make_pair(APP_TYPE::APP_SPACE_MGR, "spacemgr"));

	_names.insert(std::make_pair(APP_TYPE::APP_APPMGR, "appmgr"));

	_names.insert(std::make_pair(APP_TYPE::APP_LOGIN, "login"));
	_names.insert(std::make_pair(APP_TYPE::APP_GAME, "game"));
	_names.insert(std::make_pair(APP_TYPE::APP_SPACE, "space"));
	_names.insert(std::make_pair(APP_TYPE::APP_ROBOT, "robot"));

	_names.insert(std::make_pair(APP_TYPE::APP_ALL, "all"));
}

std::string AppTypeMgr::GetAppName(const APP_TYPE appType)
{
	const auto iter = _names.find(appType);
	if (iter == _names.end())
		return "";

	return iter->second;
}

std::pair<std::string, std::string> AppTypeMgr::GetServerIpPort(int os_type)
{
	rapidxml::file<> file(ResPath::Instance()->FindResPath("config/server_config.xml").c_str());
	rapidxml::xml_document<> doc;
	doc.parse<0>(file.data());
	rapidxml::xml_node<> *root = doc.first_node();
	std::string ip;
	std::string port;
	std::string uir_name;

	switch (os_type)
	{
	case UIR_TYPE::UT_None:

#if ENGINE_PLATFORM != PLATFORM_WIN32		
		uir_name = "linux_tcp";
#else
        uir_name = "win_tcp";
#endif
		break;
	case UIR_TYPE::UT_WIN_TCP:
        uir_name = "win_tcp";
		break;
    case UIR_TYPE::UT_LINUX_TCP:
        uir_name = "linux_tcp";
		break;
    case UIR_TYPE::UT_WIN_HTTP:
        uir_name = "win_http";
		break;
    case UIR_TYPE::UT_LINUX_HTTP:
        uir_name = "linux_http";
		break;
	default:
		break;
	}

	rapidxml::xml_node<>* node = root->first_node(uir_name.c_str());
	if (node)
	{
		auto ip_attr = node->first_attribute("ip");
		if (ip_attr)
			ip = std::string(ip_attr->value());
		auto port_attr = node->first_attribute("port");
		if (port_attr)
			port = std::string(port_attr->value());
	}
	
	return { ip,port };
}
