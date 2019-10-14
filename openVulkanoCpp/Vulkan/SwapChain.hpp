#pragma once
#include <algorithm>
#include <vulkan/vulkan.hpp>
#include "Device.hpp"
#include "Image.hpp"
#include "FrameBuffer.hpp"

namespace openVulkanoCpp
{
	namespace Vulkan
	{
		struct SwapChainImage : virtual public IImage
		{
			vk::Image image;
			vk::ImageView view;
			vk::Fence fence;


			vk::Image GetImage() override
			{
				return image;
			}

			vk::ImageView GetView() override
			{
				return view;
			}
		};

		class SwapChain : public FrameBuffer
		{
			vk::SurfaceKHR surface;
			std::vector<SwapChainImage> images;
			Device* device = nullptr;
			IVulkanWindow* window = nullptr;
			vk::SurfaceFormatKHR surfaceFormat;
			vk::PresentModeKHR presentMode;
			vk::Viewport fullscreenViewport;
			bool useVsync = false;

			uint32_t preferredImageCount = 2; //TODO add option
			vk::Extent2D size{0,0};

		public:
			vk::SwapchainKHR swapChain;
			vk::Semaphore imageAvailableSemaphore;

			SwapChain() = default;
			~SwapChain() { if (device) SwapChain::Close(); }

			void Init(Device* device, vk::SurfaceKHR surface, IVulkanWindow* window)
			{
				if (!device) throw std::runtime_error("The device must not be null");
				if (!window) throw std::runtime_error("The window must not be null");
				this->device = device;
				this->surface = surface;
				this->window = window;

				imageAvailableSemaphore = device->device.createSemaphore({});
				CreateSwapChain({window->GetWidth(), window->GetHeight() });

				FrameBuffer::Init(device, vk::Extent3D(size, 1));
			}

			void Close() override
			{
				DestroySwapChain();
				device->device.destroySemaphore(imageAvailableSemaphore);
				device = nullptr;
				FrameBuffer::Close();
			}

			void Resize(const uint32_t newWidth, const uint32_t newHeight)
			{
				if(newWidth == 0 || newHeight == 0) return; // Swap chain size of 0 pixel is not allowed
				
				CreateSwapChain({ newWidth, newHeight });
				FrameBuffer::Resize(vk::Extent3D(size, 1));
			}

			vk::Extent2D GetSize() const
			{
				return size;
			}

			vk::Viewport GetFullscreenViewport() const
			{
				return fullscreenViewport;
			}

			vk::Rect2D GetFullscreenScissor() const
			{
				return { {0,0}, GetSize() };
			}
			
			uint32_t AcquireNextImage(const vk::Fence fence = vk::Fence())
			{
				const auto resultValue = device->device.acquireNextImageKHR(swapChain, UINT64_MAX, imageAvailableSemaphore, fence);
				const vk::Result result = resultValue.result;
				if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) throw std::error_code(result);
				SetCurrentFrameId(resultValue.value);

				vk::Fence submitFence = GetCurrentSubmitFence();
				device->device.waitForFences(1, &submitFence, true, -1);
				device->device.resetFences(1, &submitFence);

				return currentFrameBufferId;
			}

			vk::Fence& GetCurrentSubmitFence()
			{
				return images[currentFrameBufferId].fence;
			}

			void Present(vk::Queue& queue ,std::vector<vk::Semaphore>& semaphores) const
			{
				queue.presentKHR(vk::PresentInfoKHR(semaphores.size(), semaphores.data(),
					1, &swapChain, &currentFrameBufferId));
			}

			bool UseVsync() { return useVsync; }

			void SetVsync(bool useVsync)
			{ //TODO change swap chain
				this->useVsync = useVsync;
			}

			uint32_t GetImageCount() const
			{
				return images.size();
			}

