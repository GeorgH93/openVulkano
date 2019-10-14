#pragma once
#include <vulkan/vulkan.hpp>
#include "Buffer.hpp"
#include "VulkanUtils.hpp"

namespace openVulkanoCpp
{
	namespace Vulkan
	{
		class IImage
		{
		public:
			virtual ~IImage() = default;

			virtual vk::Image GetImage() = 0;
			virtual vk::ImageView GetView() = 0;
		};

		struct Image : public Buffer, virtual public IImage
		{
			vk::Image image;
			vk::Extent3D extent;
			vk::ImageView view;
			vk::Sampler sampler;
			vk::Format format = vk::Format::eUndefined;
			
			/**
			 * \brief 
			 * \param device 
			 * \param imageCreateInfo 
			 * \param imageViewCreateInfo The image will be set automatically after it's creation
			 * \param memoryPropertyFlags 
			 */
			void Init(const Device* device, const vk::ImageCreateInfo& imageCreateInfo, vk::ImageViewCreateInfo imageViewCreateInfo, const vk::MemoryPropertyFlags& memoryPropertyFlags = vk::MemoryPropertyFlagBits::eDeviceLocal)
			{
				this->device = device->device;
				image = device->device.createImage(imageCreateInfo);
				format = imageCreateInfo.format;
				extent = imageCreateInfo.extent;
				
				const vk::MemoryRequirements memRequirements = device->device.getImageMemoryRequirements(image);
				size = allocSize = memRequirements.size;
				const vk::MemoryAllocateInfo memAllocInfo = { allocSize, device->GetMemoryType(memRequirements.memoryTypeBits, memoryPropertyFlags) };
				memory = device->device.allocateMemory(memAllocInfo);
				device->device.bindImageMemory(image, memory, 0);
				
				imageViewCreateInfo.image = image;
				view = device->device.createImageView(imageViewCreateInfo);
			}

			void SetLayout(vk::CommandBuffer& cmdBuffer, vk::ImageSubresourceRange subResourceRange, vk::ImageLayout newLayout, vk::ImageLayout oldLayout = vk::ImageLayout::eUndefined) const
			{
				const vk::ImageMemoryBarrier imgMemBarrier(VulkanUtils::GetAccessFlagsForLayout(oldLayout), VulkanUtils::GetAccessFlagsForLayout(newLayout), oldLayout, 
					newLayout, 0, 0, image, subResourceRange);
				cmdBuffer.pipelineBarrier(VulkanUtils::GetPipelineStageForLayout(oldLayout), VulkanUtils::GetPipelineStageForLayout(newLayout),
					{}, nullptr, nullptr, imgMemBarrier);
			}

			void SetLayout(vk::CommandBuffer& cmdBuffer, vk::ImageAspectFlags aspectMask, vk::ImageLayout newLayout, vk::ImageLayout oldLayout = vk::ImageLayout::eUndefined) const
			{
				SetLayout(cmdBuffer, vk::ImageSubresourceRange(aspectMask, 0, 1, 0, 1), newLayout, oldLayout);
			}

			void Close() override
			{
				if(sampler)
				{
					device.destroySampler(sampler);
					sampler = vk::Sampler();
				}
				if(view)
				{
					device.destroyImageView(view);
					view = vk::ImageView();
				}
				if(image)
				{
					device.destroyImage(image);
					image = vk::Image();
				}
				Buffer::Close();
			}

			operator bool() const { return image.operator bool(); }


			vk::Image GetImage() override
			{
				return image;
			}

			vk::ImageView GetView() override
			{
				return view;
			}
		};
	}
}
