#pragma once
#include <vulkan/vulkan.hpp>
#include <set>
#include <functional>
#include <fstream>

namespace openVulkanoCpp
{
	namespace Vulkan
	{
		class DeviceQueueCreateInfoBuilder
		{
			std::vector<vk::DeviceQueueCreateInfo> createInfos;
			std::vector<std::vector<float>> prioritiesVector;
		public:
			DeviceQueueCreateInfoBuilder() = default;
			~DeviceQueueCreateInfoBuilder() = default;

			void AddQueueFamily(const uint32_t queueFamilyIndex, const std::vector<float>& priorities)
			{
				prioritiesVector.push_back(priorities);
				createInfos.emplace_back(vk::DeviceQueueCreateFlags(), queueFamilyIndex, priorities.size(), prioritiesVector[prioritiesVector.size()-1].data());
			}

			void AddQueueFamily(uint32_t queueFamilyIndex, uint32_t count = 1)
			{
				std::vector<float> priorities;
				priorities.resize(count);
				std::fill(priorities.begin(), priorities.end(), 0.0f);
				AddQueueFamily(queueFamilyIndex, priorities);
			}

			std::vector<vk::DeviceQueueCreateInfo>& GetDeviceQueueCreateInfos()
			{
				return createInfos;
			}
		};

		class Device : virtual public ICloseable
		{
		public:
			vk::PhysicalDevice physicalDevice;
			std::vector<vk::QueueFamilyProperties> queueFamilyProperties; // Queue family properties
			vk::PhysicalDeviceProperties properties; // Physical device properties (for e.g. checking device limits)
			vk::PhysicalDeviceFeatures features; // Physical device features (for e.g. checking if a feature is available)
			vk::PhysicalDeviceMemoryProperties memoryProperties; // available memory properties
			vk::Device device; // Logical device, application's view of the physical device (GPU)
			vk::PipelineCache pipelineCache;
			vk::CommandPool graphicsCommandPool;
			std::set<std::string> supportedExtensions;
			vk::Queue graphicsQueue;

			struct QueueIndices {
				uint32_t graphics = VK_QUEUE_FAMILY_IGNORED;
				uint32_t compute = VK_QUEUE_FAMILY_IGNORED;
				uint32_t transfer = VK_QUEUE_FAMILY_IGNORED;

				uint32_t GetGraphics() const { return graphics; }
				uint32_t GetCompute() const { return compute != graphics ? compute : VK_QUEUE_FAMILY_IGNORED; }
				uint32_t GetTransfer() const { return (transfer != graphics && transfer != compute) ? transfer : VK_QUEUE_FAMILY_IGNORED; }
			} queueIndices;

			bool useDebugMarkers;

		public:
			Device(vk::PhysicalDevice& physicalDevice)
			{
				this->physicalDevice = physicalDevice;
				useDebugMarkers = false;
				QueryDevice();
			}

			std::string GetDeviceName() const
			{
				return properties.deviceName;
			}

			void PrepareDevice(const vk::ArrayProxy<const std::string>& requestedExtensions, const vk::SurfaceKHR& surface)
			{
				queueIndices.graphics = FindBestQueue(vk::QueueFlagBits::eGraphics, surface); // Make sure that the graphics queue supports the surface
				BuildDevice(requestedExtensions);
				//TODO setup debug marker
				pipelineCache = device.createPipelineCache(vk::PipelineCacheCreateInfo());

				graphicsQueue = device.getQueue(queueIndices.graphics, 0);
				graphicsCommandPool = device.createCommandPool({ vk::CommandPoolCreateFlagBits::eResetCommandBuffer, queueIndices.graphics, });
			}

			std::set<std::string> GetExtensions() const
			{
				return supportedExtensions;
			}

			bool IsExtensionAvailable(const vk::ArrayProxy<const std::string>& extensions) const
			{
				for(const auto& extension : extensions)
				{
					if (supportedExtensions.count(extension) == 0) return false;
				}
				return true;
			}

			void WaitIdle() const
			{ //TODO wait all queues idle
				graphicsQueue.waitIdle();
				device.waitIdle();
			}


			vk::CommandBuffer CreateCommandBuffer(vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary) const
			{
				const vk::CommandBufferAllocateInfo cmdBufferAllocInfo(graphicsCommandPool, level, 1);
				return device.allocateCommandBuffers(cmdBufferAllocInfo)[0];
			}

			void FlushCommandBuffer(vk::CommandBuffer& cmdBuffer) const
			{
				graphicsQueue.submit(vk::SubmitInfo{ 0, nullptr, nullptr, 1, &cmdBuffer }, vk::Fence());
				WaitIdle();
			}

