#pragma once

#include "Kargono/Window.h"
#include "Kargono/Renderer/GraphicsContext.h"
#include "GLFW/glfw3.h"


namespace Kargono {

	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowProps& props);
		virtual ~WindowsWindow();

		void OnUpdate() override;

/// @brief Returns the width of the window
		inline unsigned int GetWidth() const override { return m_Data.Width; }
/// @brief Returns the height of the window
		inline unsigned int GetHeight() const override { return m_Data.Height; }

/// @brief Sets the event callback function for the window
		inline void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
		void SetVSync(bool enabled) override;
		bool IsVSync() const override;

/// @brief Returns the native window handle
		inline virtual void* GetNativeWindow() const override { return m_Window; }
	private:
		virtual void Init(const WindowProps& props);
		virtual void Shutdown();
	private:
		GLFWwindow* m_Window;
		GraphicsContext* m_Context;

		struct WindowData
		{
			std::string Title;
			unsigned int Width, Height;
			bool VSync;

			EventCallbackFn EventCallback;

		};
		WindowData m_Data;

	};
}

