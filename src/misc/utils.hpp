#pragma once

#include <misc/types.hpp>
#include <iostream>

namespace TBD {

#define _TBD_ENDLOG " - " << __FILE__ << ":" << __LINE__ << "\033[0m" << std::endl

#define TBD_LOG(msg) std::cout << "[LOG]:" << msg << _TBD_ENDLOG
#define TBD_WARN(msg) std::cerr << "\033[93m[WARNING]:" << msg << _TBD_ENDLOG
#define TBD_ERROR(msg) std::cerr << "\033[31m[ERROR]:" << msg << _TBD_ENDLOG

#ifdef PROJECT_DEBUG
#define TBD_DEBUG(msg) std::cout << "\033[96m[DEBUG]:" << msg << _TBD_ENDLOG
#else
#define TBD_DEBUG(msg)                                                         \
  if (false) {                                                                 \
  }
#endif

#define _TBD_FATAL(msg) std::cerr << "\033[31m[FATAL]:" << msg << _TBD_ENDLOG
#define ABORT(reason)                                                          \
  _TBD_FATAL(reason);                                                           \
  std::abort()

#define TBD_NO_COPY(T)                                                         \
  T(const T &) = delete;                                                       \
  T &operator=(const T &) = delete;
#define TBD_NO_MOVE(T)                                                         \
  T(const T &&) = delete;                                                      \
  T &operator=(const T &&) = delete;
#define TBD_NO_COPY_MOVE(T) TBD_NO_COPY(T) TBD_NO_MOVE(T)

} // namespace TBD
