#pragma once
#include "Kargono/kgpch.h"
#include "Kargono/Core.h"
#include "Kargono/Events/Event.h"

namespace Kargono 
{
	struct WindowProps 
	{
		std::string Title;
		unsigned int Width;
		unsigned int Height;

/// @brief Constructor for the WindowProps structure
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
/// @brief Destructor for the Window class
		virtual ~Window() {}

/// @brief Pure virtual function for updating the window
		virtual void OnUpdate() = 0;

/// @brief Pure virtual function for getting the width of the window
		virtual unsigned int GetWidth() const = 0;
/// @brief Pure virtual function for getting the height of the window
		virtual unsigned int GetHeight() const = 0;

/// @brief Pure virtual function for setting the event callback function for the window
		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
/// @brief Pure virtual function for setting vertical synchronization for the window
		virtual void SetVSync(bool enabled) = 0;
/// @brief Pure virtual function for checking if vertical synchronization is enabled for the window
		virtual bool IsVSync() const = 0;

/// @brief Static function for creating a window with specified properties in the Kargono Engine
		static Window* Create(const WindowProps& props = WindowProps());

	};
}