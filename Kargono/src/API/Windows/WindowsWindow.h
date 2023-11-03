#pragma once

#include "Kargono/Core/Window.h"

#include "GLFW/glfw3.h"


namespace Kargono {

	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowProps& props);
		virtual ~WindowsWindow();

		void OnUpdate() override;

		virtual uint32_t GetWidth() const override { return m_Data.Width; }
		virtual uint32_t GetHeight() const override { return m_Data.Height; }

		void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
		void SetVSync(bool enabled) override;
		bool IsVSync() const override;
		virtual void SetFullscreen() override;
		virtual void DisableFullscreen() override;

		virtual void SetMouseCursorVisible(bool choice) override;

		virtual void* GetNativeWindow() const override { return m_Window; }
	private:
		virtual void Init(const WindowProps& props);
		void SwapBuffers();
		virtual void Shutdown();
	private:
		GLFWwindow* m_Window;

		struct WindowData
		{
			std::string Title;
			uint32_t Width, Height;
			bool VSync;
			uint8_t VersionMajor = 4, VersionMinor = 5;

			EventCallbackFn EventCallback;

		};
		WindowData m_Data;

	};
}

