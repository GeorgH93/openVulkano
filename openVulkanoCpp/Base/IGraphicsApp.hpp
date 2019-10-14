#pragma once
#include <string>
#include "IInitable.hpp"
#include "ITickable.hpp"
#include "ICloseable.hpp"

namespace openVulkanoCpp
{
	class IGraphicsAppManager;

	class IGraphicsApp : public IInitable, public ITickable, public ICloseable
	{
	private:
		IGraphicsAppManager* manager = nullptr;

	public:
		virtual ~IGraphicsApp() = default;

		IGraphicsAppManager* GetGraphicsAppManager() const { return manager; }
		void SetGraphicsAppManager(IGraphicsAppManager* manager) { this->manager = manager; }
		virtual std::string GetAppName() = 0;
		virtual std::string GetAppVersion() = 0;
		virtual int GetAppVersionAsInt() = 0;
	};
}
