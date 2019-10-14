#pragma once

namespace openVulkanoCpp
{
	class ITickable
	{
	public:
		virtual ~ITickable() = default;

		virtual void Tick() = 0;
	};
}