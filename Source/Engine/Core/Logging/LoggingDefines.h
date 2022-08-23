#pragma once

#include <spdlog/spdlog.h>

#define HE_LOG(level, ...)
#define HE_LOG_VERBOSE(...)    ::HE::LogSystem::gLogger->debug(__VA_ARGS__);
#define HE_LOG_INFO(...)       ::HE::LogSystem::gLogger->info(__VA_ARGS__);
#define HE_LOG_WARNING(...)    ::HE::LogSystem::gLogger->warn(__VA_ARGS__);
#define HE_LOG_ERROR(...)      ::HE::LogSystem::gLogger->error(__VA_ARGS__);
#define HE_LOG_FATAL(...)      ::HE::LogSystem::gLogger->critical(__VA_ARGS__);