#include "Logger.hpp"

namespace openVulkanoCpp
{
	std::vector<spdlog::sink_ptr> Logger::sinks;
	std::shared_ptr<spdlog::logger> Logger::WINDOW = nullptr;
	std::shared_ptr<spdlog::logger> Logger::MANAGER = nullptr;
	std::shared_ptr<spdlog::logger> Logger::RENDER = nullptr;
	std::shared_ptr<spdlog::logger> Logger::PHYSIC = nullptr;
	std::shared_ptr<spdlog::logger> Logger::AUDIO = nullptr;
	std::shared_ptr<spdlog::logger> Logger::DATA = nullptr;
	std::shared_ptr<spdlog::logger> Logger::SCENE = nullptr;
}