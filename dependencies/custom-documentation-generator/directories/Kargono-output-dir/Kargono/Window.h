/// @brief Ensures that the header file is only included once during compilation
#pragma once
/// @brief Included precompiled header file for optimization
#include "Kargono/kgpch.h"
/// @brief Included Core.h file
#include "Kargono/Core.h"
/// @brief Included Event.h file
#include "Kargono/Events/Event.h"

/// @namespace Kargono
namespace Kargono 
{
/// @brief Structure for defining the properties of a window
	struct WindowProps 
	{
/// @brief Title of the window
		std::string Title;
/// @brief Width of the window
		unsigned int Width;
/// @brief Height of the window
		unsigned int Height;

		WindowProps(const std::string& title = "Kargono Engine",
			unsigned int width = 1280,
			unsigned int height = 720)
			: Title(title), Width(width), Height(height)
		{}
	};

	class KG_API Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;
/// @brief Virtual destructor for the Window class
		virtual ~Window() {}

/// @brief Pure virtual function for updating the window
		virtual void OnUpdate() = 0;

/// @brief Pure virtual function for getting the width of the window
		virtual unsigned int GetWidth() const = 0;
/// @brief Pure virtual function for getting the height of the window
		virtual unsigned int GetHeight() const = 0;

/// @brief Pure virtual function for setting the event callback function
		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
/// @brief Pure virtual function for setting the vertical synchronization
		virtual void SetVSync(bool enabled) = 0;
/// @brief Pure virtual function for checking if vertical synchronization is enabled
		virtual bool IsVSync() const = 0;

/// @brief Pure virtual function for getting the native window
		virtual void* GetNativeWindow() const = 0;

/// @brief Static function for creating a window
		static Window* Create(const WindowProps& props = WindowProps());

	};
}