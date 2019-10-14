#pragma once

namespace openVulkanoCpp
{
	class IInitable
	{
	public:
		virtual ~IInitable() = default;

		virtual void Init() = 0;
	};
}