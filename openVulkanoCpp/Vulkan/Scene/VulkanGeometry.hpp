#pragma once
#include "IRecordable.hpp"
#include "../../Scene/Scene.hpp"

namespace openVulkanoCpp
{
	namespace Vulkan
	{
		class VulkanGeometry : virtual public IRecordable, virtual public ICloseable
		{
			Scene::Geometry* geometry = nullptr;
			vk::Buffer vertexBuffer, indexBuffer;
			vk::IndexType indexType;
			vk::DeviceSize* offsets = new vk::DeviceSize();

		public:
			VulkanGeometry() = default;
			virtual ~VulkanGeometry() { if (vertexBuffer) VulkanGeometry::Close(); };

			void Init(Scene::Geometry* geo, vk::Buffer vertexBuffer, vk::Buffer indexBuffer)
			{
				this->geometry = geo;
				offsets[0] = 0;
				indexType = (geo->indexType == Scene::VertexIndexType::UINT16) ? vk::IndexType::eUint16 : vk::IndexType::eUint32;
				this->vertexBuffer = vertexBuffer;
				this->indexBuffer = indexBuffer;
			}

			void Record(vk::CommandBuffer& cmdBuffer, uint32_t bufferId) override
			{
				cmdBuffer.bindVertexBuffers(0, 1, &vertexBuffer, offsets);
				cmdBuffer.bindIndexBuffer(indexBuffer, 0, indexType);
			}

			void Close() override
			{
				
			}
		};
	}
}
