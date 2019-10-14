#pragma once
#include <string>
#include <spdlog/fmt/fmt.h> //TODO replace with external fmt

namespace  openVulkanoCpp
{
#define MAKE_VERSION(major, minor, patch)  (((major) << 22) | ((minor) << 12) | (patch))

	const char* ENGINE_NAME = "openVulkanoCpp";

	struct EngineVersion
	{
		int major, minor, patch;
		int intVersion;
		std::string stringVersion;

		EngineVersion(int major, int minor, int patch, int build = 0) : major(major), minor(minor), patch(patch)
		{
			intVersion = ((major) << 24) | ((minor) << 16) | (patch);
			std::string buildConfig = "";
#ifdef _DEBUG
			buildConfig += "-MSVC_DEBUG";
#elif DEBUG
			buildConfig += "-DEBUG";
#endif
			stringVersion = fmt::format("v{0}.{1}.{2}.{3}{4}", major, minor, patch, build, buildConfig);
		}
	};

	const EngineVersion ENGINE_VERSION(0, 0, 1);
}
