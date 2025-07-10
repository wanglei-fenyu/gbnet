#pragma once
#include "app_def.h"
#include "../common/worker/worker_manager.h"
#include <memory>
#include <chrono>
#include <atomic>
#include "log/log_help.h"

class App{
public:
    App(APP_TYPE type) :
        appType_(type), runding_(false), frame_duration_(std::chrono::milliseconds(16)) {}
	virtual ~App() {}
	APP_TYPE GetAppType() { return appType_; }
    void SetFrameRate(int fps);
    int Init();
    void Stop();
    void Run();

protected:
	virtual int OnInit() = 0;
    virtual int OnStartup(gb::WorkerPtr) = 0;
    virtual int OnUpdate(gb::WorkerPtr) = 0;
    virtual int OnTick(gb::WorkerPtr, float) = 0;
    virtual int OnCleanup(gb::WorkerPtr) = 0;
    virtual int OnUnInit() = 0;

private:
	APP_TYPE appType_;
    std::atomic<bool> runding_;
    std::chrono::milliseconds frame_duration_;
};
