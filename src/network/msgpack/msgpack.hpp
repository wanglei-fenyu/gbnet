#pragma once

#include <vector>
#include <string>
#include <set>
#include <map>
#include <list>
#include <array>
#include <unordered_map>
#include <unordered_set>
#include <math.h>
#include <chrono>
#include <bitset>

#include <sol/sol.hpp>
#include "common/def.h"
#include "common/define.h"

#ifdef WIN32
#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif
#endif
NAMESPACE_BEGIN(gb::msgpack)

class Packer;
class Unpacker;

enum class UnPackErrorType
{
    eNone = 0,
    eOutRange = 1,
};

struct MsgPackBase
{
    virtual void pack(Packer &packer) = 0;
    virtual void unpack(Unpacker &unpacker) = 0;
};

struct UnpackerErrorCategory : public std::error_category
{
    const char *name() const noexcept override
    {
        return "MsgUnpacker";
    }

    std::string message(int ec) const override
    {
        switch (static_cast<UnPackErrorType>(ec))
        {
        case UnPackErrorType::eOutRange:
            return "unpack out range";
        default:
            return "unpack other error";
        }
    }
};

static const UnpackerErrorCategory sUnpackerErrorCategory{};

inline std::error_code make_error_code(UnPackErrorType error_type)
{
    std::error_code ec(static_cast<int>(error_type), sUnpackerErrorCategory);
    return ec;
}

NAMESPACE_END

// 将解包错误码

namespace std
{
    template <>
    struct is_error_code_enum<gb::msgpack::UnPackErrorType> : public true_type
    {
    };
}

NAMESPACE_BEGIN(gb::msgpack)
enum class format_t : uint8_t
{
    nil = 0xc0,
    false_bool = 0xc2,
    true_bool = 0xc3,
    bin8 = 0xc4,
    bin16 = 0xc5,
    bin32 = 0xc6,
    ext8 = 0xc7,
    ext16 = 0xc8,
    ext32 = 0xc9,
    float32 = 0xca,
    float64 = 0xcb,
    uint8 = 0xcc,
    uint16 = 0xcd,
    uint32 = 0xce,
    uint64 = 0xcf,
    int8 = 0xd0,
    int16 = 0xd1,
    int32 = 0xd2,
    int64 = 0xd3,
    fixext1 = 0xd4,
    fixext2 = 0xd5,
    fixext4 = 0xd6,
    fixext8 = 0xd7,
    fixext16 = 0xd8,
    str8 = 0xd9,
    str16 = 0xda,
    str32 = 0xdb,
    array16 = 0xdc,
    array32 = 0xdd,
    map16 = 0xde,
    map32 = 0xdf,
    table = 0xe0
};

template <typename T>
struct is_containe : public std::false_type
{
};

template <typename T, typename Alloc>
struct is_containe<std::vector<T, Alloc>> : public std::true_type
{
};

template <typename T, typename Alloc>
struct is_containe<std::list<T, Alloc>> : public std::true_type
{
};

template <typename T, typename Alloc>
struct is_containe<std::map<T, Alloc>> : public std::true_type
{
};

template <typename T, typename Alloc>
struct is_containe<std::set<T, Alloc>> : public std::true_type
{
};

template <typename T, typename Alloc>
struct is_containe<std::unordered_map<T, Alloc>> : public std::true_type
{
};

template <typename T, typename Alloc>
struct is_containe<std::unordered_set<T, Alloc>> : public std::true_type
{
};


template <typename T>
struct is_stdarray : public std::false_type
{
};

template <typename T, std::size_t N>
struct is_stdarray<std::array<T, N>> : public std::true_type
{
};

template <typename T>
struct is_map : public std::false_type
{
};

template <typename T, typename Alloc>
struct is_map<std::map<T, Alloc>> : public std::true_type
{
};

template <typename T, typename Alloc>
struct is_map<std::unordered_map<T, Alloc>> : public std::true_type
{
};

template <typename T>
struct is_stack_proxy : public std::false_type
{
};

template <>
struct is_stack_proxy<sol::stack_proxy> : public std::true_type
{
};

template <>
struct is_stack_proxy<sol::state_view> : public std::true_type
{
};

template <>
struct is_stack_proxy<sol::state> : public std::true_type
{
};

template <typename T>
struct has_pack
{
private:
    typedef char yes;
    typedef struct
    {
        char a[2];
    } no;

private:
    template <typename T1>
    static yes &has(decltype(&T1::pack));

    template <typename T1>
    static no &has(...);

public:
    static const bool value = sizeof(has<T>(nullptr)) == sizeof(yes);
};

template <typename T>
struct has_constchar : public std::false_type
{
};

template <>
struct has_constchar<const char *> : public std::true_type
{
};

template <size_t N>
struct has_constchar<const char[N]> : public std::true_type
{
};

template <size_t N>
struct has_constchar<char[N]> : public std::true_type
{
};

template <size_t N>
struct has_constchar<const char (&)[N]> : public std::true_type
{
};

template <size_t N>
struct has_constchar<char (&)[N]> : public std::true_type
{
};

template <size_t Idx = 0, typename F, typename... Args>
inline typename std::enable_if_t<Idx == sizeof...(Args)> for_each(std::tuple<Args...> &&, F)
{
}

template <size_t Idx = 0, typename F, typename... Args>
inline typename std::enable_if_t<Idx < sizeof...(Args)> for_each(std::tuple<Args...> &&tue, F &&f)
{
    f(std::get<Idx>(tue));
    for_each<Idx + 1, F, Args...>(tue, std::move(f));
}

template <size_t Idx = 0, typename F, typename... Args>
inline typename std::enable_if_t<Idx == sizeof...(Args)> for_each(std::tuple<Args...> &, F)
{
}

