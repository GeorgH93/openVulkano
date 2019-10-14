#pragma once
#include "vulkan/vulkan.hpp"
#include "../Device.hpp"
#include "../../Base/ICloseable.hpp"
#include "../Scene/VulkanShader.hpp"
#include "IShaderOwner.hpp"
#include "../Scene/VulkanGeometry.hpp"
#include "ManagedResource.hpp"
#include "../Scene/VulkanNode.hpp"

namespace openVulkanoCpp
{
	namespace Vulkan
	{
		class ResourceManager : virtual public ICloseable, virtual public IShaderOwner
		{
			Context* context;
			vk::Device device = nullptr;
			vk::Queue transferQueue = nullptr;
			vk::CommandPool* cmdPools = nullptr;
			vk::CommandBuffer* cmdBuffers = nullptr;
			vk::Semaphore* semaphores = nullptr;
			std::vector<MemoryAllocation*> allocations;
			std::vector<VulkanShader*> shaders;
			MemoryAllocation* lastAllocation = nullptr;
			std::mutex mutex;
			vk::DeviceSize uniformBufferAlignment;
			std::vector<std::vector<ManagedBuffer*>> toFree;
			std::vector<ManagedBuffer*> recycleBuffers;

			int buffers = -1, currentBuffer = -1;

		public:
			ResourceManager() = default;
			virtual ~ResourceManager() { if (device) ResourceManager::Close(); }

			void Init(Context* context, int buffers = 2)
			{
				this->context = context;
				this->device = context->device->device;
				this->buffers = buffers;

				uniformBufferAlignment = context->device->properties.limits.minUniformBufferOffsetAlignment;

				cmdPools = new vk::CommandPool[buffers];
				cmdBuffers = new vk::CommandBuffer[buffers];
				semaphores = new vk::Semaphore[buffers];
				for (int i = 0; i < buffers; i++)
				{
					cmdPools[i] = this->device.createCommandPool({ {}, context->device->queueIndices.transfer });
					cmdBuffers[i] = this->device.allocateCommandBuffers({ cmdPools[i], vk::CommandBufferLevel::ePrimary, 1 })[0];
					semaphores[i] = this->device.createSemaphore({});
				}
				toFree.resize(buffers);

				transferQueue = this->device.getQueue(context->device->queueIndices.transfer, 0);
			}

			void Close() override
			{
				transferQueue.waitIdle();
				for (int i = 0; i < buffers; i++)
				{
					device.freeCommandBuffers(cmdPools[i], 1, &cmdBuffers[i]);
					device.destroyCommandPool(cmdPools[0]);
				}
				for (auto shader : shaders)
				{
					shader->Close();
				}
				cmdBuffers = nullptr;
				cmdPools = nullptr;
				device = nullptr;
			}

