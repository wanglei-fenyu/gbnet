#include "MyApp.h"
#include "network/network.h"
#include "test.h"
#include "common/res_path.h"
int MyApp::OnInit()
{
    int*      i = (int*)malloc(sizeof(int));
	log.Init(ResPath::Instance()->FindResPath("log4/test.log").c_str(), 1024 * 1024 * 1000, 10,
             GbLog::ASYNC, GbLog::CONSOLE_AND_FILE, GbLog::LEVEL_INFO);

    gb::WorkerManager* work_mng = gb::WorkerManager::Instance(4);
    gb::net_init();
    init_http();
    Test_Register();
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