template <size_t Idx = 0, typename F, typename... Args>
inline typename std::enable_if_t<Idx < sizeof...(Args)> for_each(std::tuple<Args...> &tue, F &&f)
{
    f(std::get<Idx>(tue));
    for_each<Idx + 1, F, Args...>(tue, std::move(f));
}

class Packer
{
public:
    template <typename... Args>
    void operator()(const Args &...args)
    {
        (pack_type(std::forward<const Args &>(args)), ...);
    }

    template <typename... Args>
    void process(const Args &...args)
    {
        (pack_type(std::forward<const Args &>(args)), ...);
    }

public:
    const std::vector<uint8_t> &vector() const
    {
        return serializedObj_;
    }

    std::vector<uint8_t> &&move()
    {
        return std::move(serializedObj_);
    }

    void clear()
    {
        serializedObj_.clear();
    }

private:
    template <typename T>
    void pack_type(const T &value)
    {
        if constexpr (std::is_enum<typename std::decay<T>::type>::value) // 枚举
        {
            pack_type((int32_t)value);
        }
        else if constexpr (is_map<typename std::decay<T>::type>::value)
        {
            pack_map(value);
        }
        else if constexpr (is_containe<typename std::decay<T>::type>::value || is_stdarray<typename std::decay<T>::type>::value)
        {
            pack_array(value);
        }
        else if constexpr (is_stack_proxy<typename std::decay<T>::type>::value)
        {
            pack_stack_proxy(value);
        }
        else if constexpr (has_pack<typename std::decay<T>::type>::value)
        {
            const_cast<T &>(value).pack(*this);
        }
        else if constexpr (std::is_same_v<typename std::decay<T>::type, const char *> || std::is_same_v<typename std::decay<T>::type, std::string_view> || has_constchar<T>::value)
        {
            pack_type(std::string(value));
        }
        else
        {
        }
    }

    template <typename T>
    void pack_type(const std::chrono::time_point<T> &value)
    {
        pack_type(value.time_since_epoch().count());
    }

    template <typename T>
    void pack_array(const T &array)
    {
        if (array.size() < std::numeric_limits<uint16_t>::max())
        {
            serializedObj_.emplace_back((uint8_t)format_t::array16);
            for (auto i = sizeof(uint16_t); i > 0; --i)
            {
                serializedObj_.emplace_back(uint8_t(array.size() >> (8 * (i - 1)) & 0xff)); // 8 == uint8_t
            }
        }
        else if (array.size() < std::numeric_limits<uint32_t>::max())
        {
            serializedObj_.emplace_back((uint8_t)format_t::array32);
            for (auto i = sizeof(uint32_t); i > 0; --i)
            {
                serializedObj_.emplace_back(uint8_t(array.size() >> (8 * (i - 1)) & 0xff));
            }
        }
        else
        {
            return;
        }
        for (const auto &item : array)
        {
            pack_type(item);
        }
    }

    template <typename T>
    void pack_map(const T &map)
    {
        if (map.size() < std::numeric_limits<uint16_t>::max())
        {
            serializedObj_.emplace_back((uint8_t)format_t::map16);
            for (auto i = sizeof(uint16_t); i > 0; --i)
            {
                serializedObj_.emplace_back(uint8_t(map.size() >> (8 * (i - 1)) & 0xff));
            }
        }
        else if (map.size() < std::numeric_limits<uint32_t>::max())
        {
            serializedObj_.emplace_back((uint8_t)format_t::map32);
            for (auto i = sizeof(uint32_t); i > 0; --i)
            {
                serializedObj_.emplace_back(uint8_t(map.size() >> (8 * (i - 1)) & 0xff));
            }
        }
        for (auto &item : map)
        {
            pack_type(std::get<0>(item));
            pack_type(std::get<1>(item));
        }
    }

    template <typename T>
    void pack_table(const T &table)
    {
        serializedObj_.emplace_back((uint8_t)format_t::table);
        size_t index = serializedObj_.size();
        size_t count = 0;
        size_t offset = 0;
        for (int i = sizeof(uint32_t); i > 0; --i)
        {
            serializedObj_.emplace_back(uint8_t(0));
        }
        for (auto it = table.begin(); it != table.end(); ++it)
        {
            count++;
            auto &p = *it;
            pack_stack_proxy(p.first);
            pack_stack_proxy(p.second);
        }

        for (auto i = sizeof(uint32_t); i > 0; --i)
        {
            serializedObj_[index + offset] = (uint8_t(count >> (8 * (i - 1)) & 0xff));
            offset++;
        }
    }

    template <typename T>
    void pack_stack_proxy(const T &obj)
    {
        auto type = obj.get_type();
        switch (type)
        {
        case sol::type::lua_nil:
            pack_type(nullptr);
            break;
        case sol::type::number:
            obj.push();
            if (lua_isinteger(obj.lua_state(), -1)) // 如果是整数
            {
                pack_type(obj.template as<int64_t>());
            }
            else
            {
                pack_type(obj.template as<double>());
            }
            lua_pop(obj.lua_state(), 1);
            break;
        case sol::type::string:
            pack_type(obj.template as<std::string>());
            break;
        case sol::type::boolean:
            pack_type(obj.template as<bool>());
            break;
        case sol::type::table:
            obj.push();
            pack_table(sol::table(obj.lua_state(), -1));
            lua_pop(obj.lua_state(), 1);
            break;
        case sol::type::userdata:
            if (obj.template is<MsgPackBase>())
            {
                MsgPackBase &msg_base = obj.template as<MsgPackBase>();
                msg_base.pack(*this);
            }
            break;
        default:
            break;
        }
    }



private:
    std::bitset<8> to_binary(int8_t value)
    {
        if (value < 0)
        {
            auto abs_value = std::abs(value);
            return ~abs_value + 1;
        }
        else
        {
            return {(uint8_t)value};
        }
    }

