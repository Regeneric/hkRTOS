#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <core/logger.h>

static const char* const logLevelStrings[6] = {
    "[FATAL]: ",
    "[ERROR]: ",
    "[WARN]:  ",
    "[INFO]:  ",
    "[DEBUG]: ",
    "[TRACE]: "
}; 

void hkLogOutput(LogLevel_t level, const char* message, ...) {
    if(message == NULL) return;
    if((i16)level < 0 || (i16)level >= MAX_LOG_LEVEL) level = LOG_LEVEL_INFO;

    b8 isError = level < LOG_LEVEL_WARN;
    const char* currentLogLevel = logLevelStrings[level];

    char logBuffer[HK_LOG_MSG_MAX_LEN];
    memset(logBuffer, 0, sizeof(logBuffer));

    va_list argPointer;
    va_start(argPointer, message);
    i32 written = vsnprintf(logBuffer, HK_LOG_MSG_MAX_LEN, message, argPointer);
    va_end(argPointer);
    
    if(written < 0) return;
    if(written >= HK_LOG_MSG_MAX_LEN) {}    // Do nothing, for now

    char logMessage[HK_LOG_MSG_MAX_LEN];
    snprintf(logMessage, sizeof(logMessage), "%s%s\n", currentLogLevel, logBuffer);
    
    HPRINT(logMessage);                 // SEGGER print shorthand macro
    if(isError) printf(logMessage);     // Go to stdout, for now
    else printf(logMessage);
    
    return;
}