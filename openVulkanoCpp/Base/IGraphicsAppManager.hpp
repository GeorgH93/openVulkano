#pragma once
#include <cstdint>
#include "PlatformEnums.hpp"

namespace openVulkanoCpp
{
	class IWindow;
	class IGraphicsApp;
	class IRenderer;

	class IGraphicsAppManager
	{
	public:
		virtual ~IGraphicsAppManager() = default;

		virtual RenderAPI::RenderApi GetRenderApi() const = 0;
		virtual IGraphicsApp* GetGraphicsApp() const = 0;
		virtual IRenderer* GetRenderer() const = 0;
		virtual bool IsRunning() const = 0;
		virtual bool IsPaused() const = 0;
		virtual void Stop() = 0;
		virtual void Run() = 0;
		virtual void Pause() = 0;
		virtual void Resume() = 0;

		virtual float GetAvgFrameTime() const = 0;
		virtual float GetAvgFps() const = 0;
		virtual uint64_t GetFrameCount() const = 0;
	};
}
