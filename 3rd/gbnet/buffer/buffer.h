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
     * 向buffer中添加bufferHandle
     * 前提条件：
     *      1.之前没有调用过Next()、Backup()或Skip()方法
     *      2.buf_handle的大小应该大于0
     *      3.read_buffer不应该为NULL
     */
    void Append(const BufferHandle& buf_handle);
    void Append(const ReadBuffer* read_buffer);

    //获取缓冲区总的字节数
    int64_t TotalCount() const;

    //获取缓冲区占用块的个数
    int BlockCount() const;

    //获取缓冲区占用的总块大小。
    int64_t TotalBlockSize() const;

    std::string ToString();

    bool    Next(const void** data, int* size);
    void    BackUp(int count);
    bool    Skip(int count);
    int64_t ByteCount() const;

private:
    BufHandleList         _buf_list;
    int64_t               _total_block_size; //缓冲区总的块大小
    int64_t               _total_bytes;      //缓冲区总字节
    BufHandleListIterator _cur_it;           //当前节点
    int                   _cur_pos;
    int                   _last_bytes; //最后读取的字节数
    int64_t               _read_bytes; //总共读取的字节数

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
     * 从Writebuffer(输出流)取出数据，将其写入到ReadBuffer(输入流)
     * 执行完之后 WriteBuffer会被清空  就像一个全新的WriteBuffer
     */
    void SwapOut(ReadBuffer* is);

    /**
     * 在缓冲区中保留一些空间。
     * 成功，返回预留空间的头部位置。
     * 失败，返回-1。
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
    // 在这个buffer结尾拓展一个新的内存块
    bool Extend();

private:
    BufHandleList _buf_list;
    int64_t       _total_block_size;  //缓冲区中的总块大小
    int64_t       _total_capacity;    //缓冲区中的总容量
    int           _last_bytes;        //最后写入的字节数
    int64_t       _write_bytes;       //总共写入的字节数
    size_t        _base_block_factor; //内存块膨胀因子

    NON_COPYABLE(WriteBuffer);
};

using ReadBufferPtr = std::shared_ptr<ReadBuffer>;
using WriteBufferPtr = std::shared_ptr<WriteBuffer>;

} // namespace gb