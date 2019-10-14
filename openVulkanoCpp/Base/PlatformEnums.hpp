#pragma once

namespace openVulkanoCpp
{
	namespace RenderAPI
	{
		enum RenderApi
		{
			VULKAN = 0,
			//OpenGL,
			//DirectX11,
			//DirectX12,
			MAX_VALUE
		};

		inline std::string ToString(RenderApi api)
		{
			switch (api)
			{
			case VULKAN: return "Vulkan";
			}
			return "Invalid";
		}
	}

	namespace Platform
	{
		enum Platform
		{
			Windows = 0, MacOS, Linux, Android, MAX_VALUE
		};

		inline std::string ToString(Platform os)
		{
			switch (os)
			{
			case Windows: return "Windows";
			case MacOS: return "Windows";
			case Linux: return "Windows";
			case Android: return "Windows";
			}
			return "Invalid";
		}
	}
}