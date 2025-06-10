#pragma once
#include "Modules/Events/KeyEvent.h"
#include "Modules/Assets/Asset.h"
#include "Modules/EditorUI/EditorUIInclude.h"
#include "Modules/RuntimeUI/RuntimeUIContext.h"
#include "Kargono/Core/Base.h"

#include "Windows/UserInterface/UIEditorViewportPanel.h"
#include "Windows/UserInterface/UIEditorPropertiesPanel.h"
#include "Windows/UserInterface/UIEditorTreePanel.h"

#include <string>

namespace Kargono { class EditorApp; }

namespace Kargono::Windows
{
	//=========================
	// UI Editor Panel Class
	//=========================
	class UIEditorWindow
	{
	public:
		//=========================
		// Constructor/Destructor
		//=========================
		UIEditorWindow();

		//=========================
		// Lifecycle Functions
		//=========================
		void InitPanels();

	public:
		//=========================
		// On Event Functions
		//=========================
		bool OnInputEvent(Events::Event* event);
		void OnUpdate(Timestep ts);

		void OnEditorUIRender();
		bool OnKeyPressedEditor(Events::KeyPressedEvent event);
		bool OnAssetEvent(Events::Event* event);

	public:
		//=========================
		// External Functionality
		//=========================
		void OpenAssetInEditor(std::filesystem::path& assetLocation);
		void OpenCreateDialog(std::filesystem::path& createLocation);
		void ResetWindowResources();
		void OnRefreshData();

	public:
		//=========================
		// Core Panel Data
		//=========================
		Ref<RuntimeUI::UserInterface> m_EditorUI{ nullptr };
		Assets::AssetHandle m_EditorUIHandle { Assets::EmptyHandle };

		//=========================
		// Panels
		//=========================
		Scope<Panels::UIEditorViewportPanel> m_ViewportPanel{ nullptr };
		Scope<Panels::UIEditorPropertiesPanel> m_PropertiesPanel{ nullptr };
		Scope<Panels::UIEditorTreePanel> m_TreePanel{ nullptr };

		//=========================
		// Display Panel Booleans
		//=========================
		bool m_ShowViewport{ true };
		bool m_ShowProperties{ true };
		bool m_ShowTree{ true };
	private:
		friend Kargono::EditorApp;

	};
}
