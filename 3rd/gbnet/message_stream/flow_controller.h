#pragma once 
#include "../common/common.h"
#include <atomic>
#include <memory>

namespace gb
{

/**
 * ��������
 */

class FlowController
{
public:
    FlowController(bool read_no_limit, int read_quota, bool write_no_limit, int write_quota);
    ~FlowController();


    // ���ö�ȡ��
    // @param read_no_limit �������Ϊtrue����û�����ơ�
    // @param quota         �µ�������read_no_limitΪfalseʱ��Ч��
    void reset_read_quota(bool read_no_limit, int quota);

    // ��ֵ��ȡ��
    // @param quota Ҫ��ֵ����
    void recharge_read_quota(int quota);


    // ����д����
    // @param write_no_limit �������Ϊtrue����û�����ơ�
    // @param quota          �µ�������write_no_limitΪfalseʱ��Ч��
    void reset_write_quota(bool write_no_limit, int quota);


    // ��ֵд����
    // @param quota Ҫ��ֵ����
    void recharge_write_quota(int quota);


    // ����Ƿ��и���Ķ�ȡ��
    bool has_read_quota() const;

	// ����Ƿ��и����д����
    bool has_write_quota() const;


    // ��ȡһЩ��ȡ��
    // @param quota ������ȡ����
    // @return >0  �����ȡ�ɹ���
    // @return <=0 �����ȡʧ�ܣ�����ֵ���������кţ��Ա�Դ���˳���������Խ�ӽ��㣬����Խ�硣
    int acquire_read_quota(int quota);

    // ��ȡһЩд����
    // @param quota ������ȡ����
    // @return >0  �����ȡ�ɹ���
    // @return <=0 �����ȡʧ�ܣ�����ֵ���������кţ��Ա�Դ���˳���������Խ�ӽ��㣬����Խ�硣
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