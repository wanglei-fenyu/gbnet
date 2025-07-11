
#include "session.h"
#include "../buffer_pack.hpp"
#include "../../common/timer_help.h"
#include "gbnet/buffer/compressed_stream.h"
namespace gb
{

 Session::Session(NET_TYPE net_type, IoService& ios, const Endpoint& endpoint) 
     : MessageStream(net_type, ios, endpoint)
     , _start_heartbeat(false), _heartbeat_timer(ios)
 {
}

 Session::~Session()
{
}



bool Session::on_sending(const ReadBufferPtr& message)
{
    //todo
    return true;
}

void Session::on_sent(const ReadBufferPtr& message)
{
    if (_sent_callbcak)
        _sent_callbcak(std::dynamic_pointer_cast<Session>(shared_from_this()), message);
}

void Session::on_send_failed(std::string_view peason, const ReadBufferPtr& message)
{
    //todo

}

void Session::on_received(const ReadBufferPtr& message, int meta_size, int64_t data_size)
{
    if (_received_callback)
        _received_callback(std::dynamic_pointer_cast<Session>(shared_from_this()), message, meta_size, data_size);
}

bool Session::on_connected()
{
    MessageStream::on_connected();
    if (_connected_callback)
		_connected_callback(std::dynamic_pointer_cast<Session>(shared_from_this()));
    return true;
}

void Session::on_closed()
{
    _start_heartbeat.store(false);
    _heartbeat_timer.cancel();
    if (_closed_callback)
        _closed_callback(std::dynamic_pointer_cast<Session>(shared_from_this()));
}



void Session::Send(const Meta* meta)
{
    MessageHeader header;
    int header_size = sizeof(header);
	WriteBuffer   write_buffer;
	ReadBufferPtr read_buffer(new ReadBuffer());
	int64_t       header_pos = write_buffer.Reserve(header_size);
    if (meta == nullptr)
    {
        //空消息只有头  心跳包
		write_buffer.SetData(header_pos, reinterpret_cast<const char*>(&header), header_size);
    }
    else
    {
		if (header_size < 0)
		{
	   
			NETWORK_LOG("reserve message header failed");
		}
		if (!meta->SerializeToZeroCopyStream(&write_buffer))
		{
			NETWORK_LOG("serialize meta failed");
		}
		
		header.meta_size = static_cast<int>(write_buffer.ByteCount() - header_pos - header_size);
		header.message_size = header.meta_size + header.data_size;
		
		write_buffer.SetData(header_pos, reinterpret_cast<const char*>(&header), header_size);

    }
	write_buffer.SwapOut(read_buffer.get());
    async_send_message(read_buffer);
}





void Session::Send(const Meta* meta, const google::protobuf::Message* message)
{
    if (!meta)
        return;
    MessageHeader header;
    int header_size = sizeof(header);
    WriteBuffer   write_buffer;
    int64_t       header_pos = write_buffer.Reserve(header_size);
    if (header_size < 0)
    {
        NETWORK_LOG("reserve message header failed");
    }
	if (!meta->SerializeToZeroCopyStream(&write_buffer))
	{
		NETWORK_LOG("serialize meta failed");
	}
   
    
    header.meta_size = static_cast<int>(write_buffer.ByteCount() - header_pos - header_size);

    if ((CompressType)meta->compress_type() == CompressType::CompressTypeNone)
    {
        if (!message->SerializeToZeroCopyStream(&write_buffer))
        {
			NETWORK_LOG("serialize data failed");
            return;
        }
    }
    else
    {
        std::shared_ptr<AbstractCompressedOutputStream> os(get_compressed_output_stream(&write_buffer, (CompressType)meta->compress_type()));
        if (!message->SerializeToZeroCopyStream(os.get()))
        {
			NETWORK_LOG("serialize data failed");
            return;
        }
        os->Flush();
    }

    //if (data_buffer->TotalCount() > 0)
        //header.data_size = data_buffer->TotalCount();
    header.data_size = write_buffer.ByteCount() - header_pos - header_size - header.meta_size;
    header.message_size = header.meta_size + header.data_size;
    
    write_buffer.SetData(header_pos, reinterpret_cast<const char*>(&header), header_size);
    ReadBufferPtr read_buffer(new ReadBuffer());
    write_buffer.SwapOut(read_buffer.get());
    async_send_message(read_buffer);
}

void Session::Send(const Meta* meta, const std::vector<uint8_t>& data)
{
    MessageHeader header;
    int header_size = sizeof(header);
    WriteBuffer   write_buffer;
    int64_t       header_pos = write_buffer.Reserve(header_size);
    if (header_size < 0)
    {
        NETWORK_LOG("reserve message header failed");
    }
   
    if (!meta->SerializeToZeroCopyStream(&write_buffer))
    {
        NETWORK_LOG("serialize meta failed");
    }
    header.meta_size = static_cast<int>(write_buffer.ByteCount() - header_pos - header_size);

    if ((CompressType)meta->compress_type() == CompressType::CompressTypeNone)
    {
        write_buffer.Append((char*)data.data(), data.size());
    }
    else
    {
        std::string  compressed;
        google::protobuf::io::StringOutputStream o(&compressed);
        std::shared_ptr<AbstractCompressedOutputStream> os(get_compressed_output_stream(&o, (CompressType)meta->compress_type()));
        {
			google::protobuf::io::CodedOutputStream c(os.get());
            c.WriteVarint32(data.size());
            c.WriteRaw(data.data(), data.size());
        }
        os->Flush();
        write_buffer.Append((char*)compressed.data(), compressed.size());
    }


	header.data_size    = write_buffer.ByteCount() - header_pos - header_size - header.meta_size;
    header.message_size = header.meta_size + header.data_size;

    write_buffer.SetData(header_pos, reinterpret_cast<const char*>(&header), header_size);
    ReadBufferPtr read_buffer(new ReadBuffer());
    write_buffer.SwapOut(read_buffer.get());
    async_send_message(read_buffer);
}

void Session::Send(const Meta* meta, std::string_view data)
{
    MessageHeader header;
    int header_size = sizeof(header);
    WriteBuffer   write_buffer;
    int64_t       header_pos = write_buffer.Reserve(header_size);
    if (header_size < 0)
    {
        NETWORK_LOG("reserve message header failed");
    }
   
    if (!meta->SerializeToZeroCopyStream(&write_buffer))
    {
        NETWORK_LOG("serialize meta failed");
    }
    header.meta_size = static_cast<int>(write_buffer.ByteCount() - header_pos - header_size);

    if ((CompressType)meta->compress_type() == CompressType::CompressTypeNone)
    {
        write_buffer.Append((char*)data.data(), data.size());
    }
    else
    {
        std::string                                     compressed;
        google::protobuf::io::StringOutputStream        o(&compressed);
        std::shared_ptr<AbstractCompressedOutputStream> os(get_compressed_output_stream(&o, (CompressType)meta->compress_type()));
        {
            google::protobuf::io::CodedOutputStream c(os.get());
            c.WriteVarint32(data.size());
            c.WriteRaw(data.data(), data.size());
        }
        os->Flush();
        write_buffer.Append((char*)compressed.data(), compressed.size());
    }
    header.data_size    = write_buffer.ByteCount() - header_pos - header_size - header.meta_size;
    header.message_size = header.meta_size + header.data_size;
    
    write_buffer.SetData(header_pos, reinterpret_cast<const char*>(&header), header_size);
    ReadBufferPtr read_buffer(new ReadBuffer());
    write_buffer.SwapOut(read_buffer.get());
    async_send_message(read_buffer);

}

void Session::Send(const Meta* meta, const char* data, std::size_t size)
{
    MessageHeader header;
    int header_size = sizeof(header);
    WriteBuffer   write_buffer;
    int64_t       header_pos = write_buffer.Reserve(header_size);
    if (header_size < 0)
    {
        NETWORK_LOG("reserve message header failed");
    }
   
    if (!meta->SerializeToZeroCopyStream(&write_buffer))
    {
        NETWORK_LOG("serialize meta failed");
    }
    
    header.meta_size = static_cast<int>(write_buffer.ByteCount() - header_pos - header_size);


    if ((CompressType)meta->compress_type() == CompressType::CompressTypeNone)
    {
        write_buffer.Append(data,size);
    }
    else
    {
        std::string                                     compressed;
        google::protobuf::io::StringOutputStream        o(&compressed);
        std::shared_ptr<AbstractCompressedOutputStream> os(get_compressed_output_stream(&o, (CompressType)meta->compress_type()));
        {
            google::protobuf::io::CodedOutputStream c(os.get());
            c.WriteVarint32(size);
            c.WriteRaw(data, size);
        }
        os->Flush();
        write_buffer.Append((char*)compressed.data(), compressed.size());
    }
    header.data_size    =  write_buffer.ByteCount() - header_pos - header_size - header.meta_size;
    header.message_size = header.meta_size + header.data_size;

    write_buffer.SetData(header_pos, reinterpret_cast<const char*>(&header), header_size);
    ReadBufferPtr read_buffer(new ReadBuffer());
    write_buffer.SwapOut(read_buffer.get());
    async_send_message(read_buffer);
}

void Session::Send(const Meta* meta, const ReadBufferPtr& data_buffer)
{
    MessageHeader header;
    int header_size = sizeof(header);
    WriteBuffer   write_buffer;
    int64_t       header_pos = write_buffer.Reserve(header_size);
    if (header_size < 0)
    {
        NETWORK_LOG("reserve message header failed");
    }
   
    if (!meta->SerializeToZeroCopyStream(&write_buffer))
    {
        NETWORK_LOG("serialize meta failed");
    }
    
    header.meta_size = static_cast<int>(write_buffer.ByteCount() - header_pos - header_size);
    if ((CompressType)meta->compress_type() == CompressType::CompressTypeNone)
    {
        std::string  data = data_buffer->ToString();
        write_buffer.Append(data.data(),data.size());
    }
    else
    {
        std::string                                     compressed;
        google::protobuf::io::StringOutputStream        o(&compressed);
        std::shared_ptr<AbstractCompressedOutputStream> os(get_compressed_output_stream(&o, (CompressType)meta->compress_type()));
        {
            std::string                             data = data_buffer->ToString();
            google::protobuf::io::CodedOutputStream c(os.get());
            c.WriteVarint32(data.size());
            c.WriteString(data);
        }
        os->Flush();
        write_buffer.Append((char*)compressed.data(), compressed.size());
    }

	header.data_size    = write_buffer.ByteCount() - header_pos - header_size - header.meta_size;
    header.message_size = header.meta_size + header.data_size;
    
    write_buffer.SetData(header_pos, reinterpret_cast<const char*>(&header), header_size);
    ReadBufferPtr read_buffer(new ReadBuffer());
    write_buffer.SwapOut(read_buffer.get());
    async_send_message(read_buffer);
}


void Session::set_connected_callback(session_connected_callback_t call_bcak)
{
    _connected_callback = call_bcak;
}

void Session::set_closed_callback(session_closed_callback_t call_bcak)
{
    _closed_callback = call_bcak;
}

void Session::set_received_callback(session_received_callback_t call_bcak)
{
    _received_callback = call_bcak;
}

void Session::set_sent_callback(session_sent_callback_t call_bcak)
{
    _sent_callbcak = call_bcak;
}



void Session::set_return_io_service_pool_fun(std::function<IoServicePoolPtr()> fun)
{
    _get_io_service_pool_fun = fun;
}

const IoServicePoolPtr  Session::get_io_service_pool()
{
    if (_get_io_service_pool_fun)
		return _get_io_service_pool_fun();
    return nullptr;
}

void Session::ShutDown()
{
    close("shutdonw");
}

void Session::OnHeartbeat(const Error_code& ec)
{
    if (_start_heartbeat.load())
    {
        if (ec != Asio::error::operation_aborted)
        {
			Send(nullptr);
        }
		_heartbeat_timer.expires_at(_heartbeat_timer.expires_at() + CHRONO_SECOND(_heartbeat_duration));
        _heartbeat_timer.async_wait([this](const Error_code& ec)
            {
                 OnHeartbeat(ec);
            });
    }

}
void Session::StartHeartbeat(duration_t _time_duration)
{
    _heartbeat_duration = _time_duration;
    _start_heartbeat.store(true);
    if (_start_heartbeat.load())
    {
        _heartbeat_timer.expires_from_now(CHRONO_SECOND(_time_duration));
        _heartbeat_timer.async_wait([this](const Error_code& ec) {
            OnHeartbeat(ec);
            });
    }
}

}
