#pragma once

#define SPDLOG_DEBUG_ON
#define SPDLOG_TRACE_ON

#include <iostream>
#include <filesystem>
#include <spdlog/spdlog.h>
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/null_sink.h"
#ifndef NO_CONSOLE_LOG
#include <spdlog/sinks/stdout_color_sinks.h>
#endif
#ifdef _MSC_VER
#include "spdlog/sinks/msvc_sink.h"
#endif

namespace openVulkanoCpp
{
	class Logger
	{ //TODO add custom sink for in game/engine console
		static std::vector<spdlog::sink_ptr> sinks;
	public:
		static std::shared_ptr<spdlog::logger> WINDOW;
		static std::shared_ptr<spdlog::logger> MANAGER;
		static std::shared_ptr<spdlog::logger> RENDER;
		static std::shared_ptr<spdlog::logger> PHYSIC;
		static std::shared_ptr<spdlog::logger> AUDIO;
		static std::shared_ptr<spdlog::logger> DATA;
		static std::shared_ptr<spdlog::logger> SCENE;

		static void SetupLogger(std::string logFolder = "logs", std::string logFile = "openVulkano.log")
		{
			static bool initialized = false;
			if (initialized) return;
			try
			{
				try
				{ //TODO allow log files in folders
					sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(logFile, 1024 * 1024 * 512, 3, true));
				}
				catch (const spdlog::spdlog_ex& e)
				{
					std::cerr << "Log create file log sink: " << e.what() << std::endl;
				}
#ifndef NO_CONSOLE_LOG
				sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
#endif
#ifdef _MSC_VER // If it was build with msvc in debug we can use the msvc sink
				sinks.push_back(std::make_shared<spdlog::sinks::msvc_sink_mt>());
#endif
				// Make sure that there is always a sink for the loggers
				if (sinks.empty()) sinks.push_back(std::make_shared<spdlog::sinks::null_sink_mt>());

				MANAGER = CreateLogger("manager");
				WINDOW = CreateLogger("window");
				RENDER = CreateLogger("render");
				PHYSIC = CreateLogger("physic");
				AUDIO = CreateLogger("audio");
				DATA = CreateLogger("data");
				SCENE = CreateLogger("scene");

				spdlog::flush_every(std::chrono::seconds(5));

				MANAGER->info("Logger initialized");
				initialized = true;
			}
			catch (const spdlog::spdlog_ex& e)
			{
				std::cerr << "Log initialization failed: " << e.what() << std::endl;
			}
		}

		/**
		 * \brief Creates a new custom logger that writes to the main log file.
		 * \param name The name of the logger
		 * \param reg If set to true the logger can be accessed again with spdlog::get(name)
		 * \return The created logger
		 */
		static std::shared_ptr<spdlog::logger> CreateLogger(const std::string& name, const bool reg = true)
		{
			auto logger = std::make_shared<spdlog::logger>(name, sinks.begin(), sinks.end());
			if (reg) spdlog::register_logger(logger);
#ifdef LOG_DATE
			logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [T%t] [%^%l%$] [%n]: %v");
#else
			logger->set_pattern("[%H:%M:%S.%e] [T%t] [%^%l%$] [%n]: %v");
#endif
#ifdef DEBUG
			logger->set_level(spdlog::level::debug);
#endif
			return logger;
		}
	};
}