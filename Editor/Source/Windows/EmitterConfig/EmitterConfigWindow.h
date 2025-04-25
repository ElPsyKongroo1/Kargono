

#pragma once
#include "EventModule/KeyEvent.h"
#include "AssetModule/Asset.h"
#include "EditorUIModule/EditorUI.h"
#include "ParticleModule/ParticleService.h"
#include "Kargono/Core/Base.h"

#include "Windows/EmitterConfig/EmitterConfigViewportPanel.h"
#include "Windows/EmitterConfig/EmitterConfigPropertiesPanel.h"

#include <string>

namespace Kargono { class EditorApp; }

namespace Kargono::Windows
{
	//=========================
	// UI Editor Panel Class
	//=========================
	class EmitterConfigWindow
	{
	public:
		//=========================
		// Constructor/Destructor
		//=========================
		EmitterConfigWindow();

	private:
		void InitializeOpeningScreen();
		void InitializeEmitterConfigHeader();
	public:

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
		void LoadEditorEmitterIntoParticleService();
		void ResetWindowResources();
		void OnRefreshData();

	private:
		//=========================
		// Internal Functionality
		//=========================
		void OnCreateEmitterConfigDialog();
		void OnOpenEmitterConfigDialog();
		void OnOpenEmitterConfig(Assets::AssetHandle newHandle);

	public:
		//=========================
		// Core Panel Data
		//=========================
		Ref<Particles::EmitterConfig> m_EditorEmitterConfig{ nullptr };
		Assets::AssetHandle m_EditorEmitterConfigHandle{ Assets::EmptyHandle };
		FixedString32 m_PanelName{ "Main Particle Emitter Panel" };

		//=========================
		// Panels
		//=========================
		Scope<Panels::EmitterConfigViewportPanel> m_ViewportPanel{ nullptr };
		Scope<Panels::EmitterConfigPropertiesPanel> m_PropertiesPanel{ nullptr };

		//=========================
		// Display Panel Booleans
		//=========================
		bool m_ShowViewport{ true };
		bool m_ShowProperties{ true };

		//=========================
		// Widgets
		//=========================
		// Opening Panel w/ Popups
		EditorUI::SelectOptionSpec m_OpenEmitterConfigPopupSpec{};
		EditorUI::GenericPopupSpec m_CreateEmitterConfigPopupSpec{};
		EditorUI::EditTextSpec m_SelectEmitterConfigNameSpec{};
		EditorUI::ChooseDirectorySpec m_SelectEmitterConfigLocationSpec{};
		EditorUI::TooltipSpec m_SelectScriptTooltip{};

		// User Interface Header
		EditorUI::PanelHeaderSpec m_MainHeader{};
		EditorUI::GenericPopupSpec m_DeleteEmitterConfigWarning{};
		EditorUI::GenericPopupSpec m_CloseEmitterConfigWarning{};

	private:
		friend Kargono::EditorApp;
	};
}


