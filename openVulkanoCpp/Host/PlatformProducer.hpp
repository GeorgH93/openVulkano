#pragma once
#include <stdexcept>
#include "../Base/Logger.hpp"
#include "../Vulkan/Renderer.hpp"
#include "../Base/PlatformEnums.hpp"
#include "WindowGLFW.hpp"

namespace openVulkanoCpp
{
	/**
	 * \brief Helper class the produces all the platform depending classes
	 */
	class PlatformProducer
	{
	public:

		/**
		 * \brief Creates the renderer for the given render api
		 * \param renderApi The render api that should be used
		 * \return The created Renderer.
		 * \throws std::runtime_error if the render api is not supported
		 */
		static IRenderer* CreateRenderManager(RenderAPI::RenderApi renderApi)
		{
			switch (renderApi)
			{
			case RenderAPI::VULKAN: return new Vulkan::Renderer();
			default:
				Logger::RENDER->error("Unsupported render api requested! Requested %d", static_cast<int>(renderApi));
				throw std::runtime_error("Unsupported render api requested!");
			}
		}

		/**
		 * \brief Creates a window that fits best for the current environment
		 * \param renderApi The render api that should be used when searching for the best suited window
		 * \return The created window. nullptr if no window is supported on the current platform
		 * \throws std::runtime_error if the render api is not supported
		 */
		static IWindow* CreateBestWindow(RenderAPI::RenderApi renderApi)
		{ //TODO add more windows to chose from
			switch(renderApi)
			{
			case RenderAPI::VULKAN: return new WindowGLFW();
			default:
				Logger::RENDER->error("Unsupported render api requested! Requested %d", static_cast<int>(renderApi));
				throw std::runtime_error("Unsupported render api requested!");
			}
		}
	};
}
