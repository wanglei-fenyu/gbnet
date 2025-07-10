#pragma once
#include <google/protobuf/io/zero_copy_stream.h>
using google::protobuf::io::ZeroCopyInputStream;
using google::protobuf::io::ZeroCopyOutputStream;

#include <google/protobuf/io/coded_stream.h>
using google::protobuf::io::CodedInputStream;
using google::protobuf::io::CodedOutputStream;

#include "compressed_stream.h"

namespace gb
{
const size_t BLOCKSIZE = 64 * 1024;
class BlockCompressionInputStream : public AbstractCompressedInputStream
{
public:
    explicit BlockCompressionInputStream(ZeroCopyInputStream* sub_stream);
    virtual ~BlockCompressionInputStream();


    bool Next(const void** data, int* size);
    void BackUp(int count);
    bool Skip(int count);
    int64_t ByteCount() const { return _byte_count; }
    bool ExpectAtEnd() { return true; }
 protected:
    virtual void RawUncompress(char* input_buffer, uint32_t compressed_size) = 0;
  
    char* _output_buffer;
    size_t _output_buffer_size;
    size_t _output_buffer_capacity;

private:
    void reset_input_stream();
private:

    CodedInputStream* _sub_stream;
    ZeroCopyInputStream* _raw_stream;

    int _backed_up_bytes;
    size_t _byte_count;

    NON_COPYABLE(BlockCompressionInputStream);
};


class BlockCompressionOutputStream : public AbstractCompressedOutputStream {
 public:
    explicit BlockCompressionOutputStream(ZeroCopyOutputStream* sub_stream);
    virtual ~BlockCompressionOutputStream();
    
    bool Next(void** data, int* size);
    void BackUp(int count);
    int64_t ByteCount() const { return _byte_count; };

    bool Flush();
    bool Close() { return Flush(); }

 protected:
    virtual uint32_t MaxCompressedLength(size_t input_size) = 0;
    virtual uint32_t RawCompress(char* input_buffer, size_t input_size, char* output_buffer) = 0;
    char* _input_buffer;
   
 private:
    CodedOutputStream* _sub_stream;
  
    int _backed_up_bytes;
    size_t _byte_count;

    NON_COPYABLE(BlockCompressionOutputStream);
};

class SnappyOutputStream : public BlockCompressionOutputStream {
 public:
  explicit SnappyOutputStream(ZeroCopyOutputStream* sub_stream) : BlockCompressionOutputStream(sub_stream) {};
  
  virtual uint32_t MaxCompressedLength(size_t input_size);
  virtual uint32_t RawCompress(char* input_buffer, size_t input_size, char* output_buffer);
  
  NON_COPYABLE(SnappyOutputStream);
};


class LZ4InputStream : public BlockCompressionInputStream {
 public:

  explicit LZ4InputStream(ZeroCopyInputStream* sub_stream) : BlockCompressionInputStream(sub_stream) {};
  
  virtual void RawUncompress(char* input_buffer, uint32_t compressed_size);
  
  NON_COPYABLE(LZ4InputStream);
};

class LZ4OutputStream : public BlockCompressionOutputStream {
 public:
  explicit LZ4OutputStream(ZeroCopyOutputStream* sub_stream) : BlockCompressionOutputStream(sub_stream) {};
  
  virtual uint32_t MaxCompressedLength(size_t input_size);
  virtual uint32_t RawCompress(char* input_buffer, size_t input_size, char* output_buffer);
  
  NON_COPYABLE(LZ4OutputStream);
};

}