    std::bitset<16> to_binary(int16_t value)
    {
        if (value < 0)
        {
            auto abs_value = std::abs(value);
            return ~abs_value + 1;
        }
        else
        {
            return {(uint16_t)value};
        }
    }

    std::bitset<32> to_binary(int32_t value)
    {
        if (value < 0)
        {
            auto abs_value = std::abs(value);
            return ~abs_value + 1;
        }
        else
        {
            return {(uint32_t)value};
        }
    }

    std::bitset<64> to_binary(int64_t value)
    {
        if (value < 0)
        {
            auto abs_value = std::abs(value);
            return ~abs_value + 1;
        }
        else
        {
            return {(uint64_t)value};
        }
    }

private:
    std::vector<uint8_t> serializedObj_;
};


template <>
inline void Packer::pack_type(const int8_t &value)
{
	serializedObj_.emplace_back((uint8_t)format_t::int8);
	serializedObj_.emplace_back(uint8_t(to_binary(value).to_ulong())); // 拿到一个与bitset 相通位的一个数字 方便存储
}

template <>
inline void Packer::pack_type(const int16_t &value)
{
	if (std::abs(value) < std::abs(std::numeric_limits<int8_t>::min()))
	{
		pack_type(int8_t(value));
	}
	else
	{
		serializedObj_.emplace_back((uint8_t)format_t::int16);
		auto serialize_val = uint16_t(to_binary(value).to_ulong());
		for (int i = sizeof(value); i > 0; --i)
		{
			serializedObj_.emplace_back(uint8_t(serialize_val >> (8 * (i - 1)) & 0xff));
		}
	}
}

template <>
inline void Packer::pack_type(const int32_t &value)
{
	if (std::abs(value) < std::abs(std::numeric_limits<int16_t>::min()))
	{
		pack_type((int16_t)value);
	}
	else
	{
		serializedObj_.emplace_back((uint8_t)format_t::int32);
		auto serialize_val = uint32_t(to_binary(value).to_ulong());
		for (int i = sizeof(value); i > 0; --i)
		{
			serializedObj_.emplace_back(uint8_t(serialize_val >> (8 * (i - 1)) & 0xff));
		}
	}
}

template <>
inline void Packer::pack_type(const int64_t &value)
{
	if (std::abs(value) < std::abs(std::numeric_limits<int32_t>::min()))
	{
		pack_type((int32_t)value);
	}
	else
	{
		serializedObj_.emplace_back((uint8_t)format_t::int64);
		auto serialize_val = uint64_t(to_binary(value).to_ulong());
		for (int i = sizeof(value); i > 0; --i)
		{
			serializedObj_.emplace_back(uint8_t(serialize_val >> (8 * (i - 1)) & 0xff));
		}
	}
}

template <>
inline void Packer::pack_type(const uint8_t &value)
{
	serializedObj_.emplace_back((uint8_t)format_t::uint8);
	serializedObj_.emplace_back(value);
}

template <>
inline void Packer::pack_type(const uint16_t &value)
{
	if (value > std::numeric_limits<uint8_t>::max())
	{
		serializedObj_.emplace_back((uint8_t)format_t::uint16);
		for (auto i = sizeof(value); i > 0; --i)
		{
			serializedObj_.emplace_back(uint8_t(value >> (8 * (i - 1)) & 0xff));
		}
	}
	else
	{
		pack_type(uint8_t(value));
	}
}

template <>
inline void Packer::pack_type(const uint32_t &value)
{
	if (value > std::numeric_limits<uint16_t>::max())
	{
		serializedObj_.emplace_back((uint8_t)format_t::uint32);
		for (auto i = sizeof(value); i > 0U; --i)
		{
			serializedObj_.emplace_back(uint8_t(value >> (8 * (i - 1)) & 0xff));
		}
	}
	else
	{
		pack_type(uint16_t(value));
	}
}

template <>
inline void Packer::pack_type(const uint64_t &value)
{
	if (value > std::numeric_limits<uint32_t>::max())
	{
		serializedObj_.emplace_back((uint8_t)format_t::uint64);
		for (auto i = sizeof(value); i > 0U; --i)
		{
			serializedObj_.emplace_back(uint8_t(value >> (8 * (i - 1)) & 0xff));
		}
	}
	else
	{
		pack_type(uint32_t(value));
	}
}

template <>
inline void Packer::pack_type(const float &value) 
{
	int32_t i = (int32_t)value;
	if (float(i) == value) 
	{
		pack_type(i);
	}
	else 
	{
		serializedObj_.emplace_back((uint8_t)format_t::float32);
		uint8_t *s = (uint8_t *)&value;
		for (auto i = 0; i < sizeof(float); ++i) 
		{
			serializedObj_.emplace_back(s[i]);
		}
	}
}

template <>
inline void Packer::pack_type(const double &value) 
{
	int64_t i = (int64_t)value;
	if (double(i) == value) 
	{
		pack_type(i);
	}
	else
	{
		serializedObj_.emplace_back((uint8_t)format_t::float64);
		uint8_t *s = (uint8_t *)&value;
		for (auto i = 0; i < sizeof(double); ++i)
		{
			serializedObj_.emplace_back(s[i]);
		}
	}
}



template<>
inline void Packer::pack_type(const std::nullptr_t& none)
{
	serializedObj_.emplace_back((uint8_t)format_t::nil);
}

