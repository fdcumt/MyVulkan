#include "Log.h"


void InitLog()
{
    // 创建文件接收器
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/logfile.log", true);
    file_sink->set_level(spdlog::level::debug);  // 设置文件日志等级
    file_sink->set_pattern("[%T] [%l] %v");      // 设置日志格式

    // 创建日志器
    auto logger = std::make_shared<spdlog::logger>("file_logger", file_sink);
    logger->set_level(spdlog::level::debug);     // 设置全局日志等级

    spdlog::set_level(spdlog::level::debug); // Set global log level to debug
    
    // 设置默认日志器
    spdlog::set_default_logger(logger);
    
    // 确保日志刷新到文件
    spdlog::flush_every(std::chrono::seconds(1));
}
