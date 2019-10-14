#pragma once
#include <set>
#include <vulkan/vulkan.hpp>

#define RENDER_DOC

namespace openVulkanoCpp
{
	namespace Vulkan
	{
		namespace Debug
		{
			std::list<std::string> activeValidationLayerNames = {
				"VK_LAYER_LUNARG_assistant_layer",
				"VK_LAYER_LUNARG_standard_validation",
				//"VK_EXT_debug_marker",
#ifdef RENDER_DOC
				"VK_LAYER_RENDERDOC_Capture", // RenderDoc must be open for this layer to work!
#endif
			};

			static std::set<std::string> GetAvailableValidationLayers()
			{
				auto layers = vk::enumerateInstanceLayerProperties();
				std::set<std::string> layersVector;
				std::string layerList = "";
				for(const auto& layer : layers)
				{
					std::string name = layer.layerName;
					layersVector.insert(name);
					if (layerList.length() > 0) layerList += ", ";
					layerList += name;
				}
				Logger::RENDER->debug("Available Vulkan Validation Layers: {0}", layerList);
				return layersVector;
			}

			static std::vector<const char*> GetValidationLayers()
			{
				std::set<std::string> availableLayers = GetAvailableValidationLayers();
				std::vector<const char*> layers;
				std::string layerList = "";
				for (const auto& name : activeValidationLayerNames)
				{
					if (availableLayers.count(name) != 0)
					{
						layers.push_back(name.c_str());
						if (layerList.length() > 0) layerList += ", ";
						layerList += name;
					}
				}
				Logger::RENDER->debug("Active Vulkan Validation Layers: {0}", layerList);
				return layers;
			}

			static std::once_flag dispatcherInitFlag;
			vk::DispatchLoaderDynamic dispatcher;
			vk::DebugReportCallbackEXT msgCallback;

			inline VkBool32 ValidationLayerCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType,
				uint64_t srcObject, size_t location, int32_t msgCode, const char* layerPrefix,
				const char* msg, void* pUserData)
			{
				std::string prefix = "VK_DEBUG:";
				spdlog::level::level_enum level = spdlog::level::info;
				if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) level = spdlog::level::err;
				else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) level = spdlog::level::warn;
				else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
				{
					level = spdlog::level::warn;
					prefix = "[PERF] " + prefix;
				}
				else if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) level = spdlog::level::debug;

				Logger::RENDER->log(level, "{0} [{1}] Code {2}: {3}", prefix, layerPrefix, msgCode, msg);

				return false;
			}

			static void SetupValidationLayers(const vk::Instance& instance, const vk::DebugReportFlagsEXT& flags)
			{
				Logger::RENDER->info("Setting up Vulkan Validation Layer");
				std::call_once(dispatcherInitFlag, [&] { dispatcher.init(instance, &vkGetInstanceProcAddr); });
				vk::DebugReportCallbackCreateInfoEXT dbgCreateInfo = {};
				dbgCreateInfo.pfnCallback = (PFN_vkDebugReportCallbackEXT)ValidationLayerCallback;
				dbgCreateInfo.flags = flags;
				msgCallback = instance.createDebugReportCallbackEXT(dbgCreateInfo, nullptr, dispatcher);
				Logger::RENDER->info("Vulkan Validation Layer setup");
			}

			static void CloseValidationLayers(const vk::Instance& instance) {
				std::call_once(dispatcherInitFlag, [&] { dispatcher.init(instance, &vkGetInstanceProcAddr); });
				instance.destroyDebugReportCallbackEXT(msgCallback, nullptr, dispatcher);
			}
		};
	}
}
