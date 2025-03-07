#pragma once
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"

static bool EnableFollowLog = true;

#define VKFollowLogImpl(fmt, ...) if (EnableFollowLog){ \
static char buffer[1024]; \
int len = snprintf(buffer, sizeof(buffer), fmt, __VA_ARGS__); \
if (len < sizeof(buffer)) { \
spdlog::warn(buffer); \
} else { \
char* buf = new char[len + 1]; \
snprintf(buf, len + 1, fmt, __VA_ARGS__); \
spdlog::warn(buf); \
delete[] buf; \
}}

#define VKFollowLog(fmt, ...) VKFollowLogImpl("[VKFollow]"##fmt, ##__VA_ARGS__) 

#define InfoLog(fmt, ...) { \
    static char buffer[1024]; \
    int len = snprintf(buffer, sizeof(buffer), "[VKFollow]"##fmt, __VA_ARGS__); \
    if (len < sizeof(buffer)) { \
        spdlog::info(buffer); \
    } else { \
        char* buf = new char[len + 1]; \
        snprintf(buf, len + 1, "[VKFollow]"##fmt, __VA_ARGS__); \
        spdlog::info(buf); \
        delete[] buf; \
    }}

#define DebugLog(fmt, ...) { \
    static char buffer[1024]; \
    int len = snprintf(buffer, sizeof(buffer), fmt, __VA_ARGS__); \
    if (len < sizeof(buffer)) { \
        spdlog::debug(buffer); \
    } else { \
        char* buf = new char[len + 1]; \
        snprintf(buf, len + 1, fmt, __VA_ARGS__); \
        spdlog::debug(buf); \
        delete[] buf; \
    }}

#define WarningLog(fmt, ...) { \
     static char buffer[1024]; \
     int len = snprintf(buffer, sizeof(buffer), fmt, __VA_ARGS__); \
     if (len < sizeof(buffer)) { \
        spdlog::warn(buffer); \
    } else { \
        char* buf = new char[len + 1]; \
        snprintf(buf, len + 1, fmt, __VA_ARGS__); \
        spdlog::warn(buf); \
        delete[] buf; \
    }}

#define ErrorLog(fmt, ...) { \
    static char buffer[1024]; \
    int len = snprintf(buffer, sizeof(buffer), fmt, __VA_ARGS__); \
    if (len < sizeof(buffer)) { \
        spdlog::error(buffer); \
    } else { \
        char* buf = new char[len + 1]; \
        snprintf(buf, len + 1, fmt, __VA_ARGS__); \
        spdlog::error(buf); \
        delete[] buf; \
    }}

#define CriticalLog(fmt, ...) { \
    static char buffer[1024]; \
    int len = snprintf(buffer, sizeof(buffer), fmt, __VA_ARGS__); \
    if (len < sizeof(buffer)) { \
        spdlog::critical(buffer); \
    } else { \
        char* buf = new char[len + 1]; \
        snprintf(buf, len + 1, fmt, __VA_ARGS__); \
        spdlog::critical(buf); \
        delete[] buf; \
    }}

void InitLog();