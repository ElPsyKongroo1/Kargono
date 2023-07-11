
#pragma once


#include "Kargono/Window.h"

#include <GLFW/glfw3.h>


namespace Kargono {

	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowProps& props);
/// @brief Destructor for the WindowsWindow class
		virtual ~WindowsWindow();

		void OnUpdate() override;


		inline unsigned int GetWidth() const override { return m_Data.Width; }
		inline unsigned int GetHeight() const override { return m_Data.Height; }

		inline void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
/// @brief Function to set vertical sync for the window
		void SetVSync(bool enabled) override;
/// @brief Function to check if vertical sync is enabled for the window
		bool IsVSync() const override;
	private:
		virtual void Init(const WindowProps& props);
		virtual void Shutdown();
	private:
		GLFWwindow* m_Window;

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

