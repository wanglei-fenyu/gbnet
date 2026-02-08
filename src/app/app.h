#pragma once
#include "app_def.h"
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
    virtual int OnStartup() = 0;
    virtual int OnUpdate(float) = 0;
    virtual int OnTick() = 0;
    virtual int OnCleanup() = 0;
    virtual int OnUnInit() = 0;

private:
	APP_TYPE appType_;
    std::atomic<bool> runding_;
    std::chrono::milliseconds frame_duration_;

protected:
    std::atomic<uint64_t>     tick_id_;
    std::mutex                cvMutex_;
    std::condition_variable   cv_;
    ;
};
