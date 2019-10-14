#pragma once
#include <cstdint>
#include <vulkan/vulkan.hpp>
#include "../Base/ICloseable.hpp"
#include "Image.hpp"
#include "Device.hpp"

namespace openVulkanoCpp
{
	namespace Vulkan
	{
		class RenderPass;

		class FrameBuffer : ICloseable
		{
			Image depthBuffer;
			std::vector<vk::Framebuffer> frameBuffers;
			vk::Format depthBufferFormat = vk::Format::eUndefined, colorFormat = vk::Format::eUndefined;
			vk::Extent3D size;
			RenderPass* renderPass;
			bool useDepthBuffer;
			Device* device = nullptr;
		protected:
			uint32_t currentFrameBufferId = 0;

			FrameBuffer() = default;

			virtual ~FrameBuffer()
			{
				if (device) FrameBuffer::Close();
			}

			void Init(Device* device, vk::Extent3D size, bool useDepthBuffer = true);

			void SetCurrentFrameId(uint32_t id)
			{
				currentFrameBufferId = id;
			}

			uint32_t GetCurrentFrameId() const
			{
				return currentFrameBufferId;
			}

		public:
			void InitRenderPass(RenderPass* renderPass);

		protected:
			void Resize(vk::Extent3D size);

			void Close() override
			{
				DestroyFrameBuffer();
				if(depthBuffer) depthBuffer.Close();
				device = nullptr;
			}

		protected:

			virtual void CreateDepthStencil();

			virtual void CreateFrameBuffer();

			void DestroyFrameBuffer();

			virtual vk::Format FindColorFormat() = 0;

			virtual vk::Format FindDepthFormat()
			{
				return device->GetSupportedDepthFormat();
			}

		public:
			virtual vk::Format GetColorFormat()
			{
				return colorFormat;
			}

			virtual vk::Format GetDepthFormat()
			{
				return depthBufferFormat;
			}

			virtual std::vector<IImage*> GetImages() = 0;

			bool UseDepthBuffer() const
			{
				return useDepthBuffer;
			}

			vk::Extent3D GetSize3D() const
			{
				return size;
			}

			vk::Extent2D GetSize2D() const
			{
				return { size.width, size.height };
			}

			vk::Framebuffer& GetCurrentFrameBuffer()
			{
				return frameBuffers[currentFrameBufferId];
			}
		};
	}
}