template <>
inline void Packer::pack_type(const bool &value) 
{
	if (value) 
		serializedObj_.emplace_back((uint8_t)format_t::true_bool);
	else 
		serializedObj_.emplace_back((uint8_t)format_t::false_bool);
}

template <>
inline void Packer::pack_type(const std::string& str)
{
	if (str.size() < std::numeric_limits<uint8_t>::max())
	{
		serializedObj_.emplace_back((uint8_t)format_t::str8);
		serializedObj_.emplace_back(uint8_t(str.size()));
	}
	else if (str.size() < std::numeric_limits<uint16_t>::max())
	{
		serializedObj_.emplace_back((uint8_t)format_t::str16);
		for (auto i = sizeof(uint16_t); i > 0; --i)
		{
			serializedObj_.emplace_back(uint8_t(str.size() >> (8 * (i - 1)) & 0xff));
		}
	}
	else if (str.size() < std::numeric_limits<uint32_t>::max())
	{
		serializedObj_.emplace_back((uint8_t)format_t::str32);
		for (auto i = sizeof(uint32_t); i > 0; --i)
		{
			serializedObj_.emplace_back(uint8_t(str.size() >> (8 * (i - 1)) & 0xff));
		}
	}
	else 
	{
		return;
	}

	for (const auto& c : str)
	{
		serializedObj_.emplace_back(uint8_t(c));
	}
}


template<>
inline void Packer::pack_type(const std::vector<uint8_t>& data)
{
	if (data.size() < std::numeric_limits<uint8_t>::max())
	{
		serializedObj_.emplace_back((uint8_t)format_t::bin8);
		serializedObj_.emplace_back((uint8_t)data.size());
	}
	else if (data.size() < std::numeric_limits<uint16_t>::max())
	{
		serializedObj_.emplace_back((uint8_t)format_t::bin16);
		for (int i = sizeof(uint16_t); i > 0; --i)
		{
			serializedObj_.emplace_back(uint8_t(data.size() >> (8 * (i - 1)) & 0xff));
		}
	}
	else if (data.size() < std::numeric_limits<uint32_t>::max())
	{
		serializedObj_.emplace_back((uint8_t)format_t::bin32);
		for (int i = sizeof(uint32_t); i > 0; --i)
		{
			serializedObj_.emplace_back(uint8_t(data.size() >> (8 * (i - 1)) & 0xff));
		}
	}
	else 
	{
		return;
	}

	for (const auto& b : data)
	{
		serializedObj_.emplace_back(b);
	}
}


template<typename... Args>
std::vector<uint8_t> pack(Args& ...args)
{
    auto packer = Packer{};
    auto args_tuple = std::forward_as_tuple(std::forward<Args>(args)...);
    for_each(args_tuple, [&](auto& obj)
    {
        if constexpr (std::is_same<typename std::decay<decltype(obj)>::type, sol::variadic_args>::value)
        {
            for (int i = 0; i < obj.size(); i++)
            {
                packer(obj[i]);
            }
        }
        else if constexpr (std::is_same<typename std::decay<decltype(obj)>::type, sol::protected_function_result>::value)
        {
            for (int i = 0; i < obj.retuen_count(); i++)
            {
                packer(obj[i]);
            }
        }
        else if constexpr (has_pack<typename std::decay<decltype(obj)>::type>::value)
        {
            obj.pack(packer);
        }
        else
        {
            packer(obj);
        }
    });
    return std::move(packer.move());
}




template<typename... Args>
std::vector<uint8_t> pack(Args&& ...args)
{
    auto packer = Packer{};
    auto args_tuple = std::forward_as_tuple(std::forward<Args>(args)...);
    for_each(args_tuple, [&](auto& obj)
    {
        if constexpr (std::is_same<typename std::decay<decltype(obj)>::type, sol::variadic_args>::value)
        {
            for (int i = 0; i < obj.size(); i++)
            {
                packer(obj[i]);
            }
        }
        else if constexpr (std::is_same<typename std::decay<decltype(obj)>::type, sol::protected_function_result>::value)
        {
            for (int i = 0; i < obj.return_count(); i++)
            {
                packer(obj[i]);
            }
        }
        else if constexpr (has_pack<typename std::decay<decltype(obj)>::type>::value)
        {
            obj.pack(packer);
        }
        else
        {
            packer(obj);
        }
    });
    return std::move(packer.move());
}



template<typename... Args>
std::vector<uint8_t> pack(std::tuple<Args...>& args)
{
    auto packer = Packer{};
    for_each(std::forward<std::tuple<Args...>>(args), [&](auto& obj)
    {
        if constexpr (std::is_same<typename std::decay<decltype(obj)>::type, sol::variadic_args>::value)
        {
            for (int i = 0; i < obj.size(); i++)
            {
                packer(obj[i]);
            }
        }
        else if constexpr (std::is_same<typename std::decay<decltype(obj)>::type, sol::protected_function_result>::value)
        {
            for (int i = 0; i < obj.retuen_count(); i++)
            {
                packer(obj[i]);
            }
        }
        else if constexpr (has_pack<typename std::decay<decltype(obj)>::type>::value)
        {
            obj.pack(packer);
        }
        else
        {
            packer(obj);
        }
    });
    return std::move(packer.move());
}

