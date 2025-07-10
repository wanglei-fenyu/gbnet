#pragma once
#include <vector>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <google/protobuf/io/zero_copy_stream.h>
#include "buffer_handle.h"
#include "../common/common.h"

namespace gb
{


using BufferHandlePtr              = std::shared_ptr<BufferHandle>;
using BufHandleList                = std::deque<BufferHandle>;
using BufHandleListIterator        = std::deque<BufferHandle>::iterator;
using BufHandleListReverseIterator = std::deque<BufferHandle>::reverse_iterator;

class ReadBuffer : public google::protobuf::io::ZeroCopyInputStream
{
public:
    ReadBuffer();
    virtual ~ReadBuffer();

    /**
     * ��buffer�����bufferHandle
     * ǰ��������
     *      1.֮ǰû�е��ù�Next()��Backup()��Skip()����
     *      2.buf_handle�Ĵ�СӦ�ô���0
     *      3.read_buffer��Ӧ��ΪNULL
     */
    void Append(const BufferHandle& buf_handle);
    void Append(const ReadBuffer* read_buffer);

    //��ȡ�������ܵ��ֽ���
    int64_t TotalCount() const;

    //��ȡ������ռ�ÿ�ĸ���
    int BlockCount() const;

    //��ȡ������ռ�õ��ܿ��С��
    int64_t TotalBlockSize() const;

    std::string ToString();

    bool    Next(const void** data, int* size);
    void    BackUp(int count);
    bool    Skip(int count);
    int64_t ByteCount() const;

private:
    BufHandleList         _buf_list;
    int64_t               _total_block_size; //�������ܵĿ��С
    int64_t               _total_bytes;      //���������ֽ�
    BufHandleListIterator _cur_it;           //��ǰ�ڵ�
    int                   _cur_pos;
    int                   _last_bytes; //����ȡ���ֽ���
    int64_t               _read_bytes; //�ܹ���ȡ���ֽ���

    NON_COPYABLE(ReadBuffer);
};


class WriteBuffer : public google::protobuf::io::ZeroCopyOutputStream
{
public:
    WriteBuffer();
    virtual ~WriteBuffer();


    int64_t TotalCapacity() const;

    int BlockCount() const;

    int64_t TotalBlockSize() const;


    /**
     * ��Writebuffer(�����)ȡ�����ݣ�����д�뵽ReadBuffer(������)
     * ִ����֮�� WriteBuffer�ᱻ���  ����һ��ȫ�µ�WriteBuffer
     */
    void SwapOut(ReadBuffer* is);

    /**
     * �ڻ������б���һЩ�ռ䡣
     * �ɹ�������Ԥ���ռ��ͷ��λ�á�
     * ʧ�ܣ�����-1��
     */
    int64_t Reserve(int count);

    void SetData(int64_t pos, const char* data, int size);

    bool    Next(void** data, int* size);
    void    BackUp(int count);
    int64_t ByteCount() const;

    bool Append(const std::string& data);
    bool Append(const char* data, int size);


    void set_base_block_factor(size_t factor);

    size_t base_block_factor();

private:
    // �����buffer��β��չһ���µ��ڴ��
    bool Extend();

private:
    BufHandleList _buf_list;
    int64_t       _total_block_size;  //�������е��ܿ��С
    int64_t       _total_capacity;    //�������е�������
    int           _last_bytes;        //���д����ֽ���
    int64_t       _write_bytes;       //�ܹ�д����ֽ���
    size_t        _base_block_factor; //�ڴ����������

    NON_COPYABLE(WriteBuffer);
};

using ReadBufferPtr = std::shared_ptr<ReadBuffer>;
using WriteBufferPtr = std::shared_ptr<WriteBuffer>;

} // namespace gb