#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Events/Event.h"
#include "Kargono/Math/Math.h"

#include <filesystem>


namespace Kargono 
{
	struct WindowProps 
	{
		std::string Title;
		uint32_t Width;
		uint32_t Height;

		WindowProps()
			: Title{ "No Title" }, Width{ 0 }, Height{0} {}

		WindowProps(const std::string& title,
			uint32_t width,
			uint32_t height)
			: Title(title), Width(width), Height(height)
		{}
	};

	class Window
	{
	public:
		virtual ~Window() = default;

		virtual void OnUpdate() = 0;

		virtual void Init(const std::filesystem::path& logoPath = "resources/icons/app_logo.png") = 0;
		
		virtual void Init(const Kargono::WindowProps& props, const std::filesystem::path& logoPath = "resources/icons/app_logo.png") = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		virtual uint32_t GetViewportWidth() const = 0;
		virtual uint32_t GetViewportHeight() const = 0;

		virtual void SetViewportWidth(uint32_t width) = 0;
		virtual void SetViewportHeight(uint32_t height) = 0;

		virtual void SetEventCallback(const Events::EventCallbackFn& callback) = 0;
		virtual Events::EventCallbackFn& GetEventCallback() = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;

		virtual void SetFullscreen(bool enabled) = 0;
		virtual void SetResizable(bool resizable) = 0;
		virtual void CenterWindow() = 0;
		virtual Math::vec2 GetMonitorDimensions() = 0;
		virtual void ResizeWindow(Math::vec2 newWindowSize) = 0;
		virtual void SetVisible(bool visible) = 0;

		virtual void SetMouseCursorVisible(bool choice) = 0;

		virtual void* GetNativeWindow() const = 0;

		static Scope<Window> Create(const WindowProps& props = WindowProps());

	};
}
