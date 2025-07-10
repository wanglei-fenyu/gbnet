#include "MyApp.h"
#include "test.h"
#include "network.h"
#include "common/res_path.h"

static bool is_net_init = false;




async_simple::coro::Lazy<> test_coro_2(const gb::SessionPtr& session)
{
	gb::RpcCallPtr call = std::make_shared<gb::RpcCall>();
	call->SetSession(session);
    //co_await gb::CoRpc<>::execute(call, "test_rpc");

    int num = co_await gb::CoRpc<int>::execute(call, "square", 10000);
    LOG_INFO("CORO_TEST  {}", num);

    auto [a, b] = co_await gb::CoRpc<int, std::string>::execute(call, "test_ret_args", 2, "world");
    LOG_INFO("coro_test_2  {} {}", a, b);
}


int MyApp::OnInit()
{
	log.Init(ResPath::Instance()->FindResPath("log4/test.log").c_str(), 1024 * 1024 * 1000, 10,
		   GbLog::ASYNC, GbLog::CONSOLE_AND_FILE, GbLog::LEVEL_INFO);

    gb::WorkerManager* work_mng = gb::WorkerManager::Instance(4);
    gb::ClientOptions options;
    options.keep_alive_time = -1;
    client_.reset(new gb::Client(options));
    Test_Register();

	client_->SetCloseCallBack([](const gb::SessionPtr session) {
        LOG_INFO("net close");
    });

	client_->SetConnnectCallBack([this](const gb::SessionPtr session) {
        session->set_return_io_service_pool_fun([&]()-> gb::IoServicePoolPtr {
            return client_->GetIoServicePool();
        });
        LOG_INFO("net connect");
        is_net_init = true;
        //session->StartHeartbeat(std::chrono::seconds(2));
        
        auto t1 = gb::WorkerManager::Instance()->GetWorker(2)->GetTimerManager()->RegisterTimer(
            6000, []() {
                LOG_ERROR("t1");
            },
            false);
        gb::WorkerManager::Instance()->GetWorker(2)->GetTimerManager()->UnRegisterTimer(t1);
        auto t2 = gb::WorkerManager::Instance()->GetWorker(2)->GetTimerManager()->RegisterTimer(
            2000, []() {
                LOG_ERROR("t2");
            },
            true);
        auto t3 = gb::WorkerManager::Instance()->GetWorker(2)->GetTimerManager()->RegisterTimer(
            10000, []() {
                LOG_ERROR("t3");
            },
            false);

        SendMsg1(client_);
        gb::WorkerManager::Instance()->GetWorker(2)->Post([this]() {
           //async_simple::coro::syncAwait(test_coro_2(client_->GetSession(gb::CONNECT_TYPE::CT_GATEWAY)));
           // SendRpc(client_);
            test_coro_2(client_->GetSession(gb::CONNECT_TYPE::CT_GATEWAY)).start([](auto&&) {});
            });
    });

    
	client_->SetReceivedCallBack(gb::OnReceiveCall);

	auto [ip, port] = AppTypeMgr::Instance()->GetServerIpPort();
	std::string uir = ip + ":" + port;
    client_->Connect(gb::CONNECT_TYPE::CT_GATEWAY, uir);
    test_http();
    return 0;
}

int MyApp::OnStartup(gb::WorkerPtr worker)
{
    if (worker)
        worker->Post([worker]() {worker->OnStartup();});
      
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

    if (is_net_init)
    {
        is_net_init = false;
        //SendMsg1(client_);
       // SendRpc(client_);
  //       auto worker = client_->GetIoServicePool()->GetWorker(client_->GetSession(gb::CONNECT_TYPE::CT_GATEWAY)->GetIoServicePoolIndex());
		//if (worker.has_value())
		//{
  //          worker.value()->Post([this]() mutable
		//		{
  //                  async_simple::coro::syncAwait(test_coro_2(client_->GetSession(gb::CONNECT_TYPE::CT_GATEWAY)));
		//		});
		//}
        //
        // async_simple::coro::syncAwait(test_coro_2(client_->GetSession(gb::CONNECT_TYPE::CT_GATEWAY)));

    }
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
    client_->Shutdown();
    log.UnInit();
    return 0;
}

void MyApp::test_http()
{

}
