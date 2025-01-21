#pragma once

#include "Kargono/Assets/Asset.h"
#include "Kargono/Events/KeyEvent.h"
#include "Kargono/EditorUI/EditorUI.h"
#include "Kargono/Core/FixedString.h"

#include "Windows/MainWindow/AssetViewerPanel.h"
#include "Windows/MainWindow/AIStateEditorPanel.h"
#include "Windows/MainWindow/ContentBrowserPanel.h"
#include "Windows/MainWindow/GameStatePanel.h"
#include "Windows/MainWindow/InputMapPanel.h"
#include "Windows/MainWindow/LogPanel.h"
#include "Windows/MainWindow/PropertiesPanel.h"
#include "Windows/MainWindow/ProjectPanel.h"
#include "Windows/MainWindow/ProjectComponentPanel.h"
#include "Windows/MainWindow/ScriptEditorPanel.h"
#include "Windows/MainWindow/SceneEditorPanel.h"
#include "Windows/MainWindow/StatisticsPanel.h"
#include "Windows/MainWindow/TestingPanel.h"
#include "Windows/MainWindow/TextEditorPanel.h"
#include "Windows/MainWindow/ViewportPanel.h"

#include <string>

namespace Kargono
{
	enum class SceneState
	{
		Edit = 0,
		Play,
		Simulate
	};
}

namespace Kargono::Windows
{

	class MainWindow
	{
	public:
		//=========================
		// Constructor/Destructor
		//=========================
		MainWindow();

		//=========================
		// Lifecycle Functions
		//=========================
		void InitPanels();

		//=========================
		// On Event Functions
		//=========================
		bool OnApplicationEvent(Events::Event* event);
		bool OnAssetEvent(Events::Event* event);
		bool OnEditorEvent(Events::Event* event);
		bool OnInputEvent(Events::Event* event);
		bool OnNetworkEvent(Events::Event* event);
		bool OnSceneEvent(Events::Event* event);
		void OnUpdate(Timestep ts);
		

		void OnEditorUIRender();
		bool OnApplicationResize(Events::ApplicationResizeEvent event);
		bool OnApplicationClose(Events::ApplicationCloseEvent event);
		bool OnUpdateUserCount(Events::UpdateOnlineUsers event);
		bool OnApproveJoinSession(Events::ApproveJoinSession event);
		bool OnUpdateSessionUserSlot(Events::UpdateSessionUserSlot event);
		bool OnUserLeftSession(Events::UserLeftSession event);
		bool OnCurrentSessionInit(Events::CurrentSessionInit event);
		bool OnConnectionTerminated(Events::ConnectionTerminated event);
		bool OnStartSession(Events::StartSession event);
		bool OnSessionReadyCheckConfirm(Events::SessionReadyCheckConfirm event);
		bool OnReceiveSignal(Events::ReceiveSignal event);
		bool OnKeyPressedEditor(Events::KeyPressedEvent event);
		bool OnKeyPressedRuntime(Events::KeyPressedEvent event);
		bool OnMouseButtonPressed(Events::MouseButtonPressedEvent event);
		bool OnUpdateProjectComponent(Events::ManageAsset& event);

	public:
		//=========================
		// External Functions
		//=========================
		void LoadSceneParticleEmitters();

	private:
		//=========================
		// Internal Initialization Functions
		//=========================
		void InitializeExportProjectWidgets();
		void InitializeImportAssetWidgets();
		void InitializeGeneralPopupWidgets();
	public:
		//=========================
		// Manage Scene
		//=========================
		// Create Scene
		void NewSceneDialog();
		void NewSceneDialog(const std::filesystem::path& initialDirectory);
		bool NewScene(const std::string& sceneName);
		void DuplicateEditorScene();
		void DuplicateEditorScene(const std::filesystem::path& initialDirectory);
		// Open Scene
		void OpenSceneDialog();
		void OpenScene(const std::filesystem::path& path);
		void OpenScene(Assets::AssetHandle newScene);
		// Save Scene
		void SaveScene();
		void SerializeScene(Ref<Scenes::Scene> scene);
		// Duplicate an entity
		void OnDuplicateEntity();

		//=========================
		// Scene State Transitions
		//=========================
		void OnPlay();
		void OnSimulate();
		void OnStop();
		void OnPause();
		
		//=========================
		// Command Functions
		//=========================
		void OpenWarningMessage(const char* message);
		void OpenImportFileDialog(const std::filesystem::path& importFileLocation, Assets::AssetType assetType);

	private:
		//=========================
		// Internal Functionality
		//=========================
		// Increment the editor by a number of frames
		void Step(int frames = 1);
	public:
		