template<typename... Args>
std::vector<uint8_t> pack(std::tuple<Args...>&& args)
{
    auto packer = Packer{};
    for_each(std::forward<std::tuple<Args...>>(args), [&](auto& obj)
    {
        if constexpr (std::is_same<typename std::decay<decltype(obj)>::type, sol::variadic_args>::value)
        {
            for (int i = 0; i < obj.size(); i++)
            {
                packer(obj[i]);
            }
        }
        else if constexpr (std::is_same<typename std::decay<decltype(obj)>::type, sol::protected_function_result>::value)
        {
			for (int i = 0; i < obj.retuen_count(); i++)
			{
			   packer(obj[i]);
			}
        }
        else if constexpr (has_pack<typename std::decay<decltype(obj)>::type>::value)
        {
            obj.pack(packer);
        }
        else
        {
            packer(obj);
        }
    });
    return std::move(packer.move());
}


std::vector<uint8_t> pack(sol::variadic_args &args);
std::vector<uint8_t> pack(sol::variadic_args &&args);
std::vector<uint8_t> pack(sol::protected_function_result &args);
std::vector<uint8_t> pack(sol::protected_function_result &&args);




class Unpacker
{

public:
    Unpacker():dataPointer_(nullptr),dataEnd_(nullptr){}
    Unpacker(const uint8_t* dataStart, std::size_t size) : dataPointer_(dataStart) ,dataEnd_(dataStart+size){};
public:
    std::error_code ec{};
public:
    template<typename... Types>
    void operator()(Types& ...args)
    {
        (unpack_type(std::forward<Types&>(args)), ...);
    }

    template<typename... Types>
    void process(Types& ...args)
    {
        (unpack_type(std::forward<Types&>(args)), ...);
    }
    
    void set_data(const uint8_t* start, std::size_t size)
    {
        dataPointer_ = start;
        dataEnd_ = start + size;
    }

    bool empty() 
    {
        return dataEnd_ - dataPointer_ <= 0;
    }

private:

    uint8_t safe_data()
    {
        if (dataPointer_ < dataEnd_)
            return *dataPointer_;
        ec = make_error_code(UnPackErrorType::eOutRange);
        return 0;
    }

    void safe_incremen(std::size_t size = 1)
    {
        if (dataEnd_ - dataPointer_ >= 0)
        {
            dataPointer_ += size;
        }
        else
        {
			ec = make_error_code(UnPackErrorType::eOutRange);
        }
    }

    //template<typename T>
    //void unpack_type(T& value)
    //{
    //    if constexpr (std::is_enum<typename std::decay<T>::type>::value)
    //    {
    //        int32_t v = 0;
    //        unpack_type(v);
    //        value = (T)v;
    //    }

    //}

	template <class T>
    void unpack_type(T &value) 
    {
        if constexpr (std::is_enum<typename std::decay<T>::type>::value)
        {
            int32_t v = 0;
            unpack_type(v);
            value = (T)v;
        } else if constexpr (is_map<typename std::decay<T>::type>::value)
        {
            unpack_map(value);
        } else if constexpr (is_containe<typename std::decay<T>::type>::value)
        {
            unpack_array(value);
        } else if constexpr (is_stdarray<typename std::decay<T>::type>::value) 
        {
            unpack_stdarray(value);
        } else if constexpr (is_stack_proxy<typename std::decay<T>::type>::value)
        {
             unpack_stack_proxy(value);
        } else {
            if constexpr (has_pack<typename std::decay<T>::type>::value)
            {
                value.unpack(*this);
            }
        }
    }

    template<class Clock,class Duration>
    void unpack_type(std::chrono::time_point<Clock, Duration>& value)
    {
        using RepType = typename std::chrono::time_point<Clock, Duration>::rep;
        using DurationType = Duration;
        using TimepointType = typename std::chrono::time_point<Clock, Duration>;
        auto placeholder = RepType{};
        unpack_type(placeholder);
        value = TimepointType(DurationType(placeholder));
    }
    
    template<class T>
    void unpack_array(T& array)
    {
        using ValueType = typename T::vlaue_type;
        if (safe_data() == (uint8_t)format_t::array32)
        {
            safe_incremen();
            std::size_t array_size = 0;
            for (auto i = sizeof(uint32_t); i > 0; --i)
            {
                array_size += uint32_t(safe_data()) << 8 * (i-1);
                safe_incremen();
            }
            if (array_size)
            {
                array.reserve(array_size);
            }
            for (auto i = 0; i < array_size; ++i)
            {
                ValueType val{};
                unpack_type(val);
                array.emplace_back(val);
            }
        }
        else if (safe_data() == (uint8_t)format_t::array16)
        {
            safe_incremen();
            std::size_t array_size = 0;
            for (auto i = sizeof(uint16_t); i > 0; --i)
            {
                array_size += (uint16_t)(safe_data()) << 8 * (i - 1);
                safe_incremen();
            }
            if (array_size > 0)
            {
                array.reserve(array_size);
            }
            for (auto i = 0; i < array_size; ++i)
            {
                ValueType val{};
                unpack_type(val);
                array.emplace_back(val);
            }
        }
        else if (safe_data() == (uint8_t)format_t::table)
        {
            safe_incremen();
            std::size_t table_size = 0;
            for (auto i = sizeof(uint32_t); i > 0; --i)
            {
                table_size += uint32_t(safe_data()) << 8 * (i - 1);
                safe_incremen();
            }
            if (table_size > 0)
            {
                array.resize(table_size);
            }
            for (auto i = 0; i < table_size; ++i)
            {
                int64_t key{};
                ValueType val{};
                unpack_type(key);
                unpack_type(val);
                array[key - 1] = std::move(val);
            }
        }
        else
        {
            std::size_t array_size = safe_data() & 0b00001111;
            safe_incremen();
            if (array_size)
            {
                array.reserver(array_size);
            }
            for (auto i = 0; i < array_size; ++i)
			{
                ValueType val{};
                unpack_array(val);
                array.emplace_back(val);
			}
        }
    }

