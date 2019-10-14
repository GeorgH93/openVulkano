#include "FrameBuffer.hpp"
#include "RenderPass.hpp"

void openVulkanoCpp::Vulkan::FrameBuffer::Init(Device* device, vk::Extent3D size, bool useDepthBuffer)
{
	this->size = size;
	this->device = device;
	this->useDepthBuffer = useDepthBuffer;
	colorFormat = FindColorFormat();
	if (useDepthBuffer)
	{
		depthBufferFormat = FindDepthFormat();
		CreateDepthStencil();
	}
}

void openVulkanoCpp::Vulkan::FrameBuffer::InitRenderPass(RenderPass* renderPass)
{
	if (!device) throw std::
		runtime_error("The frame buffer needs to be initialized before binding it to a render pass");
	this->renderPass = renderPass;
	CreateFrameBuffer();
}

void openVulkanoCpp::Vulkan::FrameBuffer::Resize(vk::Extent3D size)
{
	this->size = size;
	DestroyFrameBuffer();
	if (depthBuffer) depthBuffer.Close();
	if (useDepthBuffer) CreateDepthStencil();
	CreateFrameBuffer();
	renderPass->UpdateBeginInfo();
}

void openVulkanoCpp::Vulkan::FrameBuffer::CreateDepthStencil()
{
	vk::ImageCreateInfo depthStencilCreateInfo({}, vk::ImageType::e2D, depthBufferFormat,
	                                           size, 1, 1);
	depthStencilCreateInfo.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::
		eTransferSrc;

	const vk::ImageAspectFlags aspectMask = vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
	const vk::ImageViewCreateInfo depthStencilViewCreateInfo({}, {}, vk::ImageViewType::e2D, depthBufferFormat,
	                                                         {}, vk::ImageSubresourceRange(aspectMask, 0, 1, 0, 1));
	depthBuffer.Init(device, depthStencilCreateInfo, depthStencilViewCreateInfo);

	device->ExecuteNow([&](auto commandBuffer)
	{
		depthBuffer.SetLayout(commandBuffer, aspectMask, vk::ImageLayout::eDepthStencilAttachmentOptimal);
	});
}

void openVulkanoCpp::Vulkan::FrameBuffer::CreateFrameBuffer()
{
	vk::ImageView attachments[2]; // First attachment is the color buffer, second (optional) the depth buffer
	if (useDepthBuffer) attachments[1] = depthBuffer.view; //Depth buffer is the same for all frame buffers
	const vk::FramebufferCreateInfo fbCreateInfo({}, renderPass->renderPass, useDepthBuffer ? 2 : 1, attachments, size.width,
	                                             size.height, 1);

	auto images = GetImages();
	frameBuffers.resize(images.size());
	for (uint32_t i = 0; i < frameBuffers.size(); i++)
	{
		attachments[0] = images[i]->GetView();
		frameBuffers[i] = device->device.createFramebuffer(fbCreateInfo);
	}
}

void openVulkanoCpp::Vulkan::FrameBuffer::DestroyFrameBuffer()
{
	for (const auto frameBuffer : frameBuffers)
	{
		device->device.destroyFramebuffer(frameBuffer);
	}
	frameBuffers.clear();
}
