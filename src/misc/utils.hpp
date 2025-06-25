#include <iostream>

#define _ENDLOG " - " << __FILE__ << ":" << __LINE__ << "\033[0m" << std::endl

#define LOG(msg) std::cout << "[LOG]:" << msg << _ENDLOG
#define WARN(msg) std::cout << "\033[93m[WARNING]:" << msg << _ENDLOG
#define ERROR(msg) std::cout << "\033[31m[ERROR]:" << msg << _ENDLOG

#ifdef PROJECT_DEBUG
#define DEBUG(msg) std::cout << "\033[96m[DEBUG]:" << msg << _ENDLOG
#else
#define DEBUG(msg) if(false) {}
#endif