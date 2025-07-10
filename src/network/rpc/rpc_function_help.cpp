#include "rpc_function_help.h"
#include <gbnet/buffer/compressed_stream.h>
namespace gb
{
	void GetMsgData(Meta& meta, ReadBufferPtr buffer, int meta_size, int64_t data_size, std::string& out_s)
	{
		std::string s = buffer->ToString(); // ԭʼ�ַ�������

		if ((CompressType)meta.compress_type() == CompressType::CompressTypeNone)
		{
			// ����δѹ�������ݣ�ֱ�ӽ�ȡ��Ҫ�Ĳ���
			out_s = s.substr(meta_size, data_size);
		}
		else
		{
			// ����ѹ�������ݣ�ʹ�� protobuf ����
			google::protobuf::io::ArrayInputStream         i(s.data() + meta_size, data_size);
			std::shared_ptr<AbstractCompressedInputStream> is(get_compressed_input_stream(&i, (CompressType)(CompressType)meta.compress_type()));
			google::protobuf::io::CodedInputStream         c(is.get());
			uint32_t                                       size;
			c.ReadVarint32(&size);

			// �����ַ���������洢�ڴ����out_s��
			c.ReadString(&out_s, size);
		}
	}

}