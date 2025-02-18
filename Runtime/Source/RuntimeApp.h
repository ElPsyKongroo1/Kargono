#pragma once

#include "Kargono.h"

#include "Kargono/Rendering/Framebuffer.h"

#include <filesystem>

namespace Kargono
{
	//============================================================
	// Runtime Layer Class
	//============================================================
	// This class holds all of the runtime application logic. The concept of
	//		layers is in its infancy currently. This class holds the logic for
	//		all of the different systems that the runtime uses such as rendering,
	//		audio, and physics.
	class RuntimeApp : public Application
	{
	public:
		//==========================
		// Constructor/Destructor
		//==========================
		// Does nothing other than calling parent constructor
		RuntimeApp();
		RuntimeApp(std::filesystem::path projectPath, bool headless = false);
		virtual ~RuntimeApp() = default;

		//==========================
		// LifeCycle Functions
		//==========================
		// Starts Audio System and askes the user what project to open(TEMPORARY TODO).
		virtual void Init() override;
		// Closes Audio System and calls OnSceneStop()
		virtual void Terminate() override;

		void InitializeFrameBuffer();

		//==========================
		// On Event Functions
		//==========================
		// Calls Runtime Update for Physics, Scripting, and Rendering.
		virtual void OnUpdate(Timestep ts) override;
		// Currently does nothing!
		virtual bool OnApplicationEvent(Events::Event* event) override;
		virtual bool OnNetworkEvent(Events::Event* event) override;
		virtual bool OnInputEvent(Events::Event* event) override;
		virtual bool OnPhysicsEvent(Events::Event* event) override;
		
	private:
		// This function processes key pressed events
		bool OnKeyTyped(Events::KeyTypedEvent event);
		bool OnKeyPressed(Events::KeyPressedEvent event);
		// This function processes mouse pressed events
		bool OnMousePressed(Events::MouseButtonPressedEvent event);

		bool OnMouseButtonReleased(const Events::MouseButtonReleasedEvent& event);
		// This function renders the active scene, updates scripts, and updates the scene's physics.
		void OnUpdateRuntime(Timestep ts);
		// This function updates the scene cameras when the window is resized
		bool OnWindowResize(Events::WindowResizeEvent event);
		// This function closes the runtime application
		bool OnApplicationClose(Events::ApplicationCloseEvent event);

		bool OnApplicationResize(Events::ApplicationResizeEvent event);
		// This function responds to application collision events. Currently it plays a sound(Very Temporary).
		bool OnPhysicsCollisionStart(Events::PhysicsCollisionStart event);

		bool OnPhysicsCollisionEnd(Events::PhysicsCollisionEnd event);

		bool OnUpdateUserCount(Events::UpdateOnlineUsers event);

		bool OnApproveJoinSession(Events::ApproveJoinSession event);

		bool OnUpdateSessionUserSlot(Events::UpdateSessionUserSlot event);

		bool OnUserLeftSession(Events::UserLeftSession event);

		bool OnCurrentSessionInit(Events::CurrentSessionInit event);

		bool OnConnectionTerminated(Events::ConnectionTerminated event);

		bool OnStartSession(Events::StartSession event);

		bool OnSessionReadyCheckConfirm(Events::SessionReadyCheckConfirm event);

		bool OnReceiveSignal(Events::ReceiveSignal event);

		void HandleUIMouseHovering();

		//==========================
		// Query Application State
		//==========================
		virtual Math::vec2 GetMouseViewportPosition() override;
		virtual ViewportData* GetViewportData() override;

		//==========================
		// Project Management
		//==========================

		// This function opens a file dialog and calls the other OpenProject(path) function
		//		if the file dialog was successful.
		bool OpenProject();

		// Function opens project described by the path which includes opening the initial scene
		//		and loading the new app assembly into the scripting engine.
		void OpenProject(const std::filesystem::path& path);

		// This function opens a scene. This function is only currently called by the OpenProject function
		//		to open the initial scene.
		void OpenScene(Assets::AssetHandle sceneHandle);

		// Starts and stops the runtime. This includes Scripting,
		//		Physics, and supporting functionality.
		void OnPlay();
		void OnStop();


	private:
		Ref<Rendering::Framebuffer> m_ViewportFramebuffer;
		uint16_t m_HoveredWindowID{ Kargono::RuntimeUI::k_InvalidWindowID };
		uint16_t m_HoveredWidgetID{ Kargono::RuntimeUI::k_InvalidWidgetID };
		bool m_Headless{ false };
		std::filesystem::path m_ProjectPath;
	};

}