			void ExecuteNow(const std::function<void(const vk::CommandBuffer& commandBuffer)>& function) const
			{
				vk::CommandBuffer commandBuffer = CreateCommandBuffer();
				commandBuffer.begin(vk::CommandBufferBeginInfo{ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
				function(commandBuffer);
				commandBuffer.end();
				FlushCommandBuffer(commandBuffer);
				device.freeCommandBuffers(graphicsCommandPool, commandBuffer);
			}

			vk::ShaderModule CreateShaderModule(const std::string filename)
			{
				std::ifstream file(filename, std::ios::ate | std::ios::binary);
				if (!file.is_open())  throw std::runtime_error("Failed to open shader file!");
				const size_t fileSize = static_cast<size_t>(file.tellg());
				std::vector<char> buffer(fileSize);
				file.seekg(0);
				file.read(buffer.data(), fileSize);
				file.close();
				vk::ShaderModuleCreateInfo smci = { {}, buffer.size(), reinterpret_cast<const uint32_t*>(buffer.data()) };
				return CreateShaderModule(smci);
			}

			vk::ShaderModule CreateShaderModule(vk::ShaderModuleCreateInfo& createInfo) const
			{
				return device.createShaderModule(createInfo);
			}
		private:
			void QueryDevice()
			{
				// Query device features
				queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
				properties = physicalDevice.getProperties();
				features = physicalDevice.getFeatures();
				for (auto& ext : physicalDevice.enumerateDeviceExtensionProperties()) { supportedExtensions.insert(ext.extensionName); }

				// Query device memory properties
				memoryProperties = physicalDevice.getMemoryProperties();
				queueIndices.graphics = FindBestQueue(vk::QueueFlagBits::eGraphics);
				queueIndices.compute = FindBestQueue(vk::QueueFlagBits::eCompute);
				queueIndices.transfer = FindBestQueue(vk::QueueFlagBits::eTransfer);
			}

			void BuildDevice(const vk::ArrayProxy<const std::string>& requestedExtensions)
			{
				vk::DeviceCreateInfo deviceCreateInfo;
				deviceCreateInfo.pEnabledFeatures = &features; //TODO add option to disable not needed features

				
				DeviceQueueCreateInfoBuilder deviceQueueCreateInfoBuilder;
				deviceQueueCreateInfoBuilder.AddQueueFamily(queueIndices.GetGraphics(), queueFamilyProperties[queueIndices.GetGraphics()].queueCount);
				if (queueIndices.GetCompute() != VK_QUEUE_FAMILY_IGNORED)
					deviceQueueCreateInfoBuilder.AddQueueFamily(queueIndices.GetCompute(), queueFamilyProperties[queueIndices.GetCompute()].queueCount);
				if (queueIndices.GetTransfer() != VK_QUEUE_FAMILY_IGNORED)
					deviceQueueCreateInfoBuilder.AddQueueFamily(queueIndices.GetTransfer(), queueFamilyProperties[queueIndices.GetTransfer()].queueCount);
				const std::vector<vk::DeviceQueueCreateInfo> deviceQueueCreateInfos = deviceQueueCreateInfoBuilder.GetDeviceQueueCreateInfos();

				deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(deviceQueueCreateInfos.size());
				deviceCreateInfo.pQueueCreateInfos = deviceQueueCreateInfos.data();

				std::vector<const char*> enabledExtensions;
				for (const auto& extension : requestedExtensions)
				{
					enabledExtensions.push_back(extension.c_str());
				}
#ifdef DEBUG
				if (IsExtensionAvailable({ VK_EXT_DEBUG_MARKER_EXTENSION_NAME }))
				{ // Enable debug marker extension if available
					enabledExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
					useDebugMarkers = true;
				}
#endif
				deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
				deviceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();

				device = physicalDevice.createDevice(deviceCreateInfo);
			}

			uint32_t FindBestQueue(const vk::QueueFlags& desiredFlags, const vk::SurfaceKHR& surface = nullptr) const
			{
				uint32_t best = VK_QUEUE_FAMILY_IGNORED;
				VkQueueFlags bestExtraFlagsCount = VK_QUEUE_FLAG_BITS_MAX_ENUM;
				for (size_t i = 0; i < queueFamilyProperties.size(); i++)
				{
					vk::QueueFlags flags = queueFamilyProperties[i].queueFlags;
					if (!(flags & desiredFlags)) continue; // Skip queue without desired flags
					if (surface && VK_FALSE == physicalDevice.getSurfaceSupportKHR(i, surface)) continue;
					const VkQueueFlags currentExtraFlags = (flags & ~desiredFlags).operator VkQueueFlags();

					if (0 == currentExtraFlags) return i; // return exact match

					if (best == VK_QUEUE_FAMILY_IGNORED || currentExtraFlags < bestExtraFlagsCount)
					{
						best = i;
						bestExtraFlagsCount = currentExtraFlags;
					}
				}
				return best;
			}

		public:
			/**
			 * \brief Vulkan does not require does not define a depth buffer format that must be supported. This method checks for the first supported format from an given array of formats.
			 * \param depthFormats Array of depth formats
			 * \return The first format supported as a depth buffer
			 * \throws If no depth buffer format is supported
			 */
			vk::Format GetSupportedDepthFormat(const std::vector<vk::Format>& depthFormats = { vk::Format::eD32SfloatS8Uint, vk::Format::eD32Sfloat, vk::Format::eD24UnormS8Uint, vk::Format::eD16UnormS8Uint, vk::Format::eD16Unorm }) const
			{
				for (auto& format : depthFormats)
				{
					vk::FormatProperties formatProps;
					formatProps = physicalDevice.getFormatProperties(format);
					if (formatProps.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
					{
						return format;
					}
				}

				throw std::runtime_error("No supported depth format");
			}

			vk::Bool32 GetMemoryType(uint32_t typeBits, const vk::MemoryPropertyFlags& properties, uint32_t* typeIndex) const
			{
				for (uint32_t i = 0; i < 32; i++)
				{
					if ((typeBits & 1) == 1)
					{
						if ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
						{
							*typeIndex = i;
							return VK_TRUE;
						}
					}
					typeBits >>= 1;
				}
				return VK_FALSE;
			}

			uint32_t GetMemoryType(uint32_t typeBits, const vk::MemoryPropertyFlags& properties) const
			{
				uint32_t result = 0;
				if (VK_FALSE == GetMemoryType(typeBits, properties, &result))
				{
					throw std::runtime_error("Unable to find memory type " + to_string(properties));
				}
				return result;
			}


			void Close() override
			{
				device.destroyCommandPool(graphicsCommandPool);
				//TODO fill
			}
		};
	}
}
