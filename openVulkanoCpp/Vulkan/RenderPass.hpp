#pragma once
#include <vulkan/vulkan.hpp>
#include "Device.hpp"
#include "FrameBuffer.hpp"

namespace openVulkanoCpp
{
	namespace Vulkan
	{
		class RenderPass : virtual public ICloseable
		{
		protected:
			vk::Device device;
			vk::RenderPassBeginInfo beginInfo;
			vk::ClearColorValue clearColor;
			vk::ClearDepthStencilValue clearDepth;
			std::vector<vk::ClearValue> clearValues;
			FrameBuffer* frameBuffer = nullptr;
			bool useClearColor = false, useClearDepth = true;

		public:
			vk::RenderPass renderPass;

			RenderPass() = default;

			~RenderPass()
			{
				if (frameBuffer) RenderPass::Close();
			}

			void Init(Device* device, FrameBuffer* frameBuffer)
			{
				this->device = device->device;
				this->frameBuffer = frameBuffer;
				CreateRenderPass();
				frameBuffer->InitRenderPass(this);
				SetClearColor();
			}

			void Close() override
			{
				device.destroy(renderPass);
				frameBuffer = nullptr;
				clearValues.clear();
			}

			void SetClearColor(vk::ClearColorValue clearColor = vk::ClearColorValue(std::array<float, 4>{ 0.39f, 0.58f, 0.93f, 1.0f }))
			{
				this->clearColor = clearColor;
				useClearColor = true;
				UpdateBeginInfo();
			}

			void DisableClearColor()
			{
				useClearColor = false;
				UpdateBeginInfo();
			}

			void SetClearDepth(vk::ClearDepthStencilValue clearDepth = vk::ClearDepthStencilValue(1, 0))
			{
				this->clearDepth = clearDepth;
				useClearDepth = true;
				UpdateBeginInfo();
			}

			void DisableClearDepth()
			{
				useClearDepth = false;
			}

			virtual void UpdateBeginInfo()
			{ //TODO allow to control the render rect size
				clearValues.clear();
				if (useClearColor) clearValues.emplace_back(clearColor);
				if(frameBuffer->UseDepthBuffer() && useClearDepth) clearValues.emplace_back(clearDepth);

				beginInfo = vk::RenderPassBeginInfo(renderPass, vk::Framebuffer(),
					vk::Rect2D(vk::Offset2D(), frameBuffer->GetSize2D()),
					clearValues.size(), clearValues.data());
			}

			virtual void Begin(vk::CommandBuffer& commandBuffer)
			{
				beginInfo.framebuffer = frameBuffer->GetCurrentFrameBuffer();
				commandBuffer.beginRenderPass(beginInfo, vk::SubpassContents::eSecondaryCommandBuffers);
			}

			virtual void End(vk::CommandBuffer& commandBuffer)
			{
				commandBuffer.endRenderPass();
			}

			FrameBuffer* GetFrameBuffer() const
			{
				return frameBuffer;
			}

		protected:
			virtual void CreateRenderPass()
			{
				std::vector<vk::AttachmentDescription> attachments;
				attachments.reserve(frameBuffer->UseDepthBuffer() ? 2 : 1);

				// Color attachment
				attachments.emplace_back(vk::AttachmentDescriptionFlags(), frameBuffer->GetColorFormat(), vk::SampleCountFlagBits::e1,
					vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eLoad,
					vk::AttachmentStoreOp::eStore, vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR);

				vk::AttachmentReference* depthReference = nullptr;;
				if (frameBuffer->UseDepthBuffer())
				{ // Depth attachment
					attachments.emplace_back(vk::AttachmentDescriptionFlags(), frameBuffer->GetDepthFormat(), vk::SampleCountFlagBits::e1,
						vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eDontCare, vk::AttachmentLoadOp::eClear,
						vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);
					depthReference = new vk::AttachmentReference(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);
				}

				std::vector<vk::AttachmentReference> colorAttachmentReferences;
				colorAttachmentReferences.emplace_back(0, vk::ImageLayout::eColorAttachmentOptimal);

				std::vector<vk::SubpassDescription> subPasses;
				subPasses.emplace_back(vk::SubpassDescriptionFlags(), vk::PipelineBindPoint::eGraphics, 0, nullptr,
					colorAttachmentReferences.size(), colorAttachmentReferences.data(), nullptr, depthReference, 0, nullptr);
				std::vector<vk::SubpassDependency> subPassDependencies;
				subPassDependencies.emplace_back(0, VK_SUBPASS_EXTERNAL, vk::PipelineStageFlagBits::eColorAttachmentOutput,
					vk::PipelineStageFlagBits::eBottomOfPipe, vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite,
					vk::AccessFlagBits::eMemoryRead, vk::DependencyFlagBits::eByRegion);
				subPassDependencies.emplace_back(VK_SUBPASS_EXTERNAL, 0, vk::PipelineStageFlagBits::eBottomOfPipe,
					vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::AccessFlagBits::eMemoryRead,
					vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite,
					vk::DependencyFlagBits::eByRegion);

				const vk::RenderPassCreateInfo createInfo(vk::RenderPassCreateFlags(), attachments.size(), attachments.data(),
					subPasses.size(), subPasses.data(), subPassDependencies.size(), subPassDependencies.data());
				CreateRenderPass(createInfo);
				delete depthReference;
			}

			void CreateRenderPass(const vk::RenderPassCreateInfo& renderPassCreateInfo)
			{
				renderPass = device.createRenderPass(renderPassCreateInfo);
			}
		};
	}
}