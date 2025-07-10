#pragma once


#include <atomic>
#include <cstdint>
namespace gb
{

template <typename T>
inline void atomic_inc(std::atomic<T>* n)
{
    n->fetch_add(1, std::memory_order_relaxed);
}

template <typename T>
inline void atomic_dec(std::atomic<T>* n)
{
    n->fetch_sub(1, std::memory_order_relaxed);
}

template <typename T>
inline T atomic_add_ret_old(std::atomic<T>* n, T v)
{
    return n->fetch_add(v, std::memory_order_relaxed);
}

template <typename T>
inline T atomic_inc_ret_old(std::atomic<T>* n)
{
    return n->fetch_add(1, std::memory_order_relaxed);
}

template <typename T>
inline T atomic_dec_ret_old(std::atomic<T>* n)
{
    return n->fetch_sub(1, std::memory_order_relaxed);
}

template <typename T>
inline T atomic_add_ret_old64(std::atomic<T>* n, T v)
{
    return n->fetch_add(v, std::memory_order_relaxed);
}

template <typename T>
inline T atomic_inc_ret_old64(std::atomic<T>* n)
{
    return n->fetch_add(1, std::memory_order_relaxed);
}

template <typename T>
inline T atomic_dec_ret_old64(std::atomic<T>* n)
{
    return n->fetch_sub(1, std::memory_order_relaxed);
}

template <typename T>
inline void atomic_add(std::atomic<T>* n, T v)
{
    n->fetch_add(v, std::memory_order_relaxed);
}

template <typename T>
inline void atomic_sub(std::atomic<T>* n, T v)
{
    n->fetch_sub(v, std::memory_order_relaxed);
}

template <typename T, typename C, typename D>
inline T atomic_cmpxchg(std::atomic<T>* n, C cmp, D dest)
{
    T old_value = cmp;
    n->compare_exchange_strong(old_value, dest, std::memory_order_seq_cst);
    return old_value;
}

template <typename T>
inline T atomic_swap(std::atomic<T>* lockword, T value)
{
    return lockword->exchange(value, std::memory_order_relaxed);
}

template <typename T, typename E, typename C>
inline T atomic_comp_swap(std::atomic<T>* lockword, E exchange, C comperand)
{
    T old_value = comperand;
    lockword->compare_exchange_strong(old_value, exchange, std::memory_order_seq_cst);
    return old_value;
}


} // namespace gb
