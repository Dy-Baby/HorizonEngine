module;

#include "Core/CoreCommon.h"
#include <spdlog/spdlog.h>

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