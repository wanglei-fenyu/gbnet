#include "flow_controller.h"
#include "../common/atomic.h"

namespace gb
{

 FlowController::FlowController(bool read_no_limit, int read_quota, bool write_no_limit, int write_quota) 
	 : _read_no_limit(read_no_limit)
	 , _read_quota(read_quota)
	 , _write_no_limit(write_no_limit)
	 , _write_quota(write_quota)
{
}

 FlowController::~FlowController()
{
}


void FlowController::reset_read_quota(bool read_no_limit, int quota)
{
    _read_no_limit = read_no_limit;
    _read_quota.store(quota);
}


void FlowController::recharge_read_quota(int quota)
{
    if (_read_no_limit)
        return;

    int old_quota = _read_quota.load();
    int new_quota = old_quota > 0 ? quota : old_quota + quota;

    while (atomic_comp_swap(&_read_quota, new_quota, old_quota) != old_quota)
    {
        old_quota = _read_quota.load();
        new_quota = old_quota > 0 ? quota : old_quota + quota;
    }

}

void FlowController::reset_write_quota(bool write_no_limit, int quota)
{
    _write_no_limit = write_no_limit;
    _write_quota.store(quota);
}


void FlowController::recharge_write_quota(int quota)
{
    if (_write_no_limit)
        return;

    int old_quota = _write_quota.load();
    int new_quota = old_quota > 0 ? quota : old_quota + quota;
	while (atomic_comp_swap(&_write_quota, new_quota, old_quota) != old_quota)
	{
		old_quota = _write_quota;
		new_quota = old_quota > 0 ? quota : old_quota + quota;
	}

}

bool FlowController::has_read_quota() const
{
    return _read_no_limit || _read_quota.load() > 0;
}

bool FlowController::has_write_quota() const
{
    return _write_no_limit || _write_quota.load() > 0;
}

int FlowController::acquire_read_quota(int quota)
{
    if (_read_no_limit)
            return 1;
    int old_quota = _read_quota.load();
    int new_quota = old_quota > 0 ? old_quota - quota : old_quota - 1;

	while (atomic_comp_swap(&_read_quota, new_quota, old_quota) != old_quota)
	{
		old_quota = _read_quota.load();
		new_quota = old_quota > 0 ? old_quota - quota : old_quota - 1;
	}
	return old_quota;
}

int FlowController::acquire_write_quota(int quota)
{
    if (_write_no_limit)
            return 1;

	int old_quota = _write_quota.load();
	int new_quota = old_quota > 0 ? old_quota - quota : old_quota - 1;
	
    while (atomic_comp_swap(&_write_quota, new_quota, old_quota) != old_quota)
	{
		old_quota = _write_quota.load();
		new_quota = old_quota > 0 ? old_quota - quota : old_quota - 1;
	}
	return old_quota;
}

}