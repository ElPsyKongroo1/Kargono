#pragma once
#include "Kargono.h"

#include "Panels/SceneEditorPanel.h"
#include "Panels/ContentBrowserPanel.h"
#include "Panels/LogPanel.h"
#include "Panels/ProjectPanel.h"
#include "Panels/StatisticsPanel.h"
#include "Panels/UIEditorPanel.h"
#include "Panels/ViewportPanel.h"
#include "Panels/EntityClassEditor.h"
#include "Panels/ScriptEditorPanel.h"
#include "Panels/GameStatePanel.h"
#include "Panels/TextEditorPanel.h"
#include "Panels/InputModePanel.h"
#include "Panels/PropertiesPanel.h"
#include "Panels/TestingPanel.h"

#include <filesystem>
#include <thread>
#include <unordered_map>
#include <string>
#include <functional>


namespace Kargono
{
	// Current Editor State Enum and Field
	enum class SceneState
	{
		Edit = 0, Play = 1, Simulate = 2
	};

	//============================================================
	// Editor App Class
	//============================================================
	// This class holds all of the editor application logic. The concept of
	//		layers is in its infancy currently. This class holds the logic for
	//		all of the different systems that the editor uses such as scene management,
	//		ImGui code for editor windows, and project management.
	class EditorApp : public Application
	{
	public:
		//=========================
		// Constructor/Destructor
		//=========================
		// The constructor simply calls the parent constructor and initializes
		//		the ImGui layer for window use.
		EditorApp(const std::filesystem::path& projectPath);
		virtual ~EditorApp() override = default;
		//=========================
		// LifeCycle Functions
		//=========================
		// Think of this functions as a normal Init() function. Currently, this
		//		function initializes the audio system, initializes editor icon
		//		resources, sets up framebuffer for the viewport, initializes
		//		default scene resources, initializes editor boolean values,
		//		requests the user select a project to open, and opens the selected
		//		project. There are other things this function does, take a look at it.
		virtual void Init() override;
		// This function simply closes and cleans up the audio system.

		virtual void Terminate() override;
		// This is a fairly large function and is at the heart of this application.
		//		This function runs every frame. Here is the current functionality:
		//		1. Request viewport resize on Scene to ensure in-runtime cameras are
		//		consistent with the screen aspect ratio
		//		2. Run render commands, physics updates, and script updates depending
		//		on the current state of the editor. The editor can be in the following
		//		states: Edit, Simulate, and Play.
		//		3. Run mouse picking logic to allow the selection of entities with the
		//		mouse in Edit mode.
		//		4. Draw overlay code such as physics colliders, entity selection visualization,
		//		text/ui, etc...
		virtual void OnUpdate(Timestep ts) override;
	private:
		// Increments step
		void Step(int frames = 1);
		
	public:
		// This is a large function but its output is fairly simple. The function manages the ImGui
		//		code that displays all of the panels (including the viewport) shown in the editor.
		//		Here is the functionality:
		//		1. The function starts by managing the dockspace that allows panels to be dynamically
		//		moved in the editor.
		//		2. If editor is in the Play or Simulate mode and m_RuntimeFullscreen is true, the editor
		//		simply displays the UI_Viewport() and UI_Toolbar() panels to simulate windowed fullscreen.
		//		3. The following code manages the top-left menu bar adn its options.
		//		4. The final section conditionally displays UI_Panels based on set boolean values(these are
		//		set in the toolbar above).
		virtual void OnEditorUIRender() override;

	private:
		// Supporting functions for InputEditor. These functions display different sections of the InputEditor
		//		user interface.

	public:
		// This function catches thrown application events and dispatches them to other functions
		//		(OnKeyPressed(), OnMouseButtonPressed(), and OnPhysicsCollision()).
		//		Those functions proceed to run logic that responds to the thrown event.
		virtual void OnEvent(Events::Event& event) override;
	private:
		// These private functions are called by the above OnEvent(e) function to handle application events.
		// These next functions provide different code to respond to user input.
		bool OnKeyPressedEditor(Events::KeyPressedEvent event);
		// This sets the application to stop running (Calls On Stop) at runtime.
		bool OnApplicationClose(Events::ApplicationCloseEvent event);
		bool OnKeyPressedRuntime(Events::KeyPressedEvent event);
		bool OnMouseButtonPressed(Events::MouseButtonPressedEvent event);
		// This function responds to application collision events.
		bool OnPhysicsCollision(Events::PhysicsCollisionEvent event);
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
	public:
		//=========================
		// Getters/Setters
		//=========================
		static EditorApp* GetCurrentApp() { return s_EditorApp; }
	private:
		//=========================
		// Scene/Project Management
		//=========================
		// These functions are called from different parts of the editor to allow smooth
		//		transitions between the current Project and other projects. Also allows
		//		the current project to save.
		bool OpenProject();
		void OpenProject(const std::filesystem::path& path);
		void SaveProject();
		// These functions allow transitioning between scenes in the editor and saving
		//		the current scene.
		void NewScene();
		void NewScene(const std::string& sceneName);
		void OpenScene();
	public:
		void OpenScene(const std::filesystem::path& path);
	private:
		void OpenScene(Assets::AssetHandle newScene);
		void SaveScene();
		// This function is called by SaveSceneAs() to actually serialize the scene.
		void SerializeScene(Ref<Scenes::Scene> scene);

