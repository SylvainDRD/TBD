#pragma once

#include <iostream>
#include <limits>
#include <misc/types.hpp>

namespace TBD {

#define TBD_MIN_T(T) std::numeric_limits<T>::min()
#define TBD_MAX_T(T) std::numeric_limits<T>::max()

#define _TBD_ENDLOG " - " << __FILE__ << ":" << __LINE__ << "\033[0m" << std::endl

#define TBD_LOG(msg) std::cout << "[LOG]: " << msg << _TBD_ENDLOG
#define TBD_LOG_VK(msg) std::cout << "[VK LOG]: " << msg << _TBD_ENDLOG
#define TBD_WARN(msg) std::cerr << "\033[93m[WARNING]: " << msg << _TBD_ENDLOG
#define TBD_WARN_VK(msg) std::cerr << "\033[93m[VK WARNING]: " << msg << _TBD_ENDLOG
#define TBD_ERROR(msg) std::cerr << "\033[31m[ERROR]: " << msg << _TBD_ENDLOG
#define TBD_ERROR_VK(msg) std::cerr << "\033[31m[VK ERROR]: " << msg << _TBD_ENDLOG

#define _TBD_FATAL(msg) std::cerr << "\033[31m[FATAL]: " << msg << _TBD_ENDLOG
#define TBD_ABORT(reason)   \
    _TBD_FATAL(reason); \
    std::abort()

#define _TBD_FATAL_VK(msg) std::cerr << "\033[31m[VK FATAL]: " << msg << _TBD_ENDLOG
#define TBD_ABORT_VK(reason)   \
    _TBD_FATAL_VK(reason); \
    std::abort()

#define TBD_NO_COPY(T)    \
    T(const T&) = delete; \
    T& operator=(const T&) = delete;
#define TBD_NO_MOVE(T)     \
    T(const T&&) = delete; \
    T& operator=(const T&&) = delete;
#define TBD_NO_COPY_MOVE(T) TBD_NO_COPY(T) TBD_NO_MOVE(T)

#ifdef PROJECT_DEBUG
#define TBD_DEBUG(msg) std::cout << "\033[96m[DEBUG]: " << msg << _TBD_ENDLOG
#define TBD_DEBUG_VK(msg) std::cout << "\033[96m[VK DEBUG]: " << msg << _TBD_ENDLOG

#define TBD_ASSERT(condition, msg) \
    if (!(condition)) {            \
        TBD_ABORT(msg);                \
    }
#define TBD_ASSERT_VK(condition, msg) \
    if (!(condition)) {               \
        TBD_ABORT_VK(msg);                \
    }
#else
#define _TBD_NOP \
    if (false) { \
    }
#define TBD_DEBUG(msg) _TBD_NOP
#define TBD_DEBUG_VK(msg) _TBD_NOP

#define TBD_ASSERT(condition, msg) _TBD_NOP
#define TBD_ASSERT_VK(condition, msg) _TBD_NOP
#endif

} // namespace TBD
