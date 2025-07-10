#include "compressed_stream.h"
#include "gzip_stream.h"
#include "block_wrappers.h"

namespace gb
{

AbstractCompressedOutputStream* get_compressed_output_stream(google::protobuf::io::ZeroCopyOutputStream* ostream, CompressType type, int level /*=1*/)
{
    switch (type)
    {
        case CompressTypeGzip:
            {
                GzipOutputStream::Options o;
                o.format = GzipOutputStream::GZIP;
                o.compression_level = level;
                return new GzipOutputStream(ostream, o);
            }
            break;
        case CompressTypeZlib:
             {
                GzipOutputStream::Options o;
                o.format = GzipOutputStream::ZLIB;
                o.compression_level = level;
                return new GzipOutputStream(ostream, o);
            }
            break;
        case CompressTypeLZ4:
            return new LZ4OutputStream(ostream);
            break;
        default:
            NET_CHECK(false);
            return nullptr;
    }
}

AbstractCompressedInputStream* get_compressed_input_stream(google::protobuf::io::ZeroCopyInputStream* istream, CompressType type)
{
    switch (type)
    {
        case CompressTypeGzip:
            return new GzipInputStream(istream, GzipInputStream::GZIP);
            break;
        case CompressTypeZlib:
            return new GzipInputStream(istream, GzipInputStream::ZLIB);
            break;
        case CompressTypeLZ4:
            return new LZ4InputStream(istream);
            break;
        default:
            NET_CHECK(false);
            return nullptr;
    }
}

}