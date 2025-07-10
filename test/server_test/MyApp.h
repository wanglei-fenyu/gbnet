#pragma once
#include "app/app.h"
#include "common/worker/worker.h"
#include "log/log_help.h"
#define  SERVER_APP true
class MyApp :public App
{
public:
    MyApp(APP_TYPE type) :
        App(type) {}
    ~MyApp(){};

protected:
	virtual int OnInit();
	virtual int OnStartup(gb::WorkerPtr);
	virtual int OnUpdate(gb::WorkerPtr);
	virtual int OnTick(gb::WorkerPtr, float);
	virtual int OnCleanup(gb::WorkerPtr);
    virtual int OnUnInit();

private:
    void init_http();

private:
    std::thread http_thread;
    //gb::http::HttpServerPtr http_server;
    GbLog                log;

};

