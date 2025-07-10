#pragma once 
#include <gbnet/buffer/buffer.h>
#include "../../protobuf/meta.pb.h"
namespace gb
{
	void GetMsgData(Meta& meta, ReadBufferPtr buffer, int meta_size, int64_t data_size, std::string& out_s);

}
