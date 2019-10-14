#pragma once
#include "../Base/ICloseable.hpp"
#include "Device.hpp"

namespace openVulkanoCpp
{
	namespace Vulkan
	{
		/**
		 * \brief A not managed buffer. This should be used rarely.
		 */
		struct Buffer : virtual public ICloseable
		{
			vk::Device device;
			vk::DeviceMemory memory;
			vk::DeviceSize size = 0, alignment = 0, allocSize = 0;
			vk::MemoryPropertyFlags memoryPropertyFlags;
			void* mapped = nullptr;

			/**
			 * \brief Maps the buffer into the memory of the host.
			 * \tparam T The type of the buffers data.
			 * \param offset The offset from where to map the buffer.
			 * \param size The size to be mapped. VK_WHOLE_SIZE to map the whole buffer.
			 * \return The pointer to the mapped buffer.
			 */
			template <typename T = void>
			T * Map(size_t offset = 0, VkDeviceSize size = VK_WHOLE_SIZE)
			{
				mapped = device.mapMemory(memory, offset, size, vk::MemoryMapFlags());
				return static_cast<T*>(mapped);
			}

			/**
			 * \brief Un-maps the buffer from the host.
			 */
			void UnMap()
			{
				device.unmapMemory(memory);
				mapped = nullptr;
			}

			/**
			 * \brief Copies data into the mapped buffer. Will not do anything if the buffer is not mapped!
			 * \param size The size of the data to copy.
			 * \param data The data to copy
			 * \param offset The offset for where to copy the data to in the buffer.
			 */
			void Copy(size_t size, const void* data, VkDeviceSize offset = 0) const
			{
				if (!mapped) return;
				memcpy(static_cast<uint8_t*>(mapped) + offset, data, size);
			}

			/**
			 * \brief Copies data into the mapped buffer. Will not do anything if the buffer is not mapped!
			 * \param data The data to copy.
			 * \param offset The offset for where to copy the data to in the buffer.
			 */
			template <typename T>
			void Copy(const T& data, VkDeviceSize offset = 0) const
			{
				Copy(sizeof(T), &data, offset);
			}

			/**
			 * \brief Copies data into the mapped buffer. Will not do anything if the buffer is not mapped!
			 * \param data The data to copy.
			 * \param offset The offset for where to copy the data to in the buffer.
			 */
			template <typename T>
			void Copy(const std::vector<T>& data, VkDeviceSize offset = 0) const
			{
				copy(sizeof(T) * data.size(), data.data(), offset);
			}

			/**
			 * \brief Flushes the memory region of the buffer to the device. This should be only necessary for non coherent memory.
			 * \param size The amount to flush. VK_WHOLE_SIZE flushes the entire buffer.
			 * \param offset The offset from where to start the flush.
			 */
			void Flush(vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0) const
			{
				device.flushMappedMemoryRanges(vk::MappedMemoryRange(memory, offset, size));
			}

			/**
			 * \brief Invalidates the memory region of the buffer to allow access from the host. This should be only necessary for non coherent memory.
			 * \param size The amount to make available. VK_WHOLE_SIZE invalidate the entire buffer.
			 * \param offset The offset from where to make the memory available.
			 */
			void Invalidate(vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0) const
			{
				device.invalidateMappedMemoryRanges(vk::MappedMemoryRange(memory, offset, size));
			}

			void Close() override
			{
				if (mapped) UnMap();
				if(memory)
				{
					device.free(memory);
					memory = vk::DeviceMemory();
				}
			}
			
		};
	}
}
