#pragma once
#include <memory>
#include "../common/atomic.h"

#ifndef TRAN_BUF_BLOCK_BASE_SIZE
// base_block_size = 64B
#define TRAN_BUF_BLOCK_BASE_SIZE (64u)
#endif

#ifndef TRAN_BUF_BLOCK_MAX_FACTOR
// max_block_size = 64<<9 = 32K
#define TRAN_BUF_BLOCK_MAX_FACTOR (9u)
#endif

/// <summary>
/// �̰߳�ȫ
/// ���ü���ת���ء�
/// int �� atomic<int>  ��С��ͬ
/// Block = BlockSize��int�� + RefCount(atomic<int>) + Data
/// </summary>


namespace gb
{


class TranBufPool
{
public:
    inline static void* malloc(int factor = 0)
    {
        void* p = ::malloc(TRAN_BUF_BLOCK_BASE_SIZE << factor);
        if (p != nullptr)
        {
            *(reinterpret_cast<int*>(p)) = TRAN_BUF_BLOCK_BASE_SIZE << factor; //�ڴ濪ͷ����ڴ��Ĵ�С   ������ڴ��һ��intλ
            //*(reinterpret_cast<int*>(p) + 1) = 1;                                      //������������ü�����ʼ��Ϊ1  ������ڴ�ĵڶ���intλ
            auto refCount = reinterpret_cast<std::atomic<int>*>(p) + 1;
            *refCount     = 1;
            p             = reinterpret_cast<int*>(p) + 2; //ǰ����intλ�ֱ� �ڴ�Ĵ�С�����ü���ռ��  ��������λ����ƫ������int��ʼ
        }
        return p;
    }


    inline static int block_size(void* p)
    {
        return *(reinterpret_cast<int*>(p) - 2);
    }

    inline static int capacity(void* p)
    {
        //�ڴ濪ͷ��ŵ����� - ��С�����ü���
        return *(reinterpret_cast<int*>(p) - 2) - sizeof(int) * 2;
    }

    inline static void add_ref(void* p)
    {
        auto refCount = reinterpret_cast<std::atomic<int>*>(p) - 1;
        //refCount->fetch_add(1, std::memory_order_relaxed);
        atomic_inc(refCount);
    }

    inline static void free(void* p)
    {
        auto refCount = reinterpret_cast<std::atomic<int>*>(p) - 1;
        if (atomic_dec_ret_old(refCount) == 1)
        {
            ::free(reinterpret_cast<int*>(p) - 2);
        }
        //if (atomic_dec_ret_old(reinterpret_cast<int*>(p) - 1) == 1)
        //{
        //    ::free(reinterpret_cast<int*>(p) - 2);
        //}
    }
};

} // namespace gb
