#include <chrono>
#include <functional>


namespace gb
{

class Timer
{
    using TimerCallBack = std::function<void()> ;
    friend class TimerManager;

public:
    Timer(int64_t id, bool is_loop, std::chrono::milliseconds duration, TimerCallBack func);
    virtual ~Timer() = default;

    int64_t Id() const;
    bool    IsLoop() const;
    void    Cancel();
    virtual bool IsExpired() const = 0;
    virtual int64_t RemainingTime() const = 0;
    int64_t         DurationTime() const;

protected:
    virtual void Reset() = 0;
    void operator()() const;

protected:
    bool	active_;
    int64_t id_;
    bool    loop_;
    std::chrono::milliseconds duration_;
    TimerCallBack     call_func_;
};

class SteadyTimer :public Timer
{
    friend class TimerManager;
public:
    SteadyTimer(std::chrono::milliseconds time, int64_t id, bool loop, std::function<void()>&& callFunc);
    virtual bool IsExpired() const override;
    virtual int64_t RemainingTime() const override;

protected:
    virtual void Reset() override;

private:
    std::chrono::steady_clock::time_point end_time_point_;
};


class SystemTimer : public Timer
{
    friend class TimerManager;
public:
    SystemTimer(std::chrono::milliseconds time, int64_t id, bool loop, std::function<void()>&& callFunc);
    virtual bool IsExpired() const override;
    virtual int64_t RemainingTime() const override;

protected:
    virtual void Reset() override;

private:
    std::chrono::system_clock::time_point end_time_point_;
};













}