    template<typename T>
    void unpack_stdarray(T& array)
    {
        using ValueType = typename T::value;
        auto vec = std::vector<ValueType>{};
        unpack_array(vec);
        std::copy(vec.begin(), vec.end(), array.begin());
    }

    template<typename T>
    void unpack_map(T& map)
    {
        using KeyType = typename T::key_type;
        using MappedType = typename T::mapped_type;
        if (safe_data() == (uint8_t)format_t::map32 || safe_data() == (uint8_t)format_t::table)
        {
            safe_incremen();
            std::size_t map_size = 0;
            for (auto i = sizeof(uint32_t); i > 0; --i)
            {
                map_size += uint32_t(safe_data()) << 8 * (i - 1);
                safe_incremen();
            }
            if (map_size)
            {
                map.reserve(map_size);
            }
            for (auto i = 0; i < map_size; ++i)
            {
                KeyType key{};
                MappedType val{};
                unpack_type(key);
                unpack_type(val);
                map.insert_or_assign(key, val);
            }
        }
        else if (safe_data() == (uint8_t)format_t::map16)
        {
            safe_incremen();
            std::size_t map_size = 0;
            for (auto i = sizeof(uint16_t); i > 0; --i)
            {
                map_size += uint16_t(safe_data()) << 8 * (i - 1);
                safe_incremen();
            }
            if (map_size)
            {
                map.reserve(map_size);
            }
            for (auto i = 0; i < map_size; ++i)
            {
                KeyType key{};
                MappedType val{};
                unpack_type(key);
                unpack_type(val);
                map.insert_or_assign(key, val);
            }

        }
    }

 
    template<typename T>
    void unpack_stack_proxy(T& state)
    {
        uint8_t byte = safe_data();
        switch (byte)
        {
        case(uint8_t)format_t::nil:
        {
            lua_pushnil(state.lua_state());
            safe_incremen();
            break;
        }
        case (uint8_t)format_t::false_bool:
        {
            lua_pushboolean(state.lua_state(), false);
            safe_incremen();
            break;
        }
        case (uint8_t)format_t::true_bool:
        {
            lua_pushboolean(state.lua_state(), true);
            safe_incremen();
            break;
        }
        case (uint8_t)format_t::bin8:
        case (uint8_t)format_t::bin16:
        case (uint8_t)format_t::bin32:
        {
            std::vector<uint8_t> value;
            unpack_type(value);
            lua_pushlstring(state.lua_state(), (char*)(value.data()), value.size());
            break;
        }
        case (uint8_t)format_t::float32:
        {
            float value = 0.f;
            unpack_type(value);
            lua_pushnumber(state.lua_state(), value);
            break;
        }
        case (uint8_t)format_t::float64:
        {
            double value = 0.f;
            unpack_type(value);
            lua_pushnumber(state.lua_state(), value);
            break;
        }
        case (uint8_t)format_t::uint8:
        case (uint8_t)format_t::uint16:
        case (uint8_t)format_t::uint32:
        case (uint8_t)format_t::uint64:
        {
            uint64_t value = 0;
            unpack_type(value);
            lua_pushinteger(state.lua_state(), value);
            break;
        }
        case (uint8_t)format_t::int8:
        case (uint8_t)format_t::int16:
        case (uint8_t)format_t::int32:
        case (uint8_t)format_t::int64:
        {
            int64_t value = 0;
            unpack_type(value);
            lua_pushinteger(state.lua_state(), value);
            break;
        }
        case (uint8_t)format_t::str8:
        case (uint8_t)format_t::str16:
        case (uint8_t)format_t::str32:
        {
            std::string value;
            unpack_type(value);
            lua_pushlstring(state.lua_state(), value.c_str(), value.size());
            break;
        }
        case (uint8_t)format_t::array16:
        case (uint8_t)format_t::array32:
        case (uint8_t)format_t::map16:
        case (uint8_t)format_t::map32:
        case (uint8_t)format_t::table:
        {
            unpack_table(state);
            break;
        }

        default:
        {
            if (byte >= 0 && byte <= 31)        //32bit之内
            {
                uint8_t value = 0;
                unpack_type(value);
                lua_pushinteger(state.lua_state(), value);
            }
            else if (byte >= 128 && byte < 160)
            {
                unpack_table(state);
            }
            else if (byte >= 160 && byte < 192)
            {
                std::size_t strSize = byte & 0b00011111;
                safe_incremen();
				if (dataPointer_ + strSize <= dataEnd_) 
                {
					lua_pushlstring(state.lua_state(), (char *)(dataPointer_), strSize);
                    safe_incremen(strSize);
				} else 
                {
                    ec = UnPackErrorType::eOutRange;
				}
            }
            else 
            {
                int8_t value = 0;
				unpack_type(value);
				lua_pushinteger(state.lua_state(), value);
            }
            break;
        }
        }
    }
    
    
    template<typename T>
    void unpack_table(T& state)
    {
        uint8_t byte = safe_data();
        if (byte == (uint8_t)format_t::array32)
        {
            lua_newtable(state.lua_state());
            safe_incremen();
            std::size_t array_size = 0;
            for (auto i = sizeof(uint32_t); i > 0; --i)
            {
                array_size += uint32_t(safe_data()) << 8 * (i - 1);
                safe_incremen();
            }
            for (auto i = 0; i < array_size; ++i)
            {
                unpack_stack_proxy(state);
                lua_rawseti(state.lua_state(), -2, i + 1);
            }
        }
        else if (byte == (uint8_t)format_t::array16)
        {
            lua_newtable(state.lua_state());
            safe_incremen();
            std::size_t array_size = 0;
            for (auto i = sizeof(uint16_t); i > 0; --i)
            {
                array_size += uint16_t(safe_data()) << 8 * (i - 1);
                safe_incremen();
            }
            for (auto i = 0; i < array_size; ++i)
            {
                unpack_stack_proxy(state);
                lua_rawseti(state.lua_state(), -2, i + 1);
            }
        }
        else if (byte == (uint8_t)format_t::map32 || byte == (uint8_t)format_t::table)
        {
            lua_newtable(state.lua_state());
            safe_incremen();
            std::size_t map_size = 0;
            for (auto i = sizeof(uint32_t); i > 0; --i)
            {
                map_size += uint32_t(safe_data()) << 8 * (i-1);
                safe_incremen();
            }
            for (auto i = 0; i < map_size; ++i)
            {
                unpack_stack_proxy(state);
                unpack_stack_proxy(state);
                lua_rawset(state.lua_state(), -3);
            }
        }
        else if(byte == 144)
        {
            lua_newtable(state.lua_state());
            std::size_t map_size = safe_data() & 0b00001111;
            safe_incremen();
            for (auto i = 0; i < map_size; ++i)
            {
                unpack_stack_proxy(state);
                unpack_stack_proxy(state);
                lua_rawset(state.lua_state(), -3);
            }
        }
        else
        {
            lua_newtable(state.lua_state());
            std::size_t map_size = safe_data() & 0b00001111;
            safe_incremen();
            for (auto i = 0; i < map_size; ++i)
            {
                unpack_stack_proxy(state);
                lua_rawseti(state.lua_state(), -2,i+1);
            }
        }
    }

