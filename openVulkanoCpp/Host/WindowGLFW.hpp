#pragma once
#include <GLFW/glfw3.h>
#include "../Base/UI/BaseWindow.hpp"
#include "../Base/Logger.hpp"

namespace openVulkanoCpp
{
	class WindowGLFW : public BaseWindow, virtual public IVulkanWindow, virtual public IOpenGlWindow
	{
	private:
		GLFWwindow* window = nullptr;
		uint32_t width = 1280, height = 720;
		std::string title = "Window Title";
		WindowMode windowMode = WINDOWED;
		IWindowHandler* handler = nullptr;

	public:
		WindowGLFW() = default;
		virtual ~WindowGLFW() { if (window != nullptr) Close(); }

	protected:
		void Create()
		{
			window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
			if(!window) return;
			glfwSetWindowUserPointer(window, this);
			RegisterCallbacks();
		}

		void RegisterCallbacks() const
		{
			glfwSetErrorCallback(ErrorCallback);

			glfwSetDropCallback(window, DropCallback);
			glfwSetFramebufferSizeCallback(window, ResizeCallback);
			glfwSetWindowFocusCallback(window, FocusCallback);
			glfwSetWindowRefreshCallback(window, RefreshCallback);
			glfwSetWindowIconifyCallback(window, MinimizeCallback);
			glfwSetWindowPosCallback(window, WindowMoveCallback);
			glfwSetWindowCloseCallback(window, CloseCallback);

			// Input Callbacks
			glfwSetKeyCallback(window, KeyboardCallback);
			glfwSetMouseButtonCallback(window, MouseButtonCallback);
			glfwSetCursorPosCallback(window, MouseMoveCallback);
			glfwSetScrollCallback(window, MouseScrollCallback);
		}

		static GLFWmonitor* GetPrimaryMonitor()
		{
			return glfwGetPrimaryMonitor();
		}

		static std::vector<GLFWmonitor*> GetMonitors()
		{
			int count;
			GLFWmonitor** monitorsArray = glfwGetMonitors(&count);
			std::vector<GLFWmonitor*> monitors;
			monitors.reserve(count);
			for (int i = 0; i < count; i++)
			{
				monitors[i] = monitorsArray[i];
			}
			return monitors;
		}

	public: // IWindow implementation
		void Init(RenderAPI::RenderApi renderApi) override
		{
			if (!glfwInit()) throw WindowInitFailedException("Failed to initialize glfw");
			if(renderApi == RenderAPI::VULKAN) glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			Create();
			if(!window)
			{
				glfwTerminate();
				throw WindowInitFailedException("Failed to initialize window");
			}
			if (renderApi != RenderAPI::VULKAN) MakeCurrentThread();
			Logger::WINDOW->info("GLFW Window created (id: {0})", GetWindowId());
		}

		void Close() override
		{
			glfwDestroyWindow(window);
			window = nullptr;
			glfwTerminate();
			Logger::WINDOW->info("GLFW Window destroyed (id: {0})", GetWindowId());
		}

		void Present() const override
		{
			glfwSwapBuffers(window);
		}

		void Show() override
		{
			glfwShowWindow(window);
		}

		void Hide() override
		{
			glfwHideWindow(window);
		}

		void Tick() override
		{
			glfwPollEvents();
		}

		void SetTitle(const std::string& title) override
		{
			this->title = title;
			glfwSetWindowTitle(window, title.c_str());
		}

		const std::string& GetTitle() override
		{
			return title;
		}

		void SetSize(uint32_t width, uint32_t height) override
		{
			if (!window)
			{
				this->width = width;
				this->height = height;
			}
			else
			{
				glfwSetWindowSize(window, width, height);
			}
		}

		void SetPosition(int posX, int posY) override
		{
			glfwSetWindowPos(window, posX, posY);
		}

		void SetSizeLimits(int minWidth, int minHeight, int maxWidth, int maxHeight) override
		{
			minWidth = (minWidth < 0) ? GLFW_DONT_CARE : minWidth;
			minHeight = (minHeight < 0) ? GLFW_DONT_CARE : minHeight;
			maxWidth = (maxWidth < 0) ? GLFW_DONT_CARE : maxWidth;
			maxHeight = (maxHeight < 0) ? GLFW_DONT_CARE : maxHeight;
			glfwSetWindowSizeLimits(window, minWidth, minHeight, maxWidth, maxHeight);
		}

		void MakeCurrentThread() override
		{
			glfwMakeContextCurrent(window);
		}

		void SetWindowMode(WindowMode windowMode) override
		{
			if(windowMode == this->windowMode) return; //Nothing change here 
			//TODO
			this->windowMode = windowMode;
		}

		void SetWindowHandler(IWindowHandler* handler) override
		{
			this->handler = handler;
		}

		IVulkanWindow* GetVulkanWindow() override
		{
			return this;
		}

		IOpenGlWindow* GetOpenGlWindow() override
		{
			return this;
		}

		// Status getter
		WindowMode GetWindowMode() override
		{
			return windowMode;
		}

		void GetSize(int* width, int* height) override
		{
			glfwGetWindowSize(window, width, height);
		}

		void GetPosition(int* x, int* y) override
		{
			glfwGetWindowPos(window, x, y);
		}

		IWindowHandler* GetWindowHandler() override
		{
			return handler;
		}

