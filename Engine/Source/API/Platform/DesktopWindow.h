#pragma once

#include "Kargono/Core/Window.h"
#include "Kargono/Math/Math.h"

#include "API/Platform/GlfwAPI.h"

#if defined(KG_PLATFORM_WINDOWS) || defined(KG_PLATFORM_LINUX)

namespace API::Platform
{
	//==============================
	// DesktopWindow Structs
	//==============================
	// This struct holds data that describes the GLFW window. The member variable m_Data
	//		represents that information. The title is the name presented on the window,
	//		width/height are the dimensions, VSync indicates if VSync is currently enabled,
	//		Versions simply state what version of OpenGL we are using, and EventCallback is
	//		the function that is called when an event needs to be handled. Currently the
	//		EventCallback is always connected to the Application function OnEvent()
	struct DesktopWindowData
	{
		std::string Title;
		uint32_t Width, Height;
		uint32_t ViewportWidth, ViewportHeight;
		bool VSync;
		uint8_t VersionMajor = 4, VersionMinor = 5;
		Kargono::Events::EventCallbackFn EventCallback;
	};

	//============================================================
	// Windows(Platform) Window Class
	//============================================================
	// This class represents a wrapper around a GLFW window. The GLFW
	//		window is the actual windows container itself that uses
	//		the Windows API. The window contains the viewport of OpenGL
	//		which can have differing dimensions. The window also serves
	//		to receive input events such as Key Presses, Mouse Clicks,
	//		and Window Resize Events. These events are registered with
	//		callback functions in the Init() function. The events are
	//		handled later in the engine through the Application class.
	class DesktopWindow : public Kargono::Window
	{

	public:
		//==============================
		// Constructors and Destructors
		//==============================
		// This constructor simply calls the Init() lifetime function.
		//		The Init Description will be more useful.
		DesktopWindow(const Kargono::WindowProps& props);
		// This destructor simply calls the Shutdown() lifetime function.
		virtual ~DesktopWindow() override;

		//==============================
		// Lifecycle Functions
		//==============================
	public:
		// The main focus of this function is to initialize the underlying GLFW window.
		//		The ancillary uses of this function include:
		//		1. It initializes the m_Window* with a reference to the actual GLFW window
		//		2. It fills m_Data with relevant window information
		//		3. It initializes GLFW and GLAD (GLAD retrieves the OpenGL function pointers
		//		from the GPU drivers so they are available to use)
		//		4. It registers function callbacks with GLFW so events such as keyboard presses
		//		or window resize events can be handles by the engine.
		//		5. It initializes the logo used by the application.
		virtual void Init(const std::filesystem::path& logoPath = "Resources/icons/app_logo.png") override;
		// This function allows the window properties to be changed a second time before being finally
		//		initialized!
		virtual void Init(const Kargono::WindowProps& props, const std::filesystem::path& logoPath = "Resources/icons/app_logo.png") override;
		// This function simply closes the GLFW window associated with its instance. This window
		//		should be a singleton. If all GLFW windows are closed successfully, GLFW will terminate.
		virtual void Shutdown();
	public:
		// This function simply polls window events and swaps the framebuffer using SwapBuffers(). Polling Window events
		//		essentially handles any events thrown by GLFW. The events are thrown asynchronously, however,
		//		they are handled when this function is called. 
		void OnUpdate() override;
	private:
		// This function swaps the framebuffer. Swapping the framebuffer reveals what has been
		//		drawn on the framebuffer in one step. Not using this can cause many issues such as screen tearing.
		void SwapBuffers();
	public:
		//==============================
		// Update GLFW Window Settings and GLFW Functionality
		//==============================
		// The following functions make calls to the underlying GLFW window to modify specific window properties.
		// This function enables or disables VSync. VSync requires the OpenGL context to sync the frame rate
		//		of the GPU with the refresh rate of the monitor. This can have many different advantages and
		//		disadvantages.
		void SetVSync(bool enabled) override;
		// This function enables or disabled fullscreen for this GLFW window
		virtual void SetFullscreen(bool enabled) override;
		// This functions enables or disables the ability for the user to resize the GLFW window
		virtual void SetResizable(bool resizable) override;
		// This function simply places the GLFW window in the middle of the screen
		virtual void CenterWindow() override;
		// This function queries GLFW for the dimensions of the current monitor
		virtual Kargono::Math::vec2 GetMonitorDimensions() override;
		// This function resizes the GLFW window to the specified dimensions.
		virtual void ResizeWindow(Kargono::Math::vec2 newWindowSize) override;
		virtual void ToggleMaximized() override;
		// This function enables or disables the mouse cursor over the GLFW window
		virtual void SetMouseCursorVisible(bool choice) override;
		virtual void SetVisible(bool visible) override;
		bool IsVSync() const override { return m_Data.VSync; }
		void SetEventCallback(const Kargono::Events::EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
		virtual Kargono::Events::EventCallbackFn& GetEventCallback() override { return m_Data.EventCallback; }

		//==============================
		// Getters/Setters
		//==============================
	public:
		virtual uint32_t GetWidth() const override { return m_Data.Width; }
		virtual uint32_t GetHeight() const override { return m_Data.Height; }

		virtual uint32_t GetViewportWidth() const override { return m_Data.ViewportWidth; }
		virtual uint32_t GetViewportHeight() const override { return m_Data.ViewportHeight; }

		virtual void SetViewportWidth(uint32_t width) override { m_Data.ViewportWidth = width; }
		virtual void SetViewportHeight(uint32_t height) override { m_Data.ViewportHeight = height; }

		virtual void* GetNativeWindow() const override { return m_Window; }
	private:
		// m_Window holds the reference to the underlying GLFW window that is represented by this class.
		GLFWwindow* m_Window;
		// m_Data holds specification data for the window such as its size.
		DesktopWindowData m_Data;

	};
}

#endif
