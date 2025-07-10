#pragma once
#include <vector>
#include <memory>

namespace gb
{


//buffer��
struct BufferHandle
{
    char* data; //�����׵�ַ
    int   size; //���ݴ�С

    union
    {
        int capacity; //WriteBuffer ʹ�ã��������
        int offset;   //ReadBuffer ʹ�ã������ʼλ��
    };

    BufferHandle(char* _data, int _capacity) :
        data(_data), size(0), capacity(_capacity) {}

    BufferHandle(char* _data, int _size, int _offset) :
        data(_data), size(_size), offset(_offset) {}
};


} // namespace gb