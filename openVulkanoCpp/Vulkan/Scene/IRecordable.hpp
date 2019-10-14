#pragma once
#include "vulkan/vulkan.hpp"

namespace openVulkanoCpp
{
	namespace Vulkan
	{
		class IRecordable
		{
		public:
			virtual ~IRecordable() = default;
			virtual void Record(vk::CommandBuffer& cmdBuffer, uint32_t bufferId) = 0;
		};
	}
}