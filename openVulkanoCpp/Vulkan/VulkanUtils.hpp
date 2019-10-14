#pragma once
#include <vulkan/vulkan.hpp>

namespace openVulkanoCpp
{
	namespace Vulkan
	{
		class VulkanUtils
		{
		public:
			static vk::AccessFlags GetAccessFlagsForLayout(vk::ImageLayout layout)
			{
				switch (layout)
				{
				case vk::ImageLayout::ePreinitialized: return vk::AccessFlagBits::eHostWrite;
				case vk::ImageLayout::eTransferSrcOptimal: return vk::AccessFlagBits::eTransferRead;
				case vk::ImageLayout::eTransferDstOptimal: return vk::AccessFlagBits::eTransferWrite;
				case vk::ImageLayout::eShaderReadOnlyOptimal: return vk::AccessFlagBits::eShaderRead;
				case vk::ImageLayout::eColorAttachmentOptimal: return vk::AccessFlagBits::eColorAttachmentWrite;
				case vk::ImageLayout::eDepthStencilAttachmentOptimal: return vk::AccessFlagBits::eDepthStencilAttachmentWrite;
				default: return vk::AccessFlags();
				}
			}

			static vk::PipelineStageFlags GetPipelineStageForLayout(vk::ImageLayout layout)
			{
				switch (layout)
				{
				case vk::ImageLayout::ePreinitialized: return vk::PipelineStageFlagBits::eHost;
				case vk::ImageLayout::eTransferDstOptimal:
				case vk::ImageLayout::eTransferSrcOptimal: return vk::PipelineStageFlagBits::eTransfer;
				case vk::ImageLayout::eShaderReadOnlyOptimal: return vk::PipelineStageFlagBits::eFragmentShader;
				case vk::ImageLayout::eColorAttachmentOptimal: return vk::PipelineStageFlagBits::eColorAttachmentOutput;
				case vk::ImageLayout::eDepthStencilAttachmentOptimal: return vk::PipelineStageFlagBits::eEarlyFragmentTests;
				case vk::ImageLayout::eUndefined: return vk::PipelineStageFlagBits::eTopOfPipe;
				default: return vk::PipelineStageFlagBits::eBottomOfPipe;
				}
			}
		};
	}
}