			void StartFrame(uint64_t frameId)
			{
				currentBuffer = frameId;
				FreeBuffers();
				device.resetCommandPool(cmdPools[currentBuffer], {});
				cmdBuffers[currentBuffer].begin({ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
			}

			vk::Semaphore EndFrame()
			{
				cmdBuffers[currentBuffer].end();
				vk::SubmitInfo si = { 0, nullptr, nullptr, 1, &cmdBuffers[currentBuffer], 1, &semaphores[currentBuffer] };
				transferQueue.submit(1, &si, vk::Fence());
				return semaphores[currentBuffer];
			}

			void Resize()
			{
				for (auto shader : shaders)
				{
					Scene::Shader* s = shader->shader;
					shader->Close();
					shader->Init(context, s, this);
				}
			}

			void PrepareGeometry(Scene::Geometry* geometry)
			{
				mutex.lock();
				if(!geometry->renderGeo)
				{					
					VulkanGeometry* vkGeometry = new VulkanGeometry();
					ManagedBuffer* vertexBuffer = CreateDeviceOnlyBufferWithData(sizeof(Vertex) * geometry->GetVertexCount(), vk::BufferUsageFlagBits::eVertexBuffer, geometry->GetVertices());
					ManagedBuffer* indexBuffer = CreateDeviceOnlyBufferWithData(Utils::EnumAsInt(geometry->indexType) * geometry->GetIndexCount(), vk::BufferUsageFlagBits::eIndexBuffer, geometry->GetIndices());
					vkGeometry->Init(geometry, vertexBuffer->buffer, indexBuffer->buffer);
					geometry->renderGeo = vkGeometry;
				}
				mutex.unlock();
			}

			void PrepareMaterial(Scene::Material* material)
			{
				mutex.lock();
				if(!material->shader->renderShader)
				{
					material->shader->renderShader = CreateShader(material->shader);
				}
				mutex.unlock();
			}

			void PrepareNode(Scene::Node* node)
			{
				mutex.lock();
				if (!node->renderNode)
				{
					UniformBuffer* uBuffer = new UniformBuffer();
					ManagedBuffer* buffer;
					VulkanNode* vkNode;
					const vk::DeviceSize allocSize = aligned(sizeof(glm::mat4x4), uniformBufferAlignment);
					if (node->GetUpdateFrequency() != Scene::UpdateFrequency::Never)
					{
						vkNode = new VulkanNodeDynamic();
						uint32_t imgs = context->swapChain.GetImageCount();
						buffer = CreateBuffer(imgs * allocSize, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible);
						buffer->Map();
					}
					else
					{
						vkNode = new VulkanNode();
						buffer = CreateDeviceOnlyBufferWithData(sizeof(glm::mat4), vk::BufferUsageFlagBits::eUniformBuffer, &node->worldMat);
					}
					uBuffer->Init(buffer, allocSize, &context->pipeline.descriptorSetLayout, context->pipeline.pipelineLayout);
					vkNode->Init(node, uBuffer);
					node->renderNode = vkNode;
				}
				mutex.unlock();
			}

			void RemoveShader(VulkanShader* shader) override
			{
				Utils::Remove(shaders, shader);
			}

		protected: // Allocation management
			static vk::DeviceSize aligned(vk::DeviceSize size, vk::DeviceSize byteAlignment)
			{
				return (size + byteAlignment - 1) & ~(byteAlignment - 1);
			}

			void FreeBuffer(ManagedBuffer* buffer)
			{
				toFree[currentBuffer].push_back(buffer);
			}

			void DoFreeBuffer(ManagedBuffer* buffer)
			{
				if (buffer->IsLast())
				{
					device.destroyBuffer(buffer->buffer);
					buffer->allocation->used -= buffer->size;
				}
				else
				{
					recycleBuffers.push_back(buffer);
				}
			}

			void FreeBuffers()
			{
				for (auto& i : toFree[currentBuffer])
				{
					DoFreeBuffer(i);
				}
				toFree[currentBuffer].clear();
			}

			ManagedBuffer* CreateDeviceOnlyBufferWithData(vk::DeviceSize size, vk::BufferUsageFlagBits usage, void* data)
			{
				ManagedBuffer* target = CreateBuffer(size, usage | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal);
				ManagedBuffer* uploadBuffer = CreateBuffer(size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible);
				uploadBuffer->Copy(data, size, 0);
				RecordCopy(uploadBuffer->buffer, target->buffer, size);
				FreeBuffer(uploadBuffer);
				return target;
			}

			void RecordCopy(vk::Buffer src, vk::Buffer dest, vk::DeviceSize size) const
			{
				vk::BufferCopy copyRegion = { 0, 0, size };
				cmdBuffers[currentBuffer].copyBuffer(src, dest, 1, &copyRegion);
			}
			
			ManagedBuffer* CreateBuffer(vk::DeviceSize size, const vk::BufferUsageFlags& usage, const vk::MemoryPropertyFlags& properties)
			{
				size = aligned(size, uniformBufferAlignment);
				const vk::BufferCreateInfo bufferCreateInfo = { {}, size, usage, vk::SharingMode::eExclusive };
				vk::Buffer buffer = device.createBuffer(bufferCreateInfo);
				const vk::MemoryRequirements memoryRequirements = device.getBufferMemoryRequirements(buffer);
				uint32_t memtype = context->device->GetMemoryType(memoryRequirements.memoryTypeBits, properties);
				if (memoryRequirements.size != size) Logger::DATA->warn("Memory Requirement Size ({0}) != Size ({1})", memoryRequirements.size, size);
				MemoryAllocation* allocation = GetFreeMemoryAllocation(memoryRequirements.size, memtype);
				uint32_t offset = allocation->used;
				device.bindBufferMemory(buffer, allocation->memory, offset);
				allocation->used += memoryRequirements.size;
				return new ManagedBuffer{ allocation, offset, size, buffer, usage, properties, device, nullptr };
			}
			
			MemoryAllocation* CreateMemoryAllocation(size_t size, uint32_t type, bool addToCache = true)
			{
				MemoryAllocation* alloc = new MemoryAllocation(size, type);
				const vk::MemoryAllocateInfo allocInfo = { size, type };
				alloc->memory = device.allocateMemory(allocInfo);
				if (addToCache) allocations.push_back(alloc);
				return alloc;
			}

			MemoryAllocation* GetFreeMemoryAllocation(size_t size, uint32_t type, bool createIfAllFull = true)
			{
				MemoryAllocation* alloc = nullptr;
				for (MemoryAllocation* allocation : allocations)
				{
					if (allocation->type == type && allocation->FreeSpace() >= size)
					{
						alloc = allocation;
						break;
					}
				}
				if(!alloc && createIfAllFull) alloc = CreateMemoryAllocation(256 * 1024 * 1024, type, true);
				if(alloc) lastAllocation = alloc;
				return alloc;
			}

		public:
			VulkanShader* CreateShader(Scene::Shader* shader)
			{
				VulkanShader* vkShader = new VulkanShader();
				vkShader->Init(context, shader, this);
				shaders.push_back(vkShader);
				return vkShader;
			}
		};
	}
}
