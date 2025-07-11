#pragma once
#include "gbnet/buffer/buffer.h"
#include "msgpack/msgpack.hpp"
#include "google/protobuf/message.h"
namespace gb
{

template<typename ...Args>
ReadBufferPtr PackBuffer(Args& ...args)
{
    std::tuple<Args...> tu = std::forward_as_tuple(std::forward<Args>(args)...);
    ReadBufferPtr       read_buffer(new ReadBuffer());
    WriteBuffer         write_buffer;
    if constexpr (1 == sizeof...(Args))
    {
        auto& arg = std::get<0>(tu);
        if constexpr (std::is_base_of_v<google::protobuf::Message, typename std::decay_t<std::remove_pointer_t<std::tuple_element_t<0, decltype(tu)>>>>)
        {
            if constexpr (std::is_pointer_v<typename std::decay_t<std::tuple_element_t<0, decltype(tu)>>>)
            {
                arg->SerializeToZeroCopyStream(&write_buffer);
            }
            else
            {
                arg.SerializeToZeroCopyStream(&write_buffer);
            }
        }
        else if constexpr (std::is_same_v<ReadBufferPtr,typename std::decay_t<std::remove_pointer_t<std::tuple_element_t<0, decltype(tu)>>>>)
		{
			return arg;
		}
        else
        {
            std::vector<uint8_t> data = gb::msgpack::pack(std::forward<typename std::decay_t<std::tuple_element_t<0, decltype(tu)>>>(arg));
			if (!data.empty())
			{
                write_buffer.Append((char*)data.data(), (int)data.size());
			}

        }
    }
    else
    {
        std::vector<uint8_t> data = gb::msgpack::pack(std::move(tu));
        if (!data.empty())
        {
            write_buffer.Append((char*)data.data(), (int)data.size());
        }
    }

    write_buffer.SwapOut(read_buffer.get());
    return read_buffer;

}

template<typename ...Args>
ReadBufferPtr PackBuffer(Args&& ...args)
{
    std::tuple<Args &&...> tu = std::forward_as_tuple(std::forward<Args>(args)...);
    ReadBufferPtr       read_buffer(new ReadBuffer());
    WriteBuffer         write_buffer;
    if constexpr (1 == sizeof...(Args))
    {
        auto& arg = std::get<0>(tu);
        if constexpr (std::is_base_of_v<google::protobuf::Message, typename std::decay_t<std::remove_pointer_t<std::tuple_element_t<0, decltype(tu)>>>>)
        {
            if constexpr (std::is_pointer_v<typename std::decay_t<std::tuple_element_t<0, decltype(tu)>>>)
            {
                arg->SerializeToZeroCopyStream(&write_buffer);
            }
            else
            {
                arg.SerializeToZeroCopyStream(&write_buffer);
            }
        }
		else if constexpr (std::is_same_v<ReadBufferPtr,typename std::decay_t<std::remove_pointer_t<std::tuple_element_t<0, decltype(tu)>>>>)
		{
			return arg;
		}
        else
        {
            std::vector<uint8_t> data = msgpack::pack(std::forward<typename std::decay_t<std::tuple_element_t<0, decltype(tu)>>>(arg));
			if (!data.empty())
			{
                write_buffer.Append((char*)data.data(), (int)data.size());
			}

        }
    }
    else
    {
        std::vector<uint8_t> data = msgpack::pack(std::move(tu));
        if (!data.empty())
        {
            write_buffer.Append((char*)data.data(), (int)data.size());
        }
    }

    write_buffer.SwapOut(read_buffer.get());
    return read_buffer;

}



}