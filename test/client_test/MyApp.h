#pragma once
#include "app/app.h"
#include "network/net/client.h"

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
    void test_http();

private:
    //gb::http::HttpClientPtr     http_client;
    std::shared_ptr<gb::Client> client_;
	GbLog log;

};