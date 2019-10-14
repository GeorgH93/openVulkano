#pragma once
#include <stdint.h>
#include <algorithm>

namespace  openVulkanoCpp
{
	class EngineConfiguration
	{
	private:
		EngineConfiguration() = default;
		~EngineConfiguration() = default;

		uint32_t numThreads = 1;

	public:
		static EngineConfiguration* GetEngineConfiguration()
		{
			static EngineConfiguration* config = new EngineConfiguration();
			return config;
		}

		void SetNumThreads(uint32_t numThreads)
		{
			this->numThreads = numThreads;
		}

		uint32_t GetNumThreads() const
		{
			return std::max(static_cast<uint32_t>(1), numThreads);
		}
	};
}