#pragma once 
#include <gbnet/buffer/buffer.h>
#include "network/message_meta.h"
namespace gb
{
	void GetMsgData(Meta& meta, ReadBufferPtr buffer, int meta_size, int64_t data_size, std::string& out_s);

}
