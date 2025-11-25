#pragma once

#include "Kargono.h"

#include "Modules/Rendering/Framebuffer.h"
#include "Modules/Memory/HeapAlloc.h"

#include <filesystem>

namespace Kargono
{
	class RuntimeApp : public Application
	{
	public:
		//==========================
		// Constructor/Destructor
		//==========================
		RuntimeApp();
		RuntimeApp(std::filesystem::path projectPath, bool headless = false);
		virtual ~RuntimeApp() = default;
	public:
		//==========================
		// LifeCycle Functions
		//==========================
		[[nodiscard]] virtual bool Init() override;
		[[nodiscard]] virtual bool Terminate() override;
		void InitializeFrameBuffer();
	public:
		//==========================
		// On Event Functions
		//==========================
		virtual void OnUpdate(Timestep ts) override;
		virtual bool OnApplicationEvent(Events::Event* event) override;
		virtual bool OnNetworkEvent(Events::Event* event) override;
		virtual bool OnInputEvent(Events::Event* event) override;
		virtual bool OnPhysicsEvent(Events::Event* event) override;
	private:
		bool OnKeyTyped(Events::KeyTypedEvent event);
		bool OnKeyPressed(Events::KeyPressedEvent event);
		bool OnMousePressed(Events::MouseButtonPressedEvent event);
		bool OnMouseButtonReleased(const Events::MouseButtonReleasedEvent& event);
		void OnUpdateRuntime(Timestep ts);
		bool OnWindowResize(Events::WindowResizeEvent event);
		bool OnApplicationClose(Events::ApplicationCloseEvent event);
		bool OnApplicationResize(Events::ApplicationResizeEvent event);
		bool OnPhysicsCollisionStart(Events::PhysicsCollisionStart event);
		bool OnPhysicsCollisionEnd(Events::PhysicsCollisionEnd event);
		bool OnUpdateUserCount(Events::ReceiveOnlineUsers event);
		bool OnApproveJoinSession(Events::ApproveJoinSession event);
		bool OnUpdateSessionUserSlot(Events::UpdateSessionUserSlot event);
		bool OnUserLeftSession(Events::UserLeftSession event);
		bool OnCurrentSessionInit(Events::CurrentSessionInit event);
		bool OnConnectionTerminated(Events::ConnectionTerminated event);
		bool OnStartSession(Events::StartSession event);
		bool OnSessionReadyCheckConfirm(Events::SessionReadyCheckConfirm event);
		bool OnReceiveSignal(Events::ReceiveSignal event);
		void HandleUIMouseHovering();

	private:
		//==========================
		// Query Application State
		//==========================
		virtual Math::vec2 GetMouseViewportPosition() override;
		virtual ViewportData* GetViewportData() override;
	private:
		//==========================
		// Project Management
		//==========================
		bool OpenProject();
		void OpenProject(const std::filesystem::path& path);
		void OpenScene(Assets::AssetHandle sceneHandle);
		void OnPlay();
		void OnStop();
	private:
		//==========================
		// Internal Fields
		//==========================
		Memory::HeapAllocator m_HeapAllocator;
		Ref<Rendering::Framebuffer> m_ViewportFramebuffer;
		int32_t m_HoveredWidgetID{ Kargono::RuntimeUI::k_InvalidWidgetID };
		bool m_Headless{ false };
		std::filesystem::path m_ProjectPath;
	};

}
