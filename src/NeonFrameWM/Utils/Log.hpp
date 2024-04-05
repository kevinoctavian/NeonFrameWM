#pragma once

#ifdef _DEBUG
#include <iostream>
#include <sstream>

#define LOG(type, format, ...)                                                                              \
  {                                                                                                         \
    std::ostringstream debugStream;                                                                         \
    debugStream << "[" << type << "]: " << __FUNCTION__ << ":" << __LINE__ << " - " << format << std::endl; \
    fprintf(stderr, debugStream.str().c_str(), ##__VA_ARGS__);                                              \
  }                                                                                                         \
  while (0)                                                                                                 \
    ;

#define DEBUG_LOG(format, ...) \
  LOG("DEBUG", format, ##__VA_ARGS__)

#define ERROR_LOG(format, ...) \
  LOG("ERROR", format, ##__VA_ARGS__)

#else
#define DEBUG_LOG(format, ...) \
  do                           \
  {                            \
  } while (0);

#define ERROR_LOG(format, ...) \
  DEBUG_LOG(format, ##__VA_ARGS__)

#endif