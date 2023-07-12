#pragma once

#include "Kargono/Window.h"
#include "GLFW/glfw3.h"

namespace Kargono {

	class WindowsWindow : public Window
	{
	public:
/// @brief Construct a new WindowsWindow object
/// @param props The properties of the window
		WindowsWindow(const WindowProps& props);
/// @brief Default destructor for the WindowsWindow class
		virtual ~WindowsWindow();

/// @brief Function to update the window
		void OnUpdate() override;

/// @brief Function to get the width of the window
/// @return The width of the window
		inline unsigned int GetWidth() const override { return m_Data.Width; }
/// @brief Function to get the height of the window
/// @return The height of the window
		inline unsigned int GetHeight() const override { return m_Data.Height; }

/// @brief Function to set the event callback function
/// @param callback The event callback function
		inline void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
/// @brief Function to set vertical synchronization
/// @param enabled Whether to enable or disable vertical synchronization
		void SetVSync(bool enabled) override;
/// @brief Function to check if vertical synchronization is enabled
/// @return True if vertical synchronization is enabled, false otherwise
		bool IsVSync() const override;
	private:
/// @brief Function to initialize the window
/// @param props The properties of the window
		virtual void Init(const WindowProps& props);
/// @brief Function to shut down the window
		virtual void Shutdown();
	private:
/// @brief Pointer to the GLFW window object
		GLFWwindow* m_Window;

		struct WindowData
		{
/// @brief Title of the window
			std::string Title;
/// @brief Width and height of the window
			unsigned int Width, Height;
/// @brief Flag indicating whether vertical synchronization is enabled or disabled
			bool VSync;

/// @brief Callback function for window events
			EventCallbackFn EventCallback;

		};
/// @brief Window data object
		WindowData m_Data;

	};
}

