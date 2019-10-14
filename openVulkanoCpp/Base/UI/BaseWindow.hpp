#pragma once
#include "IWindow.hpp"

namespace openVulkanoCpp
{
	class BaseWindow : virtual public IWindow
	{
		const int windowId;
	public:
		BaseWindow() : windowId(CreateWindowId()) {}
		virtual ~BaseWindow() = default;

		void GetSize(int* width, int* height) override = 0;

		void GetSize(uint32_t* width, uint32_t* height) override
		{
			int w, h;
			GetSize(&w, &h);
			*width = w;
			*height = h;
		}

		uint32_t GetWidth() override
		{
			uint32_t width, height;
			GetSize(&width, &height);
			return width;
		}

		uint32_t GetHeight() override
		{
			uint32_t width, height;
			GetSize(&width, &height);
			return height;
		}

		glm::ivec2 GetSize() override
		{
			glm::ivec2 size;
			this->GetSize(&size.x, &size.y);
			return size;
		}

		void SetSize(uint32_t width, uint32_t height) override = 0;

		void SetSize(glm::ivec2 size) override
		{
			SetSize(size.x, size.y);
		}

		void GetPosition(int* x, int* y) override = 0;

		int GetPositionX() override
		{
			int x, y;
			GetPosition(&x, &y);
			return x;
		}

		int GetPositionY() override
		{
			int x, y;
			GetPosition(&x, &y);
			return y;
		}


		glm::ivec2 GetPosition() override
		{
			glm::ivec2 position;
			GetPosition(&position.x, &position.y);
			return position;
		}

		void SetPosition(int posX, int posY) override = 0;

		void SetPosition(glm::ivec2 pos) override { SetPosition(pos.x, pos.y); }

		void Show() override = 0;
		void Hide() override = 0;

		void Show(const bool show) override { if (show) Show(); else Hide(); }

		IVulkanWindow* GetVulkanWindow() override
		{
			return nullptr;
		}

		IOpenGlWindow* GetOpenGlWindow() override
		{
			return nullptr;
		}

		int GetWindowId() const override
		{
			return windowId;
		}
	};
}
