#include "MyApp.h"
#include "test.h"
#include "common/res_path.h"
#include "network/net_manager/network_manager.h"
int MyApp::OnInit()
{
	log.Init(ResPath::Instance()->FindResPath("log4/test.log").c_str(), 1024 * 1024 * 1000, 10,
             GbLog::ASYNC, GbLog::CONSOLE_AND_FILE, GbLog::LEVEL_INFO);

    gb::WorkerManager* work_mng = gb::WorkerManager::Instance(3);

	auto [ip, port] = AppTypeMgr::Instance()->GetServerIpPort();
	std::string uir = ip + ":" + port;
	gb::ServerOptions options;
    options.keep_alive_time = -1;
    options.io_service_pool_size = 1;
    server_ = std::make_unique<gb::Server>(options);

    server_->SetConnnectCallBack([](const gb::SessionPtr& session) {
        LOG_INFO("Accept:{}", session->socket().local_endpoint().address().to_string());
    });
    server_->SetCloseCallBack([](const gb::SessionPtr& session) {
        LOG_INFO("Close:{}", session->socket().local_endpoint().address().to_string());
    });
    gb::NetworkManager::Instance()->Init(server_.get());
    Test_Register();


    server_->Start(uir);
    return 0;
}

int MyApp::OnStartup(gb::WorkerPtr worker)
{
    if (worker)
        worker->Post([worker]() { worker->OnStartup();});
    return 0;
}

int MyApp::OnUpdate(gb::WorkerPtr worker)
{
    if (worker)
        worker->Post([worker]() { worker->OnUpdate(); });
    return 0;
}

int MyApp::OnTick(gb::WorkerPtr worker, float elapsed)
{
    if (worker)
        worker->Post([worker,elapsed]() { worker->OnTick(elapsed); });
    return 0;

}

int MyApp::OnCleanup(gb::WorkerPtr worker)
{
    if (worker)
        worker->Post([worker]() { worker->OnCleanup(); });
    return 0;
}

int MyApp::OnUnInit()
{
    log.UnInit();
    http_thread.join();
    return 0;
}

void MyApp::init_http()
{
}
