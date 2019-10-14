#pragma once
#include <vulkan/vulkan.hpp>
#include "../Base/IGraphicsApp.hpp"
#include "../Base/IGraphicsAppManager.hpp"
#include "../Base/EngineConstants.hpp"
#include "../Base/Utils.hpp"
#include "Debuging/ValidationLayer.hpp"
#include "DeviceManager.hpp"
#include "SwapChain.hpp"
#include "RenderPass.hpp"
#include "Pipeline.hpp"

namespace openVulkanoCpp
{
	namespace Vulkan
	{
		class Context : virtual public ICloseable
		{
			bool enableValidationLayer, initialized;
			std::set<std::string> requiredExtensions;
		public:
			DeviceManager deviceManager;
			vk::Instance instance; // Vulkan instance
			vk::DispatchLoaderDynamic dynamicDispatch; // for access to features not available in statically linked Vulkan lib
			vk::SurfaceKHR surface; // Vulkan surface to display framebuffer on
			Device* device = nullptr;
			SwapChain swapChain;
			RenderPass swapChainRenderPass;
			IVulkanWindow* window = nullptr;
			IGraphicsAppManager* graphicsAppManager = nullptr;
			Pipeline pipeline;

			Context() : initialized(false)
			{
#ifdef DEBUG
				enableValidationLayer = true;
#else
				enableValidationLayer = false;
#endif
			}
			virtual ~Context()
			{
				if (initialized) Close();
			}

			void Init(IGraphicsAppManager* graphicsAppManager, IVulkanWindow* window)
			{
				if (initialized) throw std::runtime_error("The context is already initialized");
				this->graphicsAppManager = graphicsAppManager;
				this->window = window;

				// Get the extensions required to display on the window
				for (const auto& requiredExtension : window->GetRequiredInstanceExtensions()) { RequireExtension(requiredExtension.c_str()); }

				CreateInstance(); // Create the vulkan instance
				surface = window->CreateSurface(instance); // Create the surface from the window
				CreateDevice();

				
				swapChain.Init(device, surface, window);
				swapChainRenderPass.Init(device, &swapChain);

				pipeline.Init(device->device);
				
				initialized = true;
			}

			void Close() override
			{
				if (!initialized) return;
				device->WaitIdle();

				pipeline.Close();
				swapChainRenderPass.Close();
				swapChain.Close();
				deviceManager.Close();
				//TODO

				if (enableValidationLayer) Debug::CloseValidationLayers(instance);
				initialized = false;
			}

			void Resize(const uint32_t newWidth, const uint32_t newHeight)
			{
				device->WaitIdle();
				swapChain.Resize(newWidth, newHeight);
			}

			void RequireExtension(const char* extension)
			{
				requiredExtensions.emplace(extension);
			}
		private:
			void CreateInstance()
			{
				if (enableValidationLayer) RequireExtension(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
				vk::ApplicationInfo appInfo(graphicsAppManager->GetGraphicsApp()->GetAppName().c_str(), graphicsAppManager->GetGraphicsApp()->GetAppVersionAsInt(), ENGINE_NAME, ENGINE_VERSION.intVersion, VK_MAKE_VERSION(1, 1, 0));
				std::vector<const char*> extensions = Utils::toCString(requiredExtensions), layers = Debug::GetValidationLayers();

				const vk::InstanceCreateInfo createInfo(vk::InstanceCreateFlags(), &appInfo, enableValidationLayer ? layers.size() : 0,
					layers.data(), extensions.size(), extensions.data());

				instance = vk::createInstance(createInfo);

				if (enableValidationLayer) Debug::SetupValidationLayers(instance, vk::DebugReportFlagBitsEXT::eError | vk::DebugReportFlagBitsEXT::eWarning | vk::DebugReportFlagBitsEXT::ePerformanceWarning /*| vk::DebugReportFlagBitsEXT::eInformation | vk::DebugReportFlagBitsEXT::eDebug*/);
				dynamicDispatch.init(instance, &vkGetInstanceProcAddr);
			}

			void CreateDevice()
			{
				deviceManager.Init(instance);
				device = deviceManager.GetCompatibleDevice({ VK_KHR_SWAPCHAIN_EXTENSION_NAME });
				device->PrepareDevice({ VK_KHR_SWAPCHAIN_EXTENSION_NAME }, surface);
				dynamicDispatch.init(instance, &vkGetInstanceProcAddr, device->device, &vkGetDeviceProcAddr);
				Logger::RENDER->info("Found device: {0}", device->GetDeviceName());;
			}
		};
	}
}
