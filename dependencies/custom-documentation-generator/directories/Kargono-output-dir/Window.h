#pragma once
#include "Kargono/kgpch.h"
#include "Kargono/Core.h"
#include "Kargono/Events/Event.h"

namespace Kargono 
{
/// @brief Structure defining properties of a window in the Kargono engine
	struct WindowProps 
	{
		std::string Title;
		unsigned int Width;
		unsigned int Height;

		WindowProps(const std::string& title = "Kargono Engine",
			unsigned int width = 1280,
			unsigned int height = 720)
			: Title(title), Width(width), Height(height)
		{}
	};

/// @class Window
/// @brief Base class for windows in the Kargono engine
	class KG_API Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;
		virtual ~Window() {}

/// @brief Virtual function for updating the window
		virtual void OnUpdate() = 0;

/// @brief Virtual function to get the width of the window
/// @return The width of the window
		virtual unsigned int GetWidth() const = 0;
/// @brief Virtual function to get the height of the window
/// @return The height of the window
		virtual unsigned int GetHeight() const = 0;

/// @brief Virtual function to set the event callback function
/// @param callback The event callback function
		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
/// @brief Virtual function to set the vertical synchronization
/// @param enabled True to enable vertical synchronization, false otherwise
		virtual void SetVSync(bool enabled) = 0;
/// @brief Virtual function to check if vertical synchronization is enabled
/// @return True if vertical synchronization is enabled, false otherwise
		virtual bool IsVSync() const = 0;

/// @brief Static function to create a window
/// @param props The properties of the window
/// @return A pointer to the created window
		static Window* Create(const WindowProps& props = WindowProps());

	};
}