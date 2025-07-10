#include "rpc_function_help.h"
#include <gbnet/buffer/compressed_stream.h>
namespace gb
{
	void GetMsgData(Meta& meta, ReadBufferPtr buffer, int meta_size, int64_t data_size, std::string& out_s)
	{
		std::string s = buffer->ToString(); // 原始字符串拷贝

		if ((CompressType)meta.compress_type() == CompressType::CompressTypeNone)
		{
			// 对于未压缩的数据，直接截取需要的部分
			out_s = s.substr(meta_size, data_size);
		}
		else
		{
			// 对于压缩的数据，使用 protobuf 解码
			google::protobuf::io::ArrayInputStream         i(s.data() + meta_size, data_size);
			std::shared_ptr<AbstractCompressedInputStream> is(get_compressed_input_stream(&i, (CompressType)(CompressType)meta.compress_type()));
			google::protobuf::io::CodedInputStream         c(is.get());
			uint32_t                                       size;
			c.ReadVarint32(&size);

			// 解码字符串并将其存储在传入的out_s中
			c.ReadString(&out_s, size);
		}
	}

}