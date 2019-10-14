#pragma once
#include <vulkan/vulkan.hpp>
#include "../Base/ICloseable.hpp"

namespace openVulkanoCpp
{
	namespace Vulkan
	{
		struct Pipeline : virtual ICloseable
		{
			vk::Device device;
			vk::DescriptorSetLayout descriptorSetLayout;
			vk::PipelineLayout pipelineLayout;
			vk::DescriptorPool descriptorPool;

			void Init(vk::Device& device)
			{
				this->device = device;

				CreatePipelineLayout();
			}

			void Close() override
			{
				device.destroyPipelineLayout(pipelineLayout);
				device.destroyDescriptorSetLayout(descriptorSetLayout);
			}

		private:
			void CreatePipelineLayout()
			{
				vk::PushConstantRange camPushConstantDesc = { vk::ShaderStageFlagBits::eVertex, 0, 64 };
				vk::DescriptorSetLayoutBinding nodeLayoutBinding = { 0, vk::DescriptorType::eUniformBufferDynamic, 1, vk::ShaderStageFlagBits::eVertex };
				std::array<vk::DescriptorSetLayoutBinding, 1> layoutBindings = { nodeLayoutBinding };
				vk::DescriptorSetLayoutCreateInfo dslci = { {}, layoutBindings.size(), layoutBindings.data() };
				descriptorSetLayout = device.createDescriptorSetLayout(dslci);
				vk::PipelineLayoutCreateInfo plci = { {}, 1, &descriptorSetLayout, 1, &camPushConstantDesc };
				pipelineLayout = this->device.createPipelineLayout(plci);
			}
		};
	}
}
