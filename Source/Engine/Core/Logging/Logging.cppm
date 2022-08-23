module;

#include "Core/CoreCommon.h"

HE_DISABLE_WARNINGS
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
HE_ENABLE_WARNINGS

export module HorizonEngine.Core.Logging;

export namespace HE
{
    namespace LogSystem
    {
        extern bool gIsLoggingEnabled;
        extern std::shared_ptr<spdlog::logger> gLogger;
    }

    enum class LogLevel
    {
        Verbose,
        Info,
        Warning,
        Error,
        Fatal,
    };

    extern bool LogSystemInit();

    extern void LogSystemExit();
}