#pragma once
#include "../Scene/VulkanShader.hpp"

namespace openVulkanoCpp
{
	namespace Vulkan
	{
		struct VulkanShader;

		class IShaderOwner
		{
		public:
			virtual void RemoveShader(VulkanShader* shader) = 0;
		};
	}
}
