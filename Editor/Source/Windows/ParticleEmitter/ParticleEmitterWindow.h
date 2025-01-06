#if 0

#pragma once
#include "Kargono/Events/KeyEvent.h"
#include "Kargono/Assets/Asset.h"
#include "Kargono/EditorUI/EditorUI.h"
#include "Kargono/Particles/ParticleService.h"
#include "Kargono/Core/Base.h"

#include "Windows/ParticleEmitter/ParticleEmitterViewportPanel.h"
#include "Windows/ParticleEmitter/ParticleEmitterPropertiesPanel.h"

#include <string>

namespace Kargono { class EditorApp; }

namespace Kargono::Windows
{
	//=========================
	// UI Editor Panel Class
	//=========================
	class ParticleEmitterWindow
	{
	public:
		//=========================
		// Constructor/Destructor
		//=========================
		ParticleEmitterWindow();

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
		Ref<Particles::EmitterConfig> m_EditorEmitter{ nullptr };
		Assets::AssetHandle m_EditorEmitterHandle{ Assets::EmptyHandle };

		//=========================
		// Panels
		//=========================
		Scope<Panels::ParticleEmitterViewportPanel> m_ViewportPanel{ nullptr };
		Scope<Panels::ParticleEmitterPropertiesPanel> m_PropertiesPanel{ nullptr };

		//=========================
		// Display Panel Booleans
		//=========================
		bool m_ShowViewport{ true };
		bool m_ShowProperties{ true };
	private:
		friend Kargono::EditorApp;

	};
}

#endif
