#pragma once

#include "Core/CoreCommon.h"

HE_DISABLE_WARNINGS
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
HE_ENABLE_WARNINGS

#define HE_LOG(level, ...)    
#define HE_LOG_VERBOSE(...)    ::HE::LogSystem::gLogger->debug(__VA_ARGS__);
#define HE_LOG_INFO(...)       ::HE::LogSystem::gLogger->info(__VA_ARGS__);
#define HE_LOG_WARNING(...)    ::HE::LogSystem::gLogger->warn(__VA_ARGS__);
#define HE_LOG_ERROR(...)      ::HE::LogSystem::gLogger->error(__VA_ARGS__);
#define HE_LOG_FATAL(...)      ::HE::LogSystem::gLogger->critical(__VA_ARGS__);

enum class LogLevel
{
    Verbose,
    Info,
    Warning,
    Error,
    Fatal,
};
namespace HE
{
namespace LogSystem
{
	extern bool gIsLoggingEnabled;
	extern std::shared_ptr<spdlog::logger> gLogger;
}

extern bool LogSystemInit();
extern void LogSystemExit();

}