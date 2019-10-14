#pragma once
#include <vulkan/vulkan.hpp>
#include "../Base/ICloseable.hpp"

namespace openVulkanoCpp
{
	namespace Vulkan
	{
		struct CommandHelper : virtual ICloseable
		{
			vk::Device device;
			vk::CommandPool cmdPool;
			vk::CommandBuffer cmdBuffer;
			vk::CommandBufferLevel level;

			CommandHelper() = default;
			~CommandHelper() { if (cmdPool) CommandHelper::Close(); }

			void Init(vk::Device device, uint32_t queueIndex, vk::CommandBufferLevel level = vk::CommandBufferLevel::eSecondary)
			{
				this->level = level;
				this->device = device;
				cmdPool = device.createCommandPool(vk::CommandPoolCreateInfo({}, queueIndex));
				vk::CommandBufferAllocateInfo bufferAllocInfo = { cmdPool, level, 1 };
				cmdBuffer = device.allocateCommandBuffers(bufferAllocInfo)[0];
			}

			void Reset() const
			{
				device.resetCommandPool(cmdPool, {});
			}
			
			vk::CommandBufferLevel GetLevel() const
			{
				return level;
			}

			void Close() override
			{
				device.freeCommandBuffers(cmdPool, 1, &cmdBuffer);
				device.destroyCommandPool(cmdPool);
			}
		};
	}
}
