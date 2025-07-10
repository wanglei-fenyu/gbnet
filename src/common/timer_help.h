#pragma once 
#include <gbnet/common/define.h>
#include <chrono>

#if USE_STANDALONE_ASIO

#	define CHRONO_SECOND(std_duration) std::chrono::duration_cast<std::chrono::seconds>(std_duration)
#   define CHRONO_MICROSECONDS(std_duration) std::chrono::duration_cast<std::chrono::microseconds>(std_duration)

#else
#	define CHRONO_SECOND(std_duration) boost::posix_time::seconds(std::chrono::duration_cast<std::chrono::seconds>(std_duration).count())
#   define CHRONO_MICROSECONDS(std_duration) boost::posix_time::microseconds(std::chrono::duration_cast<std::chrono::microseconds>(std_duration).count())

#endif





