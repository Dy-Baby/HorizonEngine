#pragma once

#include <spdlog/spdlog.h>

#ifndef HE_LOG
#define HE_LOG(level, ...)
#endif

#ifndef HE_LOG_VERBOSE
#define HE_LOG_VERBOSE(...)    ::HE::LogVerbose(::HE::LogSystem::gLogger, __VA_ARGS__);
#endif

#ifndef HE_LOG_INFO
#define HE_LOG_INFO(...)       ::HE::LogInfo(::HE::LogSystem::gLogger, __VA_ARGS__);
#endif

#ifndef HE_LOG_WARNING
#define HE_LOG_WARNING(...)    ::HE::LogWarning(::HE::LogSystem::gLogger, __VA_ARGS__);
#endif

#ifndef HE_LOG_ERROR
#define HE_LOG_ERROR(...)      ::HE::LogError(::HE::LogSystem::gLogger, __VA_ARGS__);
#endif

#ifndef HE_LOG_FATAL
#define HE_LOG_FATAL(...)      ::HE::LogFatal(::HE::LogSystem::gLogger, __VA_ARGS__);
#endif