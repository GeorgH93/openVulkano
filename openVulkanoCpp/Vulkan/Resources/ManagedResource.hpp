#pragma once
#include <vulkan/vulkan.hpp>

namespace openVulkanoCpp
{
	namespace Vulkan
	{
		struct MemoryAllocation
		{
			vk::DeviceMemory memory;
			size_t used, size;
			uint32_t type;

			MemoryAllocation(size_t size, uint32_t type)
			{
				memory = nullptr;
				this->size = size;
				used = 0;
				this->type = type;
			}

			size_t FreeSpace() const
			{
				return size - used;
			}
		};

		struct ManagedBuffer
		{
			MemoryAllocation* allocation;
			vk::DeviceSize offset, size;
			vk::Buffer buffer;
			vk::BufferUsageFlags usage;
			vk::MemoryPropertyFlags properties;
			vk::Device device;
			void* mapped = nullptr;

			bool IsLast()
			{
				return (offset + size == allocation->used);
			}

			/**
			 * \brief Maps the buffer into the memory of the host.
			 * \tparam T The type of the buffers data.
			 * \param offset The offset from where to map the buffer.
			 * \param size The size to be mapped. VK_WHOLE_SIZE to map the whole buffer.
			 * \return The pointer to the mapped buffer.
			 */
			template <typename T = void>
			T* Map(size_t offset = 0, vk::DeviceSize size = VK_WHOLE_SIZE)
			{
				if (size == VK_WHOLE_SIZE) size = this->size;
				mapped = device.mapMemory(allocation->memory, this->offset + offset, size, vk::MemoryMapFlags());
				return static_cast<T*>(mapped);
			}

			/**
			 * \brief Un-maps the buffer from the host.
			 */
			void UnMap()
			{
				device.unmapMemory(allocation->memory);
				mapped = nullptr;
			}

			void Copy(void* data) const
			{
				if(mapped)
				{
					memcpy(mapped, data, size);
				}
				else
				{
					void* dataMapped = device.mapMemory(allocation->memory, offset, size);
					memcpy(dataMapped, data, size);
					device.unmapMemory(allocation->memory);
				}
			}

			void Copy(void* data, uint32_t size, uint32_t offset) const
			{
				if(mapped) memcpy(static_cast<char*>(mapped) + offset, data, size);
				else
				{
					void* dataMapped = device.mapMemory(allocation->memory, this->offset + offset, size);
					memcpy(dataMapped, data, size);
					device.unmapMemory(allocation->memory);
				}
			}
		};
	}
}
