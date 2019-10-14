#pragma once
#include <chrono>
#include <thread>
#include <string>
#include <stdexcept>
#include "../Base/IGraphicsAppManager.hpp"
#include "../Base/UI/IWindow.hpp"
#include "../Base/IGraphicsApp.hpp"
#include "../Base/PlatformEnums.hpp"
#include "../Base/Logger.hpp"
#include "../Base/Timer.hpp"
#include "../Base/Render/IRenderer.hpp"
#include "PlatformProducer.hpp"

namespace openVulkanoCpp
{
	/**
	 * \brief A simple GraphicsAppManager. It can only handle on window.
	 */
	class GraphicsAppManager : virtual public IGraphicsAppManager, virtual public IWindowHandler
	{
	private:
		IWindow* window;
		IGraphicsApp* app;
		IRenderer* renderer;
		RenderAPI::RenderApi renderApi;
		bool paused = false, running = false;
		float fpsTimer = 0, avgFps = 0, avgFrameTime = 0;
		uint64_t frameCount = 0, lastFrameCount = 0;
		Timer* frameTimer;
		std::string windowTitleFormat;

	public:
		explicit GraphicsAppManager(IGraphicsApp* app, RenderAPI::RenderApi renderApi = RenderAPI::VULKAN) : app(app), renderApi(renderApi)
		{
			if (renderApi >= RenderAPI::MAX_VALUE) throw std::runtime_error("Invalid RenderAPI");
			Logger::SetupLogger();
			if (!app)
			{
				const auto msg = "The app must not be null!";
				Logger::MANAGER->error(msg);
				throw std::runtime_error(msg);
			}
			window = PlatformProducer::CreateBestWindow(renderApi);
			renderer = PlatformProducer::CreateRenderManager(renderApi);
			app->SetGraphicsAppManager(this);
			window->SetWindowHandler(this);
			frameTimer = new Timer();
		}

		~GraphicsAppManager() override
		{
			delete renderer;
			delete window;
			delete frameTimer;
		}

	public: // Getter
		RenderAPI::RenderApi GetRenderApi() const override
		{
			return renderApi;
		}

		IGraphicsApp* GetGraphicsApp() const override
		{
			return app;
		}

		IRenderer* GetRenderer() const override
		{
			return renderer;
		}

		bool IsRunning() const override
		{
			return running;
		}

		bool IsPaused() const override
		{
			return paused;
		}

	public: // Setter
		void Stop() override
		{
			running = false;
			Logger::MANAGER->info("Graphics application stopped");
		}

		void Pause() override
		{
			paused = true;
			frameTimer->Stop();
			Logger::MANAGER->info("Graphics application paused");
		}

		void Resume() override
		{
			paused = false;
			frameTimer->Start();
			Logger::MANAGER->info("Graphics application resumed");
		}

	public:
		void Run() override
		{
			running = true;
			StartUp();
			frameTimer->Reset();
			Loop(); // Runs the rendering loop
			ShutDown();
		}

	private:
		void StartUp()
		{
			try
			{
				Logger::MANAGER->info("Initializing ...");
				app->Init();
				window->Init(renderApi);
				//TODO restore window settings if there are any set
				renderer->Init((IGraphicsAppManager*)this, window);
				windowTitleFormat = app->GetAppName() + " " + app->GetAppVersion() + " - " + renderer->GetMainRenderDeviceName() + " - {:.1f} fps ({:.1f} ms)";
				Logger::MANAGER->info("Initialized");
			}
			catch (std::exception& e)
			{
				Logger::MANAGER->error("Failed to initiate: {0}", e.what());
				running = false;
#ifdef DEBUG
				throw e;
#endif
			}
		}

		void Loop()
		{
			while (running)
			{
				window->Tick();
				if (paused)
				{ // The rendering is paused
					// No need to burn cpu time if the app is paused
					std::this_thread::sleep_for(std::chrono::milliseconds(50));
				}
				else
				{
					app->Tick();
					renderer->Tick();
					frameTimer->Tick();
					UpdateFps();
				}
			}
		}

		void ShutDown() const
		{
			Logger::MANAGER->info("Shutting down ...");
			renderer->Close();
			window->Close();
			app->Close();
			Logger::MANAGER->info("Shutdown complete");
		}

		void UpdateFps()
		{
			frameCount++;
			fpsTimer += frameTimer->GetTickSeconds();

			if(fpsTimer > 1.0f)
			{
				avgFps = static_cast<float>(frameCount - lastFrameCount) / fpsTimer;
				avgFrameTime = (1 / avgFps) * 1000.0f;
				lastFrameCount = frameCount;
				fpsTimer = 0;
				window->SetTitle(fmt::format(windowTitleFormat, avgFps, avgFrameTime));
			}
		}

	public: //FPS stuff
		uint64_t GetFrameCount() const override
		{
			return frameCount;
		}

		float GetAvgFrameTime() const override
		{
			return avgFrameTime;
		}

		float GetAvgFps() const override
		{
			return avgFps;
		}

	public: // Window Manager
		void OnWindowMinimize(IWindow* window) override
		{
			if (window != this->window) return;
			Pause();
		}

		void OnWindowRestore(IWindow* window) override
		{
			if (window != this->window) return;
			Resume();
		}

		void OnWindowFocusLost(IWindow* window) override {}
		void OnWindowFocusGained(IWindow* window) override {}
		void OnWindowMove(IWindow* window, int posX, int posY) override {} //TODO save window pos

		void OnWindowResize(IWindow* window, const uint32_t newWidth, const uint32_t newHeight) override
		{
			if(window != this->window) return;
			renderer->Resize(newWidth, newHeight);
		}

		void OnWindowClose(IWindow* window) override
		{
			if (window != this->window) return;
			Stop();
		}
	};
}
