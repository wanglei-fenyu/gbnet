#include "block_wrappers.h"
#include "../common/define.h"
#include "lz4.h"
namespace gb
{

 BlockCompressionInputStream::BlockCompressionInputStream(ZeroCopyInputStream* sub_stream) 
	 : _output_buffer(NULL)
	 , _output_buffer_size(0)
	 , _output_buffer_capacity(0)
	 , _sub_stream(NULL)
	 , _backed_up_bytes(0)
	 , _byte_count(0)
{
     _raw_stream = sub_stream;
    _sub_stream = new CodedInputStream(_raw_stream);
     _sub_stream->SetTotalBytesLimit(1 << 30);
 }

 BlockCompressionInputStream::~BlockCompressionInputStream()
{
     delete _sub_stream;
    delete[] _output_buffer;
 }

bool BlockCompressionInputStream::Next(const void** data, int* size)
{
	if (_backed_up_bytes) {
		size_t skip = _output_buffer_size - _backed_up_bytes;
		(*data) = _output_buffer + skip;
		(*size) = _backed_up_bytes;
		_backed_up_bytes = 0;
		_byte_count += *size;
		return true;
    }
    
    uint32_t compressed_size = 0;
    if (!_sub_stream->ReadVarint32(&compressed_size) || compressed_size > 2*BLOCKSIZE) {
        return false;
    }

    char * tempbuffer = new char[compressed_size];
    if (!_sub_stream->ReadRaw(tempbuffer, compressed_size)) {
        delete[] tempbuffer;
        return false;
    }

    RawUncompress(tempbuffer, compressed_size);
    delete[] tempbuffer;

    if (_sub_stream->BytesUntilLimit() < 1024*1024) reset_input_stream();
    
    (*data) = _output_buffer;
    (*size) = _output_buffer_size;
    _byte_count += *size;
    return true;
}

void BlockCompressionInputStream::BackUp(int count)
{
    if (count <= 0) return;
    NET_CHECK(_output_buffer);
    NET_CHECK_LE(static_cast<size_t>(count), _output_buffer_size - _backed_up_bytes);
    _backed_up_bytes += count;
    _byte_count -= count;
}

bool BlockCompressionInputStream::Skip(int count)
{
    NET_CHECK(false);
    return false;
}


void BlockCompressionInputStream::reset_input_stream()
{
     delete _sub_stream;
    _sub_stream = new CodedInputStream(_raw_stream);
    _sub_stream->SetTotalBytesLimit(1 << 30);
}




BlockCompressionOutputStream::BlockCompressionOutputStream(ZeroCopyOutputStream* sub_stream)
    : _input_buffer(NULL)
    , _sub_stream(new CodedOutputStream(sub_stream))
    , _backed_up_bytes(0)
    , _byte_count(0)
{
}

 BlockCompressionOutputStream::~BlockCompressionOutputStream()
{
	 if (_input_buffer) 
     {
	    NET_CHECK(false);
    }
    delete _sub_stream;
}

bool BlockCompressionOutputStream::Next(void** data, int* size)
{
    if (_backed_up_bytes) 
    {
        size_t skip = BLOCKSIZE - _backed_up_bytes;
        (*data) = _input_buffer + skip;
        (*size) = _backed_up_bytes;
        _backed_up_bytes = 0;
        _byte_count += *size;
        return true;
    }
    if(_input_buffer) Flush();
    _input_buffer = new char[BLOCKSIZE];
    (*data) = _input_buffer;
    (*size) = BLOCKSIZE;
    _byte_count += *size;
    return true;
}

void BlockCompressionOutputStream::BackUp(int count)
{
    if (count <= 0) return;
    NET_CHECK(_input_buffer);
    NET_CHECK_LE(static_cast<size_t>(count), BLOCKSIZE - _backed_up_bytes);
    _backed_up_bytes += count;
    _byte_count -= count;
}

bool BlockCompressionOutputStream::Flush()
{
    size_t size = BLOCKSIZE - _backed_up_bytes;
    if (!_input_buffer || size == 0) return true;
    
    size_t compressed_size = 0;
    char * compressed_data = new char[MaxCompressedLength(size)];
    compressed_size = RawCompress(_input_buffer, size, compressed_data);

    NET_CHECK_LE(compressed_size, 2*BLOCKSIZE);
    
    uint32_t compressed_size_32 = static_cast<uint32_t>(compressed_size);
    _sub_stream->WriteVarint32(compressed_size_32);
    _sub_stream->WriteRaw(compressed_data, compressed_size_32);
    delete[] compressed_data;

    _backed_up_bytes = 0;
    delete[] _input_buffer;
    _input_buffer = 0;
    return true;
}




void LZ4InputStream::RawUncompress(char* input_buffer, uint32_t compressed_size) {
    if (!_output_buffer) {
        _output_buffer = new char[BLOCKSIZE];
        _output_buffer_capacity = BLOCKSIZE;
    }
    _output_buffer_size = LZ4_uncompress_unknownOutputSize(input_buffer,
        _output_buffer, compressed_size, BLOCKSIZE);
}

uint32_t LZ4OutputStream::MaxCompressedLength(size_t input_size)
{
    return LZ4_compressBound(input_size);
}

uint32_t LZ4OutputStream::RawCompress(char* input_buffer, size_t input_size,
    char* output_buffer)
{
    return LZ4_compress(input_buffer, output_buffer, input_size);
}

}