		//IVulkanWindow stuff
		vk::SurfaceKHR CreateSurface(const vk::Instance& instance, const vk::AllocationCallbacks* pAllocator) override
		{
			VkSurfaceKHR rawSurface;
			const auto result = static_cast<vk::Result>(glfwCreateWindowSurface(static_cast<VkInstance>(instance), window, reinterpret_cast<const VkAllocationCallbacks*>(pAllocator), &rawSurface));
			return createResultValue(result, rawSurface, "vk::CommandBuffer::begin");
		}

		std::vector<std::string> GetRequiredInstanceExtensions() override
		{
			return GetVulkanRequiredInstanceExtensions();
		}

	public: // Window events
		void OnResize(const uint32_t newWidth, const uint32_t newHeight)
		{
			Logger::WINDOW->debug("Window (id: {0}) resized (width: {1}, height: {2})", GetWindowId(), newWidth, newHeight);
			handler->OnWindowResize(this, newWidth, newHeight);
		}

		void OnMinimize()
		{
			Logger::WINDOW->debug("Window (id: {0}) minimized", GetWindowId());
			handler->OnWindowMinimize(this);
		}

		void OnRestore()
		{
			Logger::WINDOW->debug("Window (id: {0}) restored", GetWindowId());
			handler->OnWindowRestore(this);
		}

		void OnFocusLost()
		{
			Logger::WINDOW->debug("Window (id: {0}) focus lost", GetWindowId());
			handler->OnWindowFocusLost(this);
		}

		void OnFocusGained()
		{
			Logger::WINDOW->debug("Window (id: {0}) focus gained", GetWindowId());
			handler->OnWindowFocusGained(this);
		}

		void OnMove(const int posX, const int posY)
		{
			Logger::WINDOW->debug("Window (id: {0}) moved (x: {1}, y: {2})", GetWindowId(), posX, posY);
			handler->OnWindowMove(this, posX, posY);
		}
		
		void OnClose()
		{
			Logger::WINDOW->debug("Window (id: {0}) closed", GetWindowId());
			handler->OnWindowClose(this);
		}

	public: // Input events TODO
		virtual void OnKeyPressed(int key, int mods) {}
		virtual void OnKeyReleased(int key, int mods) {}
		virtual void OnMousePressed(int button, int mods) {}
		virtual void OnMouseReleased(int button, int mods) {}
		virtual void OnMouseMoved(double posX, double posY) {}
		virtual void OnMouseScrolled(double delta) {}

	protected:
		virtual void OnKeyEvent(int key, int scanCode, int action, int mods)
		{
			switch (action)
			{
			case GLFW_PRESS: OnKeyPressed(key, mods); break;
			case GLFW_RELEASE: OnKeyReleased(key, mods); break;
			default: break;
			}
		}

		virtual void OnMouseButtonEvent(int button, int action, int mods)
		{
			switch (action)
			{
			case GLFW_PRESS: OnMousePressed(button, mods); break;
			case GLFW_RELEASE: OnMouseReleased(button, mods); break;
			default: break;
			}
		}

	private: // Callbacks
		static WindowGLFW* GetWindow(GLFWwindow* window)
		{
			return static_cast<WindowGLFW*>(glfwGetWindowUserPointer(window));
		}

		static void KeyboardCallback(GLFWwindow* window, int key, int scanCode, int action, int mods)
		{
			GetWindow(window)->OnKeyEvent(key, scanCode, action, mods);
		}

		static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
		{
			GetWindow(window)->OnMouseButtonEvent(button, action, mods);
		}

		static void MouseMoveCallback(GLFWwindow* window, double posX, double posY)
		{
			GetWindow(window)->OnMouseMoved(posX, posY);
		}

		static void MouseScrollCallback(GLFWwindow* window, double xOffset, double yOffset)
		{
			GetWindow(window)->OnMouseScrolled(yOffset);
		}

		static void ResizeCallback(GLFWwindow* window, int width, int height)
		{
			GetWindow(window)->OnResize(width, height);
		}

		static void FocusCallback(GLFWwindow* window, const int focused)
		{
			if (focused == GLFW_TRUE)
				GetWindow(window)->OnFocusGained();
			else
				GetWindow(window)->OnFocusLost();
		}

		static void MinimizeCallback(GLFWwindow* window, const int minimized)
		{
			if(minimized == GLFW_TRUE)
				GetWindow(window)->OnMinimize();
			else
				GetWindow(window)->OnRestore();
		}

		static void RefreshCallback(GLFWwindow* window)
		{
			//TODO is there really anything to do? or is it ok if the window is only redrawn on the next frame?
		}

		static void WindowMoveCallback(GLFWwindow* window, const int posX, const int posY)
		{
			GetWindow(window)->OnMove(posX, posY);
		}

		static void CloseCallback(GLFWwindow* window)
		{
			GetWindow(window)->OnClose();
		}

		static void DropCallback(GLFWwindow* window, const int count, const char** paths)
		{
			//TODO something useful
		}

		static void ErrorCallback(const int error, const char* description)
		{
			Logger::WINDOW->error("GLFW error (e{0}): {1}", error, description);
		}




	public:
		static std::vector<std::string> GetVulkanRequiredInstanceExtensions()
		{
			std::vector<std::string> result;
			uint32_t count = 0;
			const char** names = glfwGetRequiredInstanceExtensions(&count);
			if (names && count)
			{
				for (uint32_t i = 0; i < count; ++i)
				{
					result.emplace_back(names[i]);
				}
			}
			return result;
		}


	};
}