    template<typename T>
    void unpack_number(T& val)
    {
        if constexpr (std::is_arithmetic<T>::value)
        {
            if (safe_data() == (uint8_t)format_t::uint64)
            {
                uint64_t value = 0;
                safe_incremen();
                for (auto i = sizeof(uint64_t); i > 0; --i) {
                    value += uint64_t(safe_data()) << 8 * (i - 1);
                    safe_incremen();
                }
                val = (T)(value);
            }
            else if (safe_data() == (uint8_t)format_t::uint32)
            {
                uint32_t value = 0;
                safe_incremen();
                for (auto i = sizeof(uint32_t); i > 0; --i) {
                    value += uint32_t(safe_data()) << 8 * (i - 1);
                    safe_incremen();
                }
                val = (T)(value);
            }
            else if (safe_data() == (uint8_t)format_t::uint16)
            {
                uint16_t value = 0;
                safe_incremen();
                for (auto i = sizeof(uint16_t); i > 0; --i) {
                    value += uint16_t(safe_data()) << 8 * (i - 1);
                    safe_incremen();
                }
                val = (T)(value);
            }
            else if (safe_data() == (uint8_t)format_t::uint8)
            {
                uint8_t value = 0;
                safe_incremen();
                value = safe_data();
                safe_incremen();
                val = (T)(value);
            }
            else if (safe_data() == (uint8_t)format_t::int64) 
            {
                int64_t value = 0;
                safe_incremen();
                std::bitset<64> bits;
                for (auto i = sizeof(value); i > 0; --i) {
                    bits |= std::bitset<8>(safe_data()).to_ullong() << 8 * (i - 1);
                    safe_incremen();
                }
                if (bits[63]) {
                    value = -1 * ((~bits).to_ullong() + 1);
                } else {
                    value = bits.to_ullong();
                }
                val = (T)(value);
            } 
            else if (safe_data() == (uint8_t)format_t::int32) 
            {
                int32_t value = 0;
                safe_incremen();
                std::bitset<32> bits;
                for (auto i = sizeof(uint32_t); i > 0; --i) 
                {
                    bits |= uint32_t(safe_data()) << 8 * (i - 1);
                    safe_incremen();
                }
                if (bits[31]) {
                    value = -1 * ((~bits).to_ulong() + 1);
                } else {
                    value = bits.to_ulong();
                }
                val = (T)(value);
            } else if (safe_data() == (uint8_t)format_t::int16) 
            {
                int16_t value = 0;
                safe_incremen();
                std::bitset<16> bits;
                for (auto i = sizeof(uint16_t); i > 0; --i) 
                {
                    bits |= uint16_t(safe_data()) << 8 * (i - 1);
                    safe_incremen();
                }
                if (bits[15]) {
                    value = -1 * (uint16_t((~bits).to_ulong()) + 1);
                } else {
                    value = uint16_t(bits.to_ulong());
                }
                val = (T)(value);
            } else if (safe_data() == (uint8_t)format_t::int8) 
            {
                int8_t value = 0;
                safe_incremen();
                value = safe_data();
                safe_incremen();
                val = (T)(value);
            }
            else if (safe_data() == (uint8_t)format_t::float32) 
            {
                float value = 0.f;
                safe_incremen();
                uint8_t *s = (uint8_t *)&value;
                for (auto i = 0U; i < sizeof(float); ++i) 
                {
                    s[i] = safe_data();
                    safe_incremen();
                }
                val = (T)(value);
            } else if (safe_data() == (uint8_t)format_t::float64) 
            {
                double value = 0.f;
                safe_incremen();
                uint8_t *s = (uint8_t *)&value;
                for (auto i = 0U; i < sizeof(double); ++i) 
                {
                    s[i] = safe_data();
                    safe_incremen();
                }
                val = (T)(value);
            }
        }
    }
    


private:
    const uint8_t* dataPointer_;
    const uint8_t* dataEnd_;
};



template <>
inline void Unpacker::unpack_type(int8_t &value) 
{
	unpack_number(value);
}

template <>
inline void Unpacker::unpack_type(int16_t &value)
{
	unpack_number(value);
}

