#pragma once
#include <defines.h>

#define HK_LOG_MSG_MAX_LEN 80

#define LOG_FATAL 0
#define LOG_ERROR 1
#define LOG_WARN  2
#define LOG_INFO  3
#define LOG_DEBUG 4
#define LOG_TRACE 5

#ifndef HK_LOG_LEVEL
  #define HK_LOG_LEVEL LOG_INFO
#endif


#if HK_LOG_LEVEL >= LOG_WARN
    #define LOG_WARN_ENABLED true
#else
    #define LOG_WARN_ENABLED false
#endif

#if HK_LOG_LEVEL >= LOG_INFO
    #define LOG_INFO_ENABLED true
#else
    #define LOG_INFO_ENABLEd false
#endif

#if HK_LOG_LEVEL >= LOG_DEBUG
    #define LOG_DEBUG_ENABLED true
#else
    #define LOG_DEBUG_ENABLED false
#endif

#if HK_LOG_LEVEL >= LOG_TRACE
    #define LOG_TRACE_ENABLED true
#else
    #define LOG_TRACE_ENABLED false
#endif

#if HRELEASE
    #define LOG_DEBUG_ENABLED false
    #define LOG_TRACE_ENABLED false
#endif

typedef enum LogLevel_t {
    LOG_LEVEL_FATAL = LOG_FATAL,
    LOG_LEVEL_ERROR = LOG_ERROR,
    LOG_LEVEL_WARN  = LOG_WARN,
    LOG_LEVEL_INFO  = LOG_INFO,
    LOG_LEVEL_DEBUG = LOG_DEBUG,
    LOG_LEVEL_TRACE = LOG_TRACE,

    MAX_LOG_LEVEL
} LogLevel_t;

void hkLogOutput(LogLevel_t level, const char* message, ...);


#ifndef HFATAL
    #define HFATAL(message, ...) hkLogOutput(LOG_LEVEL_FATAL, message, ##__VA_ARGS__)
#endif
#ifndef HERROR
    #define HERROR(message, ...) hkLogOutput(LOG_LEVEL_ERROR, message, ##__VA_ARGS__)
#endif

#if LOG_WARN_ENABLED
    #define HWARN(message, ...)  hkLogOutput(LOG_LEVEL_WARN, message, ##__VA_ARGS__)
#else
    #define HWARN(message, ...)
#endif

#if LOG_INFO_ENABLED
    #define HINFO(message, ...)  hkLogOutput(LOG_LEVEL_INFO, message, ##__VA_ARGS__)
#else
    #define HINFO(message, ...)
#endif

#if LOG_DEBUG_ENABLED
    #define HDEBUG(message, ...) hkLogOutput(LOG_LEVEL_DEBUG, message, ##__VA_ARGS__)
#else
    #define HDEBUG(message, ...)
#endif

#if LOG_TRACE_ENABLED
    #define HTRACE(message, ...) hkLogOutput(LOG_LEVEL_TRACE, message, ##__VA_ARGS__)
#else
    #define HTRACE(message, ...)
#endif