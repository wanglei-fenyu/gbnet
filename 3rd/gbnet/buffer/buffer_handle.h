#pragma once
#include <vector>
#include <memory>

namespace gb
{


//buffer块
struct BufferHandle
{
    char* data; //数据首地址
    int   size; //数据大小

    union
    {
        int capacity; //WriteBuffer 使用，块的容量
        int offset;   //ReadBuffer 使用，块的起始位置
    };

    BufferHandle(char* _data, int _capacity) :
        data(_data), size(0), capacity(_capacity) {}

    BufferHandle(char* _data, int _size, int _offset) :
        data(_data), size(_size), offset(_offset) {}
};


} // namespace gb