template <>
inline void Unpacker::unpack_type(int32_t &value)
{
	unpack_number(value);
}

template <>
inline void Unpacker::unpack_type(int64_t &value) 
{
	unpack_number(value);
}

template <>
inline void Unpacker::unpack_type(uint8_t &value) 
{
	unpack_number(value);
}

template <>
inline void Unpacker::unpack_type(uint16_t &value)
{
	unpack_number(value);
}

template <>
inline void Unpacker::unpack_type(uint32_t &value)
{
	unpack_number(value);
}

template <>
inline void Unpacker::unpack_type(uint64_t &value) 
{
	unpack_number(value);
}

template <>
inline void Unpacker::unpack_type(std::nullptr_t & /*value*/) 
{
	safe_incremen();
}

template <>
inline void Unpacker::unpack_type(bool &value) 
{
	if (safe_data() == (uint8_t)format_t::false_bool)
	{
		value = false;
		safe_incremen();
	} else if (safe_data() == (uint8_t)format_t::true_bool) 
	{
		value = true;
		safe_incremen();
	} else 
	{
		unpack_number(value);
	}
}

template <>
inline void Unpacker::unpack_type(float &value)
{
	unpack_number(value);
}

template <>
inline void Unpacker::unpack_type(double &value)
{
	unpack_number(value);
}

template <>
inline void Unpacker::unpack_type(std::string &value)
{
	std::size_t strSize = 0;
	if (safe_data() == (uint8_t)format_t::str32)
	{
		safe_incremen();
		for (auto i = sizeof(uint32_t); i > 0; --i)
		{
			strSize += uint32_t(safe_data()) << 8 * (i - 1);
			safe_incremen();
		}
	} else if (safe_data() == (uint8_t)format_t::str16) 
	{
		safe_incremen();
		for (auto i = sizeof(uint16_t); i > 0; --i) 
		{
			strSize += uint16_t(safe_data()) << 8 * (i - 1);
			safe_incremen();
		}
	} else if (safe_data() == (uint8_t)format_t::str8)
	{
		safe_incremen();
		for (auto i = sizeof(uint8_t); i > 0; --i)
		{
			strSize += uint8_t(safe_data()) << 8 * (i - 1);
			safe_incremen();
		}
	} else {
		return;
	}
	if (dataPointer_ + strSize <= dataEnd_) 
	{
		value = std::string{dataPointer_, dataPointer_ + strSize};
		safe_incremen(strSize);
	} else 
	{
		ec = UnPackErrorType::eOutRange;
	}
}

template <>
inline void Unpacker::unpack_type(std::vector<uint8_t> &value)
{
	std::size_t binSize = 0;
	if (safe_data() == (uint8_t)format_t::bin32)
	{
		safe_incremen();
		for (auto i = sizeof(uint32_t); i > 0; --i)
		{
			binSize += uint32_t(safe_data()) << 8 * (i - 1);
			safe_incremen();
		}
	}
	else if (safe_data() == (uint8_t)format_t::bin16) 
	{
		safe_incremen();
		for (auto i = sizeof(uint16_t); i > 0; --i) 
		{
			binSize += uint16_t(safe_data()) << 8 * (i - 1);
			safe_incremen();
		}
	} else 
	{
		safe_incremen();
		for (auto i = sizeof(uint8_t); i > 0; --i) {
			binSize += uint8_t(safe_data()) << 8 * (i - 1);
			safe_incremen();
		}
	}
	if (dataPointer_ + binSize <= dataEnd_)
	{
		value = std::vector<uint8_t>{dataPointer_, dataPointer_ + binSize};
		safe_incremen(binSize);
	} else
	{
		ec = UnPackErrorType::eOutRange;
	}
}


template<typename... Args>
std::tuple<Args...> unpack(const uint8_t *dataStart, const std::size_t size, std::error_code &ec)
{
    auto unpacker = Unpacker(dataStart, size);
    std::tuple<Args...> objs;
    for_each(objs, [&](auto& obj)
	{
		if constexpr (has_pack<typename std::decay<decltype(obj)>::type>::value) {
			obj.unpack(unpacker);
		} else {
			unpacker(obj);
		}
	});
    ec = unpacker.ec;
    return objs;
}

template <class... Args>
std::tuple<Args...> unpack(const uint8_t *dataStart, const std::size_t size)
{
    std::error_code ec{};
    return unpack<Args...>(dataStart, size, ec);
}

template <class... Args>
std::tuple<Args...> unpack(const std::vector<uint8_t> &data) 
{
    std::error_code ec;
    return unpack<Args...>(data.data(), data.size(), ec);
}

template <class... Args>
std::tuple<Args...> unpack(const std::vector<uint8_t> &data, std::error_code &ec) 
{
    return unpack<Args...>(data.data(), data.size(), ec);
}

sol::variadic_args unpack(sol::state_view &state, const uint8_t *dataStart, const std::size_t size, std::error_code &ec);
sol::variadic_args unpack(sol::state_view &state, const uint8_t *dataStart, const std::size_t size);
sol::variadic_args unpack(sol::state_view &state, const std::vector<uint8_t> &data, std::error_code &ec);
sol::variadic_args unpack(sol::state_view &state, const std::vector<uint8_t> &data);

NAMESPACE_END

#define PACKER_BASE gb::msgpack::MsgPackBase

#define REGISTER_PACKER(...)                                   \
public:                                                        \
    void pack(gb::msgpack::Packer &packer)               \
    {                                                          \
        packer(__VA_ARGS__);                                   \
    }                                                          \
    void unpack(gb::msgpack::Unpacker &unpacker)         \
    {                                                          \
        unpacker(__VA_ARGS__);                                 \
    }
