#pragma once
#include "../../Base/ICloseable.hpp"
#include "IRecordable.hpp"
#include "../../Scene/Camera.hpp"
#include "../Resources/UniformBuffer.hpp"

namespace openVulkanoCpp
{
	namespace Vulkan
	{
		struct VulkanNode : virtual IRecordable, virtual ICloseable
		{
			Scene::Node* node = nullptr;
			UniformBuffer* buffer = nullptr;

			virtual void Init(Scene::Node* node, UniformBuffer* uniformBuffer)
			{
				this->node = node;
				this->buffer = uniformBuffer;
			}

			void Record(vk::CommandBuffer& cmdBuffer, uint32_t bufferId) override
			{
				buffer->Record(cmdBuffer, 0);
			}
			
			void Close() override {}
		};

		struct VulkanNodeDynamic : VulkanNode
		{
			uint32_t lastUpdate = -1;

			void Init(Scene::Node* node, UniformBuffer* uniformBuffer) override
			{
				VulkanNode::Init(node, uniformBuffer);
				lastUpdate = -1;
			}

			void Record(vk::CommandBuffer& cmdBuffer, uint32_t bufferId) override
			{
				if(bufferId != lastUpdate)
				{
					lastUpdate = bufferId;
					buffer->Update(&node->worldMat, sizeof(glm::mat4x4), bufferId);
				}
				buffer->Record(cmdBuffer, bufferId);
			}

			void Close() override{}
		};
	}
}
