#pragma once
#include <stdexcept>
#include <vector>
#include "../Base/ICloseable.hpp"
#include "Device.hpp"

namespace openVulkanoCpp
{
	namespace Vulkan
	{

		class DeviceManager : virtual public ICloseable
		{
			std::vector<Device> devices;
		public:
			void Init(const vk::Instance& instance)
			{
				devices = std::vector<Device>();
				for (auto& physicalDevice : instance.enumeratePhysicalDevices())
				{
					devices.emplace_back(physicalDevice);
				}
			}

			Device* GetCompatibleDevice(const std::vector<std::string>& deviceExtensions)
			{
				for (auto& device : devices)
				{
					if (device.IsExtensionAvailable(deviceExtensions)) return &device;
				}
				throw std::runtime_error("No device with required extensions found!");
			}

			void Close() override
			{
				for (auto& device : devices)
				{
					device.Close();
				}
				devices.clear();
			}
		};
	}
}
