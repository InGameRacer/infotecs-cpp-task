#ifndef DYNAMIC_LIB_H
#define DYNAMIC_LIB_H

#include <string>

#ifdef _WIN32
  #ifdef BUILDING_DYNAMIC_LIB
    #define LIB_API __declspec(dllexport)
  #else
    #define LIB_API __declspec(dllimport)
  #endif
#else
  #define LIB_API __attribute__((visibility("default")))
#endif

LIB_API void function1(std::string& str);
LIB_API int function2(const std::string& str);
LIB_API bool function3(const std::string& str);

#endif // DYNAMIC_LIB_H