		private:
			void CreateSwapChain(vk::Extent2D size)
			{
				Logger::RENDER->debug("Creating swap chain for window {0} ...", window->GetWindowId());
				surfaceFormat = ChoseSurfaceFormat();
				presentMode = ChosePresentMode();
				const vk::SurfaceCapabilitiesKHR surfaceCapabilities = device->physicalDevice.getSurfaceCapabilitiesKHR(surface);
				if(surfaceCapabilities.currentExtent.width != ~static_cast<uint32_t>(0))
				{ // The surface does provide it's size to the vulkan driver
					size = surfaceCapabilities.currentExtent;
				}

				vk::SurfaceTransformFlagBitsKHR preTransform; //TODO add option to allow rotation and other modifications
				if (surfaceCapabilities.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity)
				{ preTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity; }
				else { preTransform = surfaceCapabilities.currentTransform; }

                uint32_t usingImages = std::max(preferredImageCount, surfaceCapabilities.minImageCount);
				if (surfaceCapabilities.maxImageCount > 0) //GPU has limit of swap chain images
					usingImages = std::min(usingImages, surfaceCapabilities.maxImageCount);
				Logger::RENDER->debug("GPU supports {0} to {1} swap chain images. Preferred: {2}, Using: {3}", surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount, preferredImageCount, usingImages);
				
				const vk::SwapchainCreateInfoKHR createInfo({}, surface, usingImages, surfaceFormat.format,
					surfaceFormat.colorSpace, size, 1,
					vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst,
					vk::SharingMode::eExclusive, 0, nullptr, preTransform,
					vk::CompositeAlphaFlagBitsKHR::eOpaque, presentMode, VK_TRUE, swapChain);
				const vk::SwapchainKHR newSwapChain = device->device.createSwapchainKHR(createInfo);

				DestroySwapChain();
				swapChain = newSwapChain;
				this->size = size;

				CreateSwapChainImages();

				fullscreenViewport = vk::Viewport{ 0, 0, (float)size.width, (float)size.height, 0, 1 };
				Logger::RENDER->debug("Swap chain for window {0} created", window->GetWindowId());
			}

			void CreateSwapChainImages()
			{
				vk::ImageViewCreateInfo imgViewCreateInfo;
				imgViewCreateInfo.format = surfaceFormat.format;
				imgViewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
				imgViewCreateInfo.subresourceRange.levelCount = 1;
				imgViewCreateInfo.subresourceRange.layerCount = 1;
				imgViewCreateInfo.viewType = vk::ImageViewType::e2D;

				auto swapChainImages = device->device.getSwapchainImagesKHR(swapChain);

				images.resize(swapChainImages.size());
				for (uint32_t i = 0; i < swapChainImages.size(); i++)
				{
					images[i].image = swapChainImages[i];
					imgViewCreateInfo.image = swapChainImages[i];
					images[i].view = device->device.createImageView(imgViewCreateInfo);
					images[i].fence = device->device.createFence({ vk::FenceCreateFlags(vk::FenceCreateFlagBits::eSignaled)});
				}
			}

			void DestroySwapChain() const
			{
				for(auto& image : images)
				{
					device->device.destroyImageView(image.view);
					device->device.destroyFence(image.fence);
				}
				device->device.destroySwapchainKHR(swapChain);
			}

		protected:
			vk::Format FindColorFormat() override
			{
				return surfaceFormat.format;
			}

			virtual vk::PresentModeKHR ChosePresentMode()
			{
				std::vector<vk::PresentModeKHR> presentModes = device->physicalDevice.getSurfacePresentModesKHR(surface);
#ifdef DEBUG
				std::string availableModes = "";
				for (const auto& presentMode : presentModes)
				{
					if (availableModes.length() > 0) availableModes += ", ";
					availableModes += vk::to_string(presentMode);
				}
				Logger::RENDER->debug("Available swap chain present modes {0}. Searching best mode for: vsync={1}", availableModes, useVsync);
#endif
				vk::PresentModeKHR mode = vk::PresentModeKHR::eFifo;
				if (useVsync)
				{
					if (Utils::Contains(presentModes, vk::PresentModeKHR::eMailbox)) mode = vk::PresentModeKHR::eMailbox;
				}
				else
				{
					if (Utils::Contains(presentModes, vk::PresentModeKHR::eImmediate)) mode = vk::PresentModeKHR::eImmediate;
					else if (Utils::Contains(presentModes, vk::PresentModeKHR::eFifoRelaxed)) mode = vk::PresentModeKHR::eFifoRelaxed;
				}
				Logger::RENDER->debug("Using swap chain present mode {0}", vk::to_string(mode));
				return mode;
			}

			virtual vk::SurfaceFormatKHR ChoseSurfaceFormat()
			{ //TODO allow to chose output format
				std::vector<vk::SurfaceFormatKHR> surfaceFormats = device->physicalDevice.getSurfaceFormatsKHR(surface);
				if(surfaceFormats.size() == 1 && surfaceFormats[0].format == vk::Format::eUndefined)
				{ // GPU does not have a preferred surface format
					return vk::SurfaceFormatKHR{ vk::Format::eB8G8R8A8Unorm, surfaceFormats[0].colorSpace };
				}
				else
				{ //TODO chose best fitting
					return surfaceFormats[0];
				}
			}


		public:
			std::vector<IImage*> GetImages() override
			{
				std::vector<IImage*> imgs;
				for (auto& image : images)
				{
					imgs.push_back(&image);
				}
				return imgs;
			}
		};
	}
}
