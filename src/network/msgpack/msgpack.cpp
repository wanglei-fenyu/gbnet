#include "msgpack.hpp"
#include "log/log_help.h"
NAMESPACE_BEGIN(gb::msgpack)

std::vector<uint8_t> pack(sol::variadic_args& args)
{
	auto packer = Packer{};
	for (int i = 0; i < args.size(); ++i)
	{
		packer(args[i]);
	}

	return std::move(packer.move());
}

std::vector<uint8_t> pack(sol::variadic_args&& args)
{
	auto packer = Packer{};
	for (int i = 0; i < args.size(); ++i)
	{
		packer(args[i]);
	}
	return std::move(packer.move());
}

std::vector<uint8_t> pack(sol::protected_function_result& args)
{
	auto packer = Packer{};
	for (int i = 0; i < args.return_count(); ++i)
	{
		packer(args[i]);
	}
	return std::move(packer.move());
}

std::vector<uint8_t> pack(sol::protected_function_result&& args)
{
	auto packer = Packer{};
	for (int i = 0; i < args.return_count(); ++i)
	{
		packer(args[i]);
	}
	return std::move(packer.move());
}





sol::variadic_args unpack(sol::state_view& state, const uint8_t* dataStart, const std::size_t size, std::error_code& ec)
{
	auto unpacker = Unpacker(dataStart, size);
	int i = 0;
	while (!unpacker.empty())
	{
		unpacker(state);
		i++;
	}
	ec = unpacker.ec;
	sol::variadic_args ret(state.lua_state(),-i);
	return ret;
}

sol::variadic_args unpack(sol::state_view& state, const uint8_t* dataStart, const std::size_t size)
{
	auto unpacker = Unpacker(dataStart, size);
	int i = 0;
	while (!unpacker.empty())
	{
		unpacker(state);
		i++;
	}
	sol::variadic_args ret(state.lua_state(),-i);
	return ret;
}

sol::variadic_args unpack(sol::state_view& state, const std::vector<uint8_t>& data, std::error_code& ec)
{
	auto unpacker = Unpacker(data.data(), data.size());
	int i = 0;
	while (!unpacker.empty())
	{
		unpacker(state);
		i++;
	}
	ec = unpacker.ec;
	sol::variadic_args ret(state.lua_state(),-i);
	return ret;

}

sol::variadic_args unpack(sol::state_view& state, const std::vector<uint8_t>& data)
{
	auto unpacker = Unpacker(data.data(), data.size());
	int i = 0;
	while (!unpacker.empty())
	{
		unpacker(state);
		i++;
	}
	sol::variadic_args ret(state.lua_state(),-i);
	return ret;
}

NAMESPACE_END