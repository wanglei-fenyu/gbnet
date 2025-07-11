#pragma once 
#include "../common/common.h"
#include <atomic>
#include <memory>

namespace gb
{

/**
 * 流量控制
 */

class FlowController
{
public:
    FlowController(bool read_no_limit, int read_quota, bool write_no_limit, int write_quota);
    ~FlowController();


    // 重置读取配额。
    // @param read_no_limit 如果设置为true，则没有限制。
    // @param quota         新的配额，仅当read_no_limit为false时有效。
    void reset_read_quota(bool read_no_limit, int quota);

    // 充值读取配额。
    // @param quota 要充值的配额。
    void recharge_read_quota(int quota);


    // 重置写入配额。
    // @param write_no_limit 如果设置为true，则没有限制。
    // @param quota          新的配额，仅当write_no_limit为false时有效。
    void reset_write_quota(bool write_no_limit, int quota);


    // 充值写入配额。
    // @param quota 要充值的配额。
    void recharge_write_quota(int quota);


    // 检查是否有更多的读取配额。
    bool has_read_quota() const;

	// 检查是否有更多的写入配额。
    bool has_write_quota() const;


    // 获取一些读取配额。
    // @param quota 期望获取的配额。
    // @return >0  如果获取成功。
    // @return <=0 如果获取失败，返回值可用作序列号，以便对触发顺序进行排序：越接近零，触发越早。
    int acquire_read_quota(int quota);

    // 获取一些写入配额。
    // @param quota 期望获取的配额。
    // @return >0  如果获取成功。
    // @return <=0 如果获取失败，返回值可用作序列号，以便对触发顺序进行排序：越接近零，触发越早。
    int acquire_write_quota(int quota);

private:
    bool             _read_no_limit;
    std::atomic<int> _read_quota;

    bool             _write_no_limit;
    std::atomic<int> _write_quota;   

    NON_COPYABLE(FlowController);
};


using FlowControllerPtr = std::shared_ptr<FlowController>;
}