		//=========================
		// Scene State Transitions
		//=========================
		void OnPlay();
		void OnSimulate();
		void OnStop();
		void OnPause();

		// Extra function. Allows the duplicate of an entity in the current m_EditorScene
		void OnDuplicateEntity();
	private:
		//=========================
		// Private Fields
		//=========================
		static EditorApp* s_EditorApp;
		// Booleans to display UI Windows
		bool m_ShowSceneHierarchy = true;
		bool m_ShowProperties = true;
		bool m_ShowContentBrowser = true;
		bool m_ShowLog = false;
		bool m_ShowStats = false;
		bool m_ShowViewport = true;
		bool m_ShowProject = false;
		bool m_ShowDemoWindow = false;
		bool m_ShowTesting = false;
		bool m_ShowUserInterfaceEditor = false;
		bool m_ShowScriptEditor = false;
		bool m_ShowClassEditor = false;
		bool m_ShowTextEditor = false;
		bool m_ShowGameStateEditor = false;
		bool m_ShowInputModeEditor = false;

		// Settings UI Booleans
		bool m_ShowPhysicsColliders = false;
		bool m_ShowCameraFrustums = true;
		bool m_RuntimeFullscreen = false;
		bool m_ShowUserInterface = true;
		// Editor Scenes
		Ref<Scenes::Scene> m_EditorScene;
		Assets::AssetHandle m_EditorSceneHandle;
		SceneState m_SceneState = SceneState::Edit;

		// Cached Scene Data
		Ref<Input::InputMode> m_EditorInputMode = nullptr;
		Assets::AssetHandle m_EditorInputModeHandle{0};
		Ref<RuntimeUI::UserInterface> m_EditorUIObject = nullptr;
		Assets::AssetHandle m_EditorUIObjectHandle{0};

		// Stepping Fields
		bool m_IsPaused = false;
		int m_StepFrames = 0;

		// Input Maps
		std::unordered_map<std::string, std::function<bool(Events::KeyPressedEvent)>> m_PanelToKeyboardInput {};

		// Initialization Fields
		std::filesystem::path m_InitProjectPath {};
	public:
		// Panels
		Scope<Panels::SceneEditorPanel> m_SceneHierarchyPanel;
		Scope<Panels::PropertiesPanel> m_PropertiesPanel;
		Scope<Panels::ContentBrowserPanel>  m_ContentBrowserPanel;
		Scope<Panels::LogPanel>  m_LogPanel;
		Scope<Panels::StatisticsPanel>  m_StatisticsPanel;
		Scope<Panels::ProjectPanel>  m_ProjectPanel;
		Scope<Panels::UIEditorPanel>  m_UIEditorPanel;
		Scope<Panels::ViewportPanel>  m_ViewportPanel;
		Scope<Panels::ScriptEditorPanel>  m_ScriptEditorPanel;
		Scope<Panels::EntityClassEditor>  m_EntityClassEditor;
		Scope<Panels::TextEditorPanel>  m_TextEditorPanel;
		Scope<Panels::GameStatePanel>  m_GameStatePanel;
		Scope<Panels::InputModePanel>  m_InputModePanel;
		Scope<Panels::TestingPanel>  m_TestingPanel;
	private:
		friend Panels::ViewportPanel;
		friend Panels::SceneEditorPanel;
		friend Panels::ContentBrowserPanel;
		friend Panels::LogPanel;
		friend Panels::StatisticsPanel;
		friend Panels::ProjectPanel;
		friend Panels::UIEditorPanel;
		friend Panels::ScriptEditorPanel;
		friend Panels::EntityClassEditor;
		friend Panels::TextEditorPanel;
		friend Panels::GameStatePanel;
		friend Panels::InputModePanel;
		friend Panels::PropertiesPanel;
		friend Panels::TestingPanel;

	};

}
