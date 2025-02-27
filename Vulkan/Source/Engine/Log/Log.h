#pragma once
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"

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

void InitLog();