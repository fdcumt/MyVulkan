#include "Log.h"

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/stdout_sinks-inl.h"


void InitLog()
{
    // 创建一个日志器，同时包含控制台和文件输出
    //auto console_sink = spdlog::stdout_logger_mt("console");
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    //console_sink->set_level(spdlog::level::trace);  // 设置文件日志等级
    //console_sink->set_pattern("[%T] [%l] %v");      // 设置日志格式
    
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/my_log.txt", true);
    //file_sink->set_level(spdlog::level::trace);  // 设置文件日志等级
   // file_sink->set_pattern("[%T] [%l] %v");      // 设置日志格式

    std::initializer_list<spdlog::sink_ptr> InitList = {console_sink, file_sink};
    
    // 创建一个日志器，并添加多个输出目标
    auto logger = std::make_shared<spdlog::logger>("multi_sink_logger", InitList);
    
    
    //logger->set_pattern("[%H:%M:%S] [%n] [%L] %v");
    
    // 设置默认日志器
    spdlog::set_default_logger(logger);
    spdlog::set_level(spdlog::level::trace); // Set global log level to debug

    // 设置日志格式
    //logger->set_pattern("[%H:%M:%S-%L] %v");
    //spdlog::set_pattern("[source %s] [function %!] [line %#] %v");
    spdlog::set_pattern("[%T] %^[%l] %v%$");
    
    // 确保日志刷新到文件
    spdlog::flush_every(std::chrono::seconds(1));
#if 0
    DebugLog("---------Test log begin------------");
    InfoLog("This is InfoLog. %d %f", 123, 0.123);
    DebugLog("This is DebugLog. %d %f", 123, 0.123);
    WarningLog("This is WarningLog. %d %f", 123, 0.123);
    ErrorLog("This is ErrorLog. %d %f", 123, 0.123);
    CriticalLog("This is CriticalLog. %d %f", 123, 0.123);

    spdlog::info("Support for floats {:03.2f}", 1.23456);
    spdlog::info("Positional args are {1} {0}..", "too", "supported");
    spdlog::info("{:<30}", "left aligned");
    spdlog::debug("{:<30}", "left aligned");
    spdlog::warn("Easy padding in numbers like {:08d}", 12);
    spdlog::error("Easy padding in numbers like {:08d}", 12);
    spdlog::critical("Support for int: {0:d};  hex: {0:x};  oct: {0:o}; bin: {0:b}", 42);
    
    DebugLog("---------Test log End------------");
#endif
}
