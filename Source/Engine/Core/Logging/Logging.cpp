module;

#include "Core/CoreCommon.h"
#include "Core/Logging/LoggingDefines.h"

HE_DISABLE_WARNINGS
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
HE_ENABLE_WARNINGS

module HorizonEngine.Core.Logging;

namespace HE
{
	namespace LogSystem
	{
		bool gIsLoggingEnabled = false;
		std::shared_ptr<spdlog::logger> gLogger = nullptr;
	}

	bool LogSystemInit()
	{
		std::vector<spdlog::sink_ptr> sinks;
		sinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
		sinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("Horizon.log", true));
		sinks[0]->set_pattern("%^[%T] %n: %v%$");
		sinks[1]->set_pattern("[%T] [%l] %n: %v");
		auto color_sink = static_cast<spdlog::sinks::stdout_color_sink_mt*>(sinks[0].get());
		color_sink->set_color(spdlog::level::info, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		color_sink->set_color(spdlog::level::warn, FOREGROUND_RED | FOREGROUND_GREEN);
		color_sink->set_color(spdlog::level::err, FOREGROUND_RED);
		LogSystem::gLogger = std::make_shared<spdlog::logger>("Console Logger", begin(sinks), end(sinks));
		spdlog::register_logger(LogSystem::gLogger);
		LogSystem::gLogger->set_level(spdlog::level::debug);
		LogSystem::gLogger->flush_on(spdlog::level::debug);
		LogSystem::gIsLoggingEnabled = true;

		HE_LOG_INFO("Log system init.");
		return true;
	}

	void LogSystemExit()
	{
		HE_LOG_INFO("Log system exit.");
	}
}
