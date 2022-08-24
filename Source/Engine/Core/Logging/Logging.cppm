module;

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

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

    template<typename... Args>
    void LogVerbose(const std::shared_ptr<spdlog::logger>& logger, Args&&... args)
    {
        logger->debug(std::forward<Args>(args)...);
    }

    template<typename... Args>
    void LogInfo(const std::shared_ptr<spdlog::logger>& logger, Args&&... args)
    {
        logger->info(std::forward<Args>(args)...);
    }

    template<typename... Args>
    void LogWarning(const std::shared_ptr<spdlog::logger>& logger, Args&&... args)
    {
        logger->warn(std::forward<Args>(args)...);
    }

    template<typename... Args>
    void LogError(const std::shared_ptr<spdlog::logger>& logger, Args&&... args)
    {
        logger->error(std::forward<Args>(args)...);
    }

    template<typename... Args>
    void LogFatal(const std::shared_ptr<spdlog::logger>& logger, Args&&... args)
    {
        logger->critical(std::forward<Args>(args)...);
    }
}