		//=========================
		// Panels for this Window
		//=========================
		Scope<Panels::AssetViewerPanel> m_AssetViewerPanel;
		Scope<Panels::AIStateEditorPanel>  m_AIStatePanel;
		Scope<Panels::ContentBrowserPanel>  m_ContentBrowserPanel;
		Scope<Panels::GameStatePanel>  m_GameStatePanel;
		Scope<Panels::InputMapPanel>  m_InputMapPanel;
		Scope<Panels::LogPanel>  m_LogPanel;
		Scope<Panels::PropertiesPanel> m_PropertiesPanel;
		Scope<Panels::ProjectPanel>  m_ProjectPanel;
		Scope<Panels::ProjectComponentPanel>  m_ProjectComponentPanel;
		Scope<Panels::ScriptEditorPanel>  m_ScriptEditorPanel;
		Scope<Panels::TextEditorPanel>  m_TextEditorPanel;
		Scope<Panels::SceneEditorPanel> m_SceneEditorPanel;
		Scope<Panels::StatisticsPanel>  m_StatisticsPanel;
		Scope<Panels::TestingPanel>  m_TestingPanel;
		Scope<Panels::ViewportPanel>  m_ViewportPanel;

	public:
		//=========================
		// Core Window Data
		//=========================
		// Input Maps
		std::unordered_map<std::string, std::function<bool(Events::KeyPressedEvent)>> m_PanelToKeyboardInput{};
		std::unordered_map<std::string, std::function<bool(Events::MouseButtonPressedEvent)>> m_PanelToMousePressedInput{};

		// Editor Scenes
		Ref<Scenes::Scene> m_EditorScene;
		Assets::AssetHandle m_EditorSceneHandle;
		SceneState m_SceneState = SceneState::Edit;

		// TODO: Move into input map editor
		// Cached Scene Data
		Ref<Input::InputMap> m_EditorInputMap{ nullptr };
		Assets::AssetHandle m_EditorInputMapHandle{ Assets::EmptyHandle };

		// Stepping Fields
		bool m_IsPaused = false;
		int m_StepFrames = 0;

	private:
		FixedString32 m_WindowName{ "Main Window" };

		//=========================
		// Widgets
		//=========================
		// Warning widget
		EditorUI::WarningPopupSpec m_GeneralWarningSpec{};
		FixedString256 m_GeneralWarningMessage{};

		// Project export popup widgets
		EditorUI::GenericPopupSpec m_ExportProjectSpec{};
		EditorUI::ChooseDirectorySpec m_ExportProjectLocation{};
		EditorUI::CheckboxSpec m_ExportProjectServer{};

		// Import asset widgets
		EditorUI::GenericPopupSpec m_ImportAssetPopup{};
		EditorUI::EditTextSpec m_ImportNewAssetName{};
		std::filesystem::path m_ImportSourceFilePath{};
		Assets::AssetType m_ImportAssetType{ Assets::AssetType::None };
		EditorUI::ChooseDirectorySpec m_ImportNewFileLocation{};
		
		//=========================
		// Booleans for Displaying UI Windows
		//=========================
		bool m_ShowAssetViewer = false;
		bool m_ShowSceneHierarchy = true;
		bool m_ShowProperties = true;
		bool m_ShowContentBrowser = true;
		bool m_ShowLog = false;
		bool m_ShowStats = false;
		bool m_ShowViewport = true;
		bool m_ShowProject = false;
		bool m_ShowProjectComponent = false;
		bool m_ShowDemoWindow = false;
		bool m_ShowTesting = false;
		bool m_ShowScriptEditor = false;
		bool m_ShowTextEditor = false;
		bool m_ShowGameStateEditor = false;
		bool m_ShowInputMapEditor = false;
		bool m_ShowAIStateEditor = false;

		//=========================
		// General Editor Settings
		//=========================
		bool m_ShowPhysicsColliders = false;
		bool m_ShowCameraFrustums = true;
		bool m_RuntimeFullscreen = false;

	private:
		//=========================
		// Friend Declarations
		//=========================
		friend Panels::AIStateEditorPanel;
		friend Panels::AssetViewerPanel;
		friend Panels::GameStatePanel;
		friend Panels::ContentBrowserPanel;
		friend Panels::InputMapPanel;
		friend Panels::LogPanel;
		friend Panels::ProjectComponentPanel;
		friend Panels::ProjectPanel;
		friend Panels::PropertiesPanel;
		friend Panels::ScriptEditorPanel;
		friend Panels::StatisticsPanel;
		friend Panels::SceneEditorPanel;
		friend Panels::TextEditorPanel;
		friend Panels::TestingPanel;
		friend Panels::ViewportPanel;
	};
}
