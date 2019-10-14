#pragma once
#include "../../Base/ICloseable.hpp"
#include "../Scene/IRecordable.hpp"
#include "ManagedResource.hpp"

namespace openVulkanoCpp
{
	namespace Vulkan
	{
		struct UniformBuffer : virtual ICloseable, virtual IRecordable
		{
			ManagedBuffer* buffer;
			vk::DescriptorPool descPool;
			vk::DescriptorSet descSet;
			vk::PipelineLayout layout;
			uint32_t allocSizeFrame;

			void Init(ManagedBuffer* buffer, uint32_t allocSizeFrame, vk::DescriptorSetLayout* descriptorSetLayout, vk::PipelineLayout layout)
			{
				this->buffer = buffer;
				this->layout = layout;
				this->allocSizeFrame = allocSizeFrame;
				vk::DescriptorPoolSize poolSize = { vk::DescriptorType::eUniformBufferDynamic, 1 };
				const vk::DescriptorPoolCreateInfo poolCreateInfo = { {}, 1, 1, &poolSize };
				descPool = buffer->device.createDescriptorPool(poolCreateInfo);
				const vk::DescriptorSetAllocateInfo descSetAllocInfo = { descPool, 1, descriptorSetLayout };
				descSet = buffer->device.allocateDescriptorSets(descSetAllocInfo)[0];
				vk::DescriptorBufferInfo bufferInfo = { buffer->buffer, 0, allocSizeFrame };
				vk::WriteDescriptorSet writeDescriptorSet = { descSet };
				writeDescriptorSet.descriptorCount = 1;
				writeDescriptorSet.descriptorType = vk::DescriptorType::eUniformBufferDynamic;
				writeDescriptorSet.pBufferInfo = &bufferInfo;
				buffer->device.updateDescriptorSets(1, &writeDescriptorSet, 0, nullptr);
			}

			void Record(vk::CommandBuffer& cmdBuffer, uint32_t bufferId) override
			{
				uint32_t frameOffset = allocSizeFrame * bufferId;
				cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, layout, 0, 1,
					&descSet, 1, &frameOffset);
			}

			void Update(void* data, uint32_t size, uint32_t bufferId) const
			{
				buffer->Copy(data, size, allocSizeFrame * bufferId);
			}

			void Close() override
			{
				buffer->device.destroyDescriptorPool(descPool);
			}
		};
	}
}
