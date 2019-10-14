#pragma once
#include <string>
#include <stdexcept>
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>
#include "../PlatformEnums.hpp"
#include "../ITickable.hpp"
#include "../ICloseable.hpp"

namespace openVulkanoCpp
{
	enum WindowMode
	{
		WINDOWED, BORDERLESS, FULLSCREEN, BORDERLESS_FULLSCREEN
	};

	class IWindowHandler;
	class IVulkanWindow;
	class IOpenGlWindow;

	class IWindow : public ITickable, public ICloseable
	{
	public:
		virtual ~IWindow() = default;

		virtual void Init(RenderAPI::RenderApi renderApi) = 0;

		virtual const std::string& GetTitle() = 0;
		virtual void SetTitle(const std::string& title) = 0;

		virtual WindowMode GetWindowMode() = 0;
		virtual void SetWindowMode(WindowMode) = 0;
		virtual void SetFullscreen() { SetWindowMode(FULLSCREEN); }
		virtual void SetWindowed() { SetWindowMode(WINDOWED); }

		virtual uint32_t GetWidth() = 0;
		virtual uint32_t GetHeight() = 0;
		virtual void GetSize(int* width, int* height) = 0;
		virtual void GetSize(uint32_t* width, uint32_t* height) = 0;
		virtual glm::ivec2 GetSize() = 0;
		virtual void SetSize(uint32_t width, uint32_t height) = 0;
		virtual void SetSize(glm::ivec2 size) { SetSize(size.x, size.y); }
		virtual void SetSizeLimits(int minWidth, int minHeight, int maxWidth, int maxHeight) = 0;

		virtual int GetPositionX() = 0;
		virtual int GetPositionY() = 0;
		virtual void GetPosition(int* x, int* y) = 0;
		virtual glm::ivec2 GetPosition() = 0;
		virtual void SetPosition(int posX, int posY) = 0;
		virtual void SetPosition(glm::ivec2 pos) = 0;

		virtual void Show() = 0;
		virtual void Hide() = 0;
		virtual void Show(bool show) = 0;

		virtual IWindowHandler* GetWindowHandler() = 0;
		virtual void SetWindowHandler(IWindowHandler* handler) = 0;

		/**
		 * \brief Gets the vulkan window implementation of the window.
		 * \return The IVulkanWindow reference of the window. nullptr if the current Window dose not implement IVulkanWindow
		 */
		virtual IVulkanWindow* GetVulkanWindow() = 0;
		virtual IOpenGlWindow* GetOpenGlWindow() = 0;

		virtual int GetWindowId() const = 0;

	protected:
		static int CreateWindowId()
		{
			static int id = 0;
			return id++;
		}
	};

	class IVulkanWindow : virtual public IWindow
	{
	public:
		virtual ~IVulkanWindow() = default;

		virtual vk::SurfaceKHR CreateSurface(const vk::Instance& instance, const vk::AllocationCallbacks* pAllocator = nullptr) = 0;
		virtual std::vector<std::string> GetRequiredInstanceExtensions() = 0;
	};

	class IOpenGlWindow : virtual public IWindow
	{
	public:
		virtual ~IOpenGlWindow() = default;

		virtual void MakeCurrentThread() = 0;
		virtual void Present() const = 0;
	};

	class IWindowHandler
	{
	public:
		virtual ~IWindowHandler() = default;

		virtual void OnWindowMinimize(IWindow* window) = 0;
		virtual void OnWindowRestore(IWindow* window) = 0;
		virtual void OnWindowFocusLost(IWindow* window) = 0;
		virtual void OnWindowFocusGained(IWindow* window) = 0;
		virtual void OnWindowMove(IWindow* window, int posX, int posY) = 0;
		virtual void OnWindowResize(IWindow* window, uint32_t newWidth, uint32_t newHeight) = 0;
		virtual void OnWindowClose(IWindow* window) = 0;
	};

	class WindowInitFailedException : public std::runtime_error
	{
	public:
		WindowInitFailedException(char const* const message) : runtime_error(message) {}
	};
}
