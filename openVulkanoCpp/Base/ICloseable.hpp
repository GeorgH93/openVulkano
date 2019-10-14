#pragma once

namespace openVulkanoCpp
{
	class ICloseable
	{
	public:
		virtual ~ICloseable() = default;

		virtual void Close() = 0;
	};
}