#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Events/Event.h"
#include "Kargono/Math/Math.h"

#include <filesystem>


namespace Kargono 
{
	//==============================
	// Window Properties Struct
	//==============================
	struct WindowProps 
	{
		std::string Title {"No Title"};
		uint32_t Width{ 0 };
		uint32_t Height{ 0 };

		WindowProps() {}

		WindowProps(const std::string& title, uint32_t width, uint32_t height)
			: Title(title), Width(width), Height(height) {}
	};

	//==============================
	// Viewport Data Struct
	//==============================
	struct ViewportData
	{
		uint32_t m_Width{ 0 };
		uint32_t m_Height{ 0 };
	};

	enum class CursorIconType : uint16_t
	{
		None = 0,
		Standard,
		IBeam,
		Hand
	};

	//==============================
	// Window Class
	//==============================
	class Window
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		virtual ~Window() = default;

		//==============================
		// LifeCycle Functions
		//==============================
		virtual void Init(const std::filesystem::path& logoPath = "Resources/icons/app_logo.png") = 0;
		virtual void Init(const Kargono::WindowProps& props, const std::filesystem::path& logoPath = "Resources/icons/app_logo.png") = 0;
		
		//==============================
		// OnEvent
		//==============================
		virtual void OnUpdate() = 0;
		
		//==============================
		// Window Dimensions
		//==============================
		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		virtual ViewportData& GetActiveViewport() const = 0;
		virtual void SetActiveViewport(ViewportData* viewportData) = 0;

		virtual Math::uvec2 GetMonitorDimensions() = 0;
		virtual void ResizeWindow(Math::vec2 newWindowSize) = 0;
		//==============================
		// Manage Event Callback
		//==============================
		virtual void SetEventCallback(const Events::EventCallbackFn& callback) = 0;
		virtual Events::EventCallbackFn& GetEventCallback() = 0;

		//==============================
		// Window Toggle Options
		//==============================
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;
		virtual void SetFullscreen(bool enabled) = 0;
		virtual void SetResizable(bool resizable) = 0;
		virtual void SetVisible(bool visible) = 0;
		virtual void SetMouseCursorVisible(bool choice) = 0;
		virtual void SetMouseCursorIcon(CursorIconType iconType) = 0;
		virtual void CenterWindow() = 0;
		virtual void ToggleMaximized() = 0;

		//==============================
		// Underlying Window Management
		//==============================
		virtual void* GetNativeWindow() const = 0;
		static Scope<Window> Create(const WindowProps& props = WindowProps());

	};
}
