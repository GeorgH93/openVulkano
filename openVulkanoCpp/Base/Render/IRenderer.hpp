#pragma once
#include <string>
#include "../ITickable.hpp"
#include "../ICloseable.hpp"
#include "../../Scene/Scene.hpp"

namespace openVulkanoCpp
{
	class IWindow;
	class IGraphicsAppManager;

	class IRenderer : virtual public ITickable, virtual public ICloseable
	{
	public:
		virtual ~IRenderer() = default;

		virtual void Init(IGraphicsAppManager* graphicsAppManager, IWindow* window) = 0;

		virtual std::string GetMainRenderDeviceName() = 0;
		virtual void Resize(uint32_t newWidth, uint32_t newHeight) = 0;

		virtual void SetScene(Scene::Scene* scene) = 0;
		virtual Scene::Scene* GetScene() = 0;
	};
}
