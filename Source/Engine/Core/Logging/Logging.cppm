module;

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

export module HorizonEngine.Core.Logging;

export import "LoggingDefinitions.h";

namespace HE::LogSystem
{
   bool gIsLoggingEnabled;
    std::shared_ptr<spdlog::logger> gLogger;
}

export namespace HE
{
    enum class LogLevel
    {
        Verbose,
        Info,
        Warning,
        Error,
        Fatal,
    };

    bool LogSystemInit();

    void LogSystemExit();

    template<typename... Args>
    void LogVerbose(Args&&... args)
    {
        LogSystem::gLogger->debug(std::forward<Args>(args)...);
    }

    template<typename... Args>
    void LogInfo(Args&&... args)
    {
        LogSystem::gLogger->info(std::forward<Args>(args)...);
    }

    template<typename... Args>
    void LogWarning(Args&&... args)
    {
        LogSystem::gLogger->warn(std::forward<Args>(args)...);
    }

    template<typename... Args>
    void LogError(Args&&... args)
    {
        LogSystem::gLogger->error(std::forward<Args>(args)...);
    }

    template<typename... Args>
    void LogFatal(Args&&... args)
    {
        LogSystem::gLogger->critical(std::forward<Args>(args)...);
    }
}