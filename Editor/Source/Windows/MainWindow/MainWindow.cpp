#include "MainWindow.h"
#include "EditorApp.h"

#include "Kargono/Scenes/Scene.h"
#include "Kargono/Scripting/ScriptModuleBuilder.h"
#include "Kargono/Utility/OSCommands.h"
#include "Kargono/Utility/FileDialogs.h"
#include "Kargono/Network/Client.h"
#include "Kargono/Core/AppTick.h"
#include "Kargono/Input/InputService.h"

static Kargono::EditorApp* s_EditorApp{ nullptr };

namespace Kargono::Windows
{
	void MainWindow::InitializeExportProjectWidgets()
	{
		m_ExportProjectSpec.m_Label = "Export Project";
		m_ExportProjectSpec.m_PopupWidth = 420.0f;
		m_ExportProjectSpec.m_PopupContents = [&]()
			{
				EditorUI::EditorUIService::ChooseDirectory(m_ExportProjectLocation);
				EditorUI::EditorUIService::Checkbox(m_ExportProjectServer);
			};
		m_ExportProjectSpec.m_ConfirmAction = [&]()
			{
				Projects::ProjectService::ExportProject(m_ExportProjectLocation.m_CurrentOption, m_ExportProjectServer.m_CurrentBoolean);
			};

		m_ExportProjectLocation.m_Label = "Export Location";
		m_ExportProjectLocation.m_CurrentOption = std::filesystem::current_path().parent_path() / "Export";

		m_ExportProjectServer.m_Label = "Export Server";
		m_ExportProjectServer.m_CurrentBoolean = true;
	}

	void MainWindow::InitializeImportAssetWidgets()
	{

		// Initialize import asset popup data
		m_ImportAssetPopup.m_Label = "Import Asset";
		m_ImportAssetPopup.m_PopupContents = [&]()
			{
				EditorUI::EditorUIService::LabeledText("Source File:", m_ImportSourceFilePath.string());
				EditorUI::EditorUIService::ChooseDirectory(m_ImportNewFileLocation);
				EditorUI::EditorUIService::EditText(m_ImportNewAssetName);
			};
		m_ImportAssetPopup.m_ConfirmAction = [&]()
			{
				// Actually import the asset
				switch (m_ImportAssetType)
				{
				case Assets::AssetType::Font:
					Assets::AssetService::ImportFontFromFile(m_ImportSourceFilePath, m_ImportNewAssetName.m_CurrentOption.c_str(), m_ImportNewFileLocation.m_CurrentOption);
					break;
				case Assets::AssetType::Audio:
					Assets::AssetService::ImportAudioBufferFromFile(m_ImportSourceFilePath, m_ImportNewAssetName.m_CurrentOption.c_str(), m_ImportNewFileLocation.m_CurrentOption);
					break;
				case Assets::AssetType::Texture:
					Assets::AssetService::ImportTexture2DFromFile(m_ImportSourceFilePath, m_ImportNewAssetName.m_CurrentOption.c_str(), m_ImportNewFileLocation.m_CurrentOption);
					break;
				default:
					KG_ERROR("Unsupported asset type attempting to be imported");
					break;
				}
			};

		// Initialize internal popup widgets
		m_ImportNewAssetName.m_Label = "New Asset Name:";
		m_ImportNewAssetName.m_CurrentOption = "NewAsset";

		m_ImportNewFileLocation.m_Label = "Destination Folder:";
		m_ImportNewFileLocation.m_ConfirmAction = [&](const std::string& path)
			{
				if (!Utility::FileSystem::DoesPathContainSubPath(Projects::ProjectService::GetActiveAssetDirectory(), path))
				{
					KG_WARN("Cannot create an asset outside of the project's asset directory.");
					m_ImportNewFileLocation.m_CurrentOption = Projects::ProjectService::GetActiveAssetDirectory();
				}
			};
	}

	void MainWindow::InitializeGeneralPopupWidgets()
	{
		m_GeneralWarningSpec.m_Label = "Warning";
		m_GeneralWarningSpec.m_PopupContents = [&]()
		{
			EditorUI::EditorUIService::LabeledText("Warning Message:", m_GeneralWarningMessage.CString());
		};
	}

	void MainWindow::LoadSceneParticleEmitters()
	{
		if (!m_EditorScene)
		{
			return;
		}

		// Load the emitters for the editor scene
		Particles::ParticleService::ClearEmitters();
		Particles::ParticleService::LoadSceneEmitters(m_EditorScene);
	}

	MainWindow::MainWindow()
	{
		// Initialize the editor app
		s_EditorApp = EditorApp::GetCurrentApp();

		// Initialize the main window widgets
		InitializeExportProjectWidgets();
		InitializeImportAssetWidgets();
		InitializeGeneralPopupWidgets();

		// Initialize the editor scene
		m_EditorScene = CreateRef<Scenes::Scene>();
		Scenes::SceneService::SetActiveScene(m_EditorScene, m_EditorSceneHandle);
		m_SceneState = SceneState::Edit;
		
	}
	void MainWindow::InitPanels()
	{
		// Initialize all editor panels
		m_SceneEditorPanel = CreateScope<Panels::SceneEditorPanel>();
		m_AssetViewerPanel = CreateScope<Panels::AssetViewerPanel>();
		m_LogPanel = CreateScope<Panels::LogPanel>();
		m_StatisticsPanel = CreateScope<Panels::StatisticsPanel>();
		m_ProjectPanel = CreateScope<Panels::ProjectPanel>();
		m_ProjectComponentPanel = CreateScope<Panels::ProjectComponentPanel>();
		m_TextEditorPanel = CreateScope<Panels::TextEditorPanel>();
		m_ScriptEditorPanel = CreateScope<Panels::ScriptEditorPanel>();
		m_GameStatePanel = CreateScope<Panels::GameStatePanel>();
		m_InputMapPanel = CreateScope<Panels::InputMapPanel>();
		m_ContentBrowserPanel = CreateScope<Panels::ContentBrowserPanel>();
		m_PropertiesPanel = CreateScope<Panels::PropertiesPanel>();
		m_AIStatePanel = CreateScope<Panels::AIStateEditorPanel>();

		m_ViewportPanel = CreateScope<Panels::ViewportPanel>();

		m_TestingPanel = CreateScope<Panels::TestingPanel>();
	}
	void MainWindow::OnUpdate(Timestep ts)
	{
		// Call the update function for all panels
		m_ViewportPanel->OnUpdate(ts);
	}
	bool MainWindow::OnNetworkEvent(Events::Event* event)
	{
		bool handled = false;
		if (m_SceneState == SceneState::Play)
		{
			switch (event->GetEventType())
			{
			case Events::EventType::UpdateOnlineUsers:
				handled = OnUpdateUserCount(*(Events::UpdateOnlineUsers*)event);
				break;
			case Events::EventType::ApproveJoinSession:
				handled = OnApproveJoinSession(*(Events::ApproveJoinSession*)event);
				break;
			case Events::EventType::UserLeftSession:
				handled = OnUserLeftSession(*(Events::UserLeftSession*)event);
				break;
			case Events::EventType::CurrentSessionInit:
				handled = OnCurrentSessionInit(*(Events::CurrentSessionInit*)event);
				break;
			case Events::EventType::ConnectionTerminated:
				handled = OnConnectionTerminated(*(Events::ConnectionTerminated*)event);
				break;
			case Events::EventType::UpdateSessionUserSlot:
				handled = OnUpdateSessionUserSlot(*(Events::UpdateSessionUserSlot*)event);
				break;
			case Events::EventType::StartSession:
				handled = OnStartSession(*(Events::StartSession*)event);
				break;
			case Events::EventType::SendReadyCheckConfirm:
				handled = OnSessionReadyCheckConfirm(*(Events::SessionReadyCheckConfirm*)event);
				break;
			case Events::EventType::ReceiveSignal:
				handled = OnReceiveSignal(*(Events::ReceiveSignal*)event);
				break;
			}
		}
		return handled;
	}
	bool MainWindow::OnSceneEvent(Events::Event* event)
	{
		m_SceneEditorPanel->OnSceneEvent(event);
		return false;
	}
	bool MainWindow::OnInputEvent(Events::Event* event)
	{
		bool handled{ false };
		FixedString32 focusedWindow = EditorUI::EditorUIService::GetFocusedWindowName();
		if (focusedWindow == m_ViewportPanel->m_PanelName)
		{
			m_ViewportPanel->OnInputEvent(event);
		}

		if (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Simulate)
		{
			if (event->GetEventType() == Events::EventType::MouseButtonPressed)
			{
				handled = OnMouseButtonPressed(*(Events::MouseButtonPressedEvent*)event);
			}

			if (event->GetEventType() == Events::EventType::KeyPressed)
			{
				handled = OnKeyPressedEditor(*(Events::KeyPressedEvent*)event);
			}

			if (event->GetEventType() == Events::EventType::KeyReleased)
			{
				handled = m_ViewportPanel->m_EditorCamera.OnKeyReleased(*(Events::KeyReleasedEvent*)event);
			}
		}

		if (m_SceneState == SceneState::Play)
		{
			if (event->GetEventType() == Events::EventType::MouseButtonPressed)
			{
				handled = OnMouseButtonPressed(*(Events::MouseButtonPressedEvent*)event);
			}

			if (event->GetEventType() == Events::EventType::MouseButtonReleased)
			{
				handled = OnMouseButtonReleasedRuntime(*(Events::MouseButtonReleasedEvent*)event);
			}

			if (event->GetEventType() == Events::EventType::KeyPressed)
			{
				handled = OnKeyPressedRuntime(*(Events::KeyPressedEvent*)event);
			}
			if (event->GetEventType() == Events::EventType::KeyTyped)
			{
				handled = OnKeyTypedRuntime(*(Events::KeyTypedEvent*)event);
			}
		}

		m_TestingPanel->OnInputEvent(event);

		return handled;
	}
	bool MainWindow::OnAssetEvent(Events::Event* event)
	{
		Events::ManageAsset& manageAsset = *(Events::ManageAsset*)event;

		// Refresh text editor
		m_TextEditorPanel->OnAssetEvent(event);

		// Handle adding a project component to the active editor scene
		if (manageAsset.GetAssetType() == Assets::AssetType::ProjectComponent &&
			manageAsset.GetAction() == Events::ManageAssetAction::Create &&
			m_EditorScene)
		{
			// Create project component inside scene registry
			m_EditorScene->AddProjectComponentRegistry(manageAsset.GetAssetID());
		}
		// Handle editing a project component by modifying entity component data inside the Assets::AssetService::SceneRegistry and the active editor scene
		if (manageAsset.GetAssetType() == Assets::AssetType::ProjectComponent &&
			manageAsset.GetAction() == Events::ManageAssetAction::UpdateAsset)
		{
			OnUpdateProjectComponent(manageAsset);
		}
		// Handle deleting a project component by removing entity data from all scenes
		if (manageAsset.GetAssetType() == Assets::AssetType::ProjectComponent &&
			manageAsset.GetAction() == Events::ManageAssetAction::PreDelete &&
			m_EditorScene)
		{
			// Remove project component from editor scene
			Assets::AssetService::RemoveProjectComponentFromScene(m_EditorScene, manageAsset.GetAssetID());
		}

		// Handle deleting a emitter config by removing entity data from all scenes
		if (manageAsset.GetAssetType() == Assets::AssetType::EmitterConfig &&
			manageAsset.GetAction() == Events::ManageAssetAction::PreDelete &&
			m_EditorScene)
		{
			// Remove emitter config from editor scene
			Assets::AssetService::RemoveEmitterConfigFromScene(m_EditorScene, manageAsset.GetAssetID());

			// Reset emitters in the scene
			LoadSceneParticleEmitters();
		}

		// Handle removing scripts from editor scene
		if (manageAsset.GetAssetType() == Assets::AssetType::Script &&
			manageAsset.GetAction() == Events::ManageAssetAction::PreDelete &&
			m_EditorScene)
		{
			Assets::AssetService::RemoveScriptFromScene(m_EditorScene, manageAsset.GetAssetID());
		}

		// Handle removing ai state from editor scene
		if (manageAsset.GetAssetType() == Assets::AssetType::AIState &&
			manageAsset.GetAction() == Events::ManageAssetAction::PreDelete &&
			m_EditorScene)
		{
			Assets::AssetService::RemoveAIStateFromScene(m_EditorScene, manageAsset.GetAssetID());
		}

		if (manageAsset.GetAssetType() == Assets::AssetType::Scene &&
			manageAsset.GetAssetID() == m_EditorSceneHandle)
		{
			if (manageAsset.GetAction() == Events::ManageAssetAction::PreDelete)
			{
				// Create new scene w/ unique name
				uint32_t iteration{ 1 };
				bool success{ false };
				while (!success)
				{
					FixedString16 sceneName{ "NewScene" };
					sceneName.AppendInteger(iteration);
					success = NewScene(sceneName.CString());
					iteration++;
				}
			}

		}

		m_SceneEditorPanel->OnAssetEvent(event);
		m_AssetViewerPanel->OnAssetEvent(event);
		m_AIStatePanel->OnAssetEvent(event);
		m_InputMapPanel->OnAssetEvent(event);
		m_ProjectPanel->OnAssetEvent(event);

		switch (manageAsset.GetAssetType())
		{
		case Assets::AssetType::GameState:
			m_GameStatePanel->OnAssetEvent(event);
			break;
		case Assets::AssetType::ProjectComponent:
			m_ProjectComponentPanel->OnAssetEvent(event);
			break;
		case Assets::AssetType::Script:
			m_ScriptEditorPanel->OnAssetEvent(event);
			break;
		default:
			break;
		}

		return false;
	}
	bool MainWindow::OnEditorEvent(Events::Event* event)
	{
		m_ViewportPanel->OnEditorEvent(event);
		return false;
	}
	void MainWindow::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION();
		EditorUI::EditorUIService::StartRendering();

		EditorUI::EditorUIService::StartDockspaceWindow();

		// Set the active viewport for the window
		EngineService::GetActiveWindow().SetActiveViewport(&m_ViewportPanel->m_ViewportData);

		// Set up Menu Toolbar
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open Project"))
				{
					s_EditorApp->OpenProject();
				}

				if (ImGui::MenuItem("Save Project", "Ctrl+S"))
				{
					SaveScene();
					s_EditorApp->SaveProject();
				}

				if (ImGui::MenuItem("Export Project"))
				{
					m_ExportProjectSpec.m_OpenPopup = true;
				}

				ImGui::Separator();

				if (ImGui::MenuItem("Reload Script Module"))
				{
					Scripting::ScriptService::LoadActiveScriptModule();
				}
				if (ImGui::MenuItem("Rebuild Script Module"))
				{
					Scripting::ScriptModuleBuilder::CreateScriptModule();
				}

				ImGui::Separator();

				if (ImGui::MenuItem("Exit"))
				{
					EngineService::EndRun();
				}
				ImGui::EndMenu();

			}

			if (ImGui::BeginMenu("Windows"))
			{
				if (ImGui::MenuItem("User Interface Editor"))
				{
					EngineService::SubmitToMainThread([]() 
					{
						s_EditorApp->SetActiveEditorWindow(ActiveEditorUIWindow::UIEditorWindow);
					});
				}
				if (ImGui::MenuItem("Particle Emitter Editor"))
				{
					EngineService::SubmitToMainThread([]()
					{
						s_EditorApp->SetActiveEditorWindow(ActiveEditorUIWindow::EmitterConfigWindow);
						s_EditorApp->m_EmitterConfigEditorWindow->LoadEditorEmitterIntoParticleService();
					});
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Panels"))
			{
				ImGui::MenuItem("Asset Viewer", NULL, &m_ShowAssetViewer);
				ImGui::MenuItem("Content Browser", NULL, &m_ShowContentBrowser);
				ImGui::MenuItem("Scene Hierarchy", NULL, &m_ShowSceneHierarchy);
				ImGui::MenuItem("Viewport", NULL, &m_ShowViewport);
				ImGui::MenuItem("Properties", NULL, &m_ShowProperties);
				ImGui::Separator();
				ImGui::MenuItem("Input Map Editor", NULL, &m_ShowInputMapEditor);
				ImGui::MenuItem("Script Editor", NULL, &m_ShowScriptEditor);
				ImGui::MenuItem("Text Editor", NULL, &m_ShowTextEditor);
				ImGui::MenuItem("Game State Editor", NULL, &m_ShowGameStateEditor);
				ImGui::MenuItem("Component Editor", NULL, &m_ShowProjectComponent);
				ImGui::MenuItem("AI State Editor", NULL, &m_ShowAIStateEditor);
				ImGui::Separator();
				ImGui::MenuItem("Project Settings", NULL, &m_ShowProject);
				ImGui::EndMenu();
			}
			// TODO: Fullscreen for select panels SHOUTOUT OPERATOR DOOOOOOM
#if 0
			if (ImGui::BeginMenu("View"))
			{
				ImGui::MenuItem("Display Content Browser Fullscreen", NULL, &m_ContentBrowserFullscreen);
				ImGui::EndMenu();
			}
#endif

			if (ImGui::BeginMenu("Debug"))
			{
				ImGui::MenuItem("Log", NULL, &m_ShowLog);
				if (ImGui::MenuItem("Profiler"))
				{
					Utility::OSCommands::OpenProfiler();
				}

				ImGui::MenuItem("Testing Window", NULL, &m_ShowTesting);
				ImGui::MenuItem("Stats", NULL, &m_ShowStats);

				if (ImGui::BeginMenu("ImGui Options"))
				{

					if (ImGui::MenuItem("Save Config Settings"))
					{
						ImGui::SaveIniSettingsToDisk("./Resources/EditorConfig.ini");
					}
					ImGui::MenuItem("ImGui Demo", NULL, &m_ShowDemoWindow);
					ImGui::EndMenu();
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Help"))
			{
				if (ImGui::MenuItem("Engine Docs"))
				{
					Utility::OSCommands::OpenWebURL("https://elpsykongroo1.github.io/Kargono/");
				}
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		// Only display the viewport if the scene is in play mode and the fullscreen flag is set
		if (m_RuntimeFullscreen && (m_SceneState == SceneState::Play || m_SceneState == SceneState::Simulate) && !m_IsPaused)
		{
			if (m_ShowViewport) { m_ViewportPanel->OnEditorUIRender(); }
			EditorUI::EditorUIService::EndWindow();
			EditorUI::EditorUIService::EndRendering();
			return;
		}

#if 0 // TODO: SHOUTOUT OPERATOR DOOOM
		/*if (m_ContentBrowserFullscreen)
		{
			m_ContentBrowserPanel->OnEditorUIRender();
			EditorUI::EditorUIService::EndWindow();
			EditorUI::EditorUIService::EndRendering();
			return;
		}*/
#endif

		// Display all main window panels
		if (m_ShowAssetViewer) { m_AssetViewerPanel->OnEditorUIRender(); }
		if (m_ShowSceneHierarchy) { m_SceneEditorPanel->OnEditorUIRender(); }
		if (m_ShowContentBrowser) { m_ContentBrowserPanel->OnEditorUIRender(); }
		if (m_ShowLog) { m_LogPanel->OnEditorUIRender(); }
		if (m_ShowStats) { m_StatisticsPanel->OnEditorUIRender(); }
		if (m_ShowViewport) { m_ViewportPanel->OnEditorUIRender(); }
		if (m_ShowProject) { m_ProjectPanel->OnEditorUIRender(); }
		if (m_ShowProjectComponent) { m_ProjectComponentPanel->OnEditorUIRender(); }
		if (m_ShowScriptEditor) { m_ScriptEditorPanel->OnEditorUIRender(); }
		if (m_ShowTextEditor) { m_TextEditorPanel->OnEditorUIRender(); }
		if (m_ShowGameStateEditor) { m_GameStatePanel->OnEditorUIRender(); }
		if (m_ShowInputMapEditor) { m_InputMapPanel->OnEditorUIRender(); }
		if (m_ShowProperties) { m_PropertiesPanel->OnEditorUIRender(); }
		if (m_ShowDemoWindow) { ImGui::ShowDemoWindow(&m_ShowDemoWindow); }
		if (m_ShowTesting) { m_TestingPanel->OnEditorUIRender(); }
		if (m_ShowAIStateEditor) { m_AIStatePanel->OnEditorUIRender(); }

		// Handle displaying popups if necessary
		EditorUI::EditorUIService::GenericPopup(m_ExportProjectSpec);
		EditorUI::EditorUIService::GenericPopup(m_ImportAssetPopup);
		EditorUI::EditorUIService::WarningPopup(m_GeneralWarningSpec);

		// Clean up dockspace window
		EditorUI::EditorUIService::EndDockspaceWindow();

		// Add highlighting around the focused window
		EditorUI::EditorUIService::HighlightFocusedWindow();

		// End Editor UI Rendering
		EditorUI::EditorUIService::EndRendering();
	}

	bool MainWindow::OnApplicationResize(Events::ApplicationResizeEvent event)
	{
		// Resize the window's viewport
		Math::uvec2 aspectRatio = Utility::GetClosestAspectRatio({ event.GetWidth(), event.GetHeight() });
		m_ViewportPanel->SetViewportAspectRatio(aspectRatio);
		return false;
	}

	bool MainWindow::OnApplicationClose(Events::ApplicationCloseEvent event)
	{
		OnStop();
		return false;
	}

	void MainWindow::Step(int frames)
	{
		m_StepFrames = frames;
	}

	void MainWindow::NewSceneDialog()
	{
		NewSceneDialog(Projects::ProjectService::GetActiveAssetDirectory());
	}

	bool MainWindow::NewScene(const std::string& sceneName)
	{
		// Ensure scene does not already exist
		std::filesystem::path filepath = Projects::ProjectService::GetActiveAssetDirectory() / ("Scenes/" + sceneName + ".kgscene");
		if (Assets::AssetService::HasScene(filepath.stem().string()))
		{
			KG_WARN("Attempt to create scene with duplicate name!");
			return false;
		}

		// Create new scene
		m_EditorSceneHandle = Assets::AssetService::CreateScene(filepath.stem().string().c_str());

		// Validate scene creation
		if (m_EditorSceneHandle == Assets::EmptyHandle)
		{
			return false;
		}

		// Reset editor data
		*Scenes::SceneService::GetActiveScene()->GetHoveredEntity() = {};
		m_EditorScene = Assets::AssetService::GetScene(m_EditorSceneHandle);
		Scenes::SceneService::SetActiveScene(m_EditorScene, m_EditorSceneHandle);
		return true;
	}

	void MainWindow::NewSceneDialog(const std::filesystem::path& initialDirectory)
	{
		// Provide file dialog to select location and name of scene
		std::filesystem::path filepath = Utility::FileDialogs::SaveFile("Kargono Scene (*.kgscene)\0*.kgscene\0", initialDirectory.string().c_str());
		if (filepath.empty())
		{
			KG_WARN("Attempt to create scene failed. No creation directory returned from save dialog");
			return;
		}
		if (Assets::AssetService::HasScene(filepath.stem().string()))
		{
			KG_WARN("Attempt to create scene with duplicate name!");
			return;
		}

		// Create scene file
		std::string fileName = filepath.stem().string();
		std::filesystem::path sceneDirectory = filepath.parent_path();
		m_EditorSceneHandle = Assets::AssetService::CreateScene(fileName.c_str(), sceneDirectory);

		// Open new scene in editor
		*Scenes::SceneService::GetActiveScene()->GetHoveredEntity() = {};
		m_EditorScene = Assets::AssetService::GetScene(m_EditorSceneHandle);
		Scenes::SceneService::SetActiveScene(m_EditorScene, m_EditorSceneHandle);
		LoadSceneParticleEmitters();

	}

	void MainWindow::DuplicateEditorScene()
	{
		DuplicateEditorScene(Projects::ProjectService::GetActiveAssetDirectory());
	}

	void MainWindow::DuplicateEditorScene(const std::filesystem::path& initialDirectory)
	{
		// Provide file dialog to select location and name of scene
		std::filesystem::path filepath = Utility::FileDialogs::SaveFile("Kargono Scene (*.kgscene)\0*.kgscene\0", initialDirectory.string().c_str());
		if (filepath.empty())
		{
			KG_WARN("Attempt to create scene failed. No creation directory returned from save dialog");
			return;
		}
		if (Assets::AssetService::HasScene(filepath.stem().string()))
		{
			KG_WARN("Attempt to create scene with duplicate name!");
			return;
		}

		// Create scene file
		std::string fileName = filepath.stem().string();
		std::filesystem::path sceneDirectory = filepath.parent_path();
		m_EditorSceneHandle = Assets::AssetService::CreateScene(fileName.c_str(), sceneDirectory);

		// Duplicate existing scene
		*Scenes::SceneService::GetActiveScene()->GetHoveredEntity() = {};
		Assets::AssetService::SaveScene(m_EditorSceneHandle, m_EditorScene);

		// Open new scene in engine
		Scenes::SceneService::SetActiveScene(m_EditorScene, m_EditorSceneHandle);
	}

	void MainWindow::OpenSceneDialog()
	{
		std::filesystem::path initialDirectory = Projects::ProjectService::GetActiveAssetDirectory();
		std::filesystem::path filepath = Utility::FileDialogs::OpenFile("Kargono Scene (*.kgscene)\0*.kgscene\0", initialDirectory.string().c_str());

		if (!filepath.empty())
		{
			OpenScene(filepath);
		}
	}

	void MainWindow::OpenScene(const std::filesystem::path& path)
	{
		if (m_SceneState != SceneState::Edit)
		{
			OnStop();
		}

		if (path.extension().string() != ".kgscene")
		{
			KG_WARN("Could not load {0} - not a scene file", path.filename().string());
			return;
		}
		auto [sceneHandle, newScene] = Assets::AssetService::GetScene(path);

		m_EditorScene = newScene;
		m_EditorSceneHandle = sceneHandle;
		Scenes::SceneService::SetActiveScene(m_EditorScene, m_EditorSceneHandle);
		EngineService::SubmitToMainThread([&]()
		{
			LoadSceneParticleEmitters();
		});

	}

	void MainWindow::OpenScene(Assets::AssetHandle sceneHandle)
	{
		if (m_SceneState != SceneState::Edit)
		{
			OnStop();
		}

		Ref<Scenes::Scene> newScene = Assets::AssetService::GetScene(sceneHandle);
		if (!newScene) { newScene = CreateRef<Scenes::Scene>(); }

		m_EditorScene = newScene;
		m_EditorSceneHandle = sceneHandle;
		Scenes::SceneService::SetActiveScene(m_EditorScene, m_EditorSceneHandle);

		EngineService::SubmitToMainThread([&]() 
		{
			LoadSceneParticleEmitters();
		});
		
	}

	void MainWindow::SaveScene()
	{
		SerializeScene(m_EditorScene);
	}

	void MainWindow::SerializeScene(Ref<Scenes::Scene> scene)
	{
		Assets::AssetService::SaveScene(m_EditorSceneHandle, scene);
	}

	void MainWindow::OnDuplicateEntity()
	{
		if (m_SceneState != SceneState::Edit)
		{
			return;
		}

		ECS::Entity selectedEntity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
		if (selectedEntity)
		{
			ECS::Entity newEntity = m_EditorScene->DuplicateEntity(selectedEntity);
			m_SceneEditorPanel->SetSelectedEntity(newEntity);
			m_SceneEditorPanel->SetDisplayedComponent(ECS::ComponentType::None);
		}
	}

	void MainWindow::OnPlay()
	{
		// Resize the window to the project's viewport settings
		m_ViewportPanel->SetViewportAspectRatio(Utility::ScreenResolutionToAspectRatio(Projects::ProjectService::GetActiveTargetResolution()));

		RuntimeUI::RuntimeUIService::ClearActiveUI();
		// Cache Current InputMap in editor
		if (!Input::InputMapService::GetActiveInputMap())
		{
			m_EditorInputMap = nullptr;
		}
		else
		{
			m_EditorInputMap = Input::InputMapService::GetActiveInputMap();
			m_EditorInputMapHandle = Input::InputMapService::GetActiveInputMapHandle();
		}

		// Load Default Game State
		if (Projects::ProjectService::GetActiveStartGameStateHandle() == 0)
		{
			Scenes::GameStateService::ClearActiveGameState();
		}
		else
		{
			Scenes::GameStateService::SetActiveGameState(
				Assets::AssetService::GetGameState(Projects::ProjectService::GetActiveStartGameStateHandle()),
				Projects::ProjectService::GetActiveStartGameStateHandle());
		}

		*Scenes::SceneService::GetActiveScene()->GetHoveredEntity() = {};
		if (m_SceneState == SceneState::Simulate) { OnStop(); }

		Particles::ParticleService::ClearEmitters();

		m_SceneState = SceneState::Play;
		Scenes::SceneService::SetActiveScene(Scenes::SceneService::CreateSceneCopy(m_EditorScene), m_EditorSceneHandle);
		Physics::Physics2DService::Init(Scenes::SceneService::GetActiveScene().get(), Scenes::SceneService::GetActiveScene()->m_PhysicsSpecification);
		Scenes::SceneService::GetActiveScene()->OnRuntimeStart();
		Assets::AssetHandle scriptHandle = Projects::ProjectService::GetActiveOnRuntimeStartHandle();
		if (scriptHandle != 0)
		{
			Utility::CallWrappedVoidNone(Assets::AssetService::GetScript(scriptHandle)->m_Function);
		}

		if (Projects::ProjectService::GetActiveAppIsNetworked())
		{
			Network::ClientService::Init();
		}

		// Load particle emitters
		Particles::ParticleService::LoadSceneEmitters(Scenes::SceneService::GetActiveScene());

		AppTickService::LoadGeneratorsFromProject();
		EngineService::GetActiveEngine().UpdateAppStartTime();
		EditorUI::EditorUIService::SetFocusedWindow(m_ViewportPanel->m_PanelName);
	}

	void MainWindow::OnSimulate()
	{
		*Scenes::SceneService::GetActiveScene()->GetHoveredEntity() = {};
		if (m_SceneState == SceneState::Play) { OnStop(); }

		m_SceneState = SceneState::Simulate;
		Scenes::SceneService::SetActiveScene(Scenes::SceneService::CreateSceneCopy(m_EditorScene), m_EditorSceneHandle);
		Physics::Physics2DService::Init(Scenes::SceneService::GetActiveScene().get(), Scenes::SceneService::GetActiveScene()->m_PhysicsSpecification);
	}
	void MainWindow::OnStop()
	{
		// Resize the window to the project's viewport settings
		m_ViewportPanel->SetViewportAspectRatio(Utility::ScreenResolutionToAspectRatio(Projects::ProjectService::GetActiveTargetResolution()));

		*Scenes::SceneService::GetActiveScene()->GetHoveredEntity() = {};
		KG_ASSERT(m_SceneState == SceneState::Play || m_SceneState == SceneState::Simulate, "Unknown Scene State Given to OnSceneStop")

			if (m_SceneState == SceneState::Play)
			{
				Physics::Physics2DService::Terminate();
				Scenes::SceneService::GetActiveScene()->OnRuntimeStop();
			}
			else if (m_SceneState == SceneState::Simulate)
			{
				Physics::Physics2DService::Terminate();
			}

		Scenes::SceneService::GetActiveScene()->DestroyAllEntities();
		Scenes::SceneService::SetActiveScene(m_EditorScene, m_EditorSceneHandle);
		Audio::AudioService::StopAllAudio();

		// TODO: DEAL WITH THIS
		//// Clear UIObjects during runtime.
		//if (m_UIEditorPanel->m_EditorUI)
		//{
		//	RuntimeUI::RuntimeUIService::SetActiveUI(m_UIEditorPanel->m_EditorUI, m_UIEditorPanel->m_EditorUIHandle);
		//}
		//else
		//{
		//	RuntimeUI::RuntimeUIService::ClearActiveUI();
		//}

		// Clear InputMaps during runtime.
		if (m_EditorInputMap)
		{
			Input::InputMapService::SetActiveInputMap(m_EditorInputMap, m_EditorInputMapHandle);
		}
		else
		{
			Input::InputMapService::SetActiveInputMap(nullptr, Assets::EmptyHandle);
		}

		Scenes::GameStateService::ClearActiveGameState();

		if (Projects::ProjectService::GetActiveAppIsNetworked() && m_SceneState == SceneState::Play)
		{
			Network::ClientService::Terminate();
		}

		// Revalidate particles for editor scene
		Particles::ParticleService::ClearEmitters();
		Particles::ParticleService::LoadSceneEmitters(m_EditorScene);

		// Bring back the old UI
		if (s_EditorApp->m_UIEditorWindow->m_EditorUI)
		{
			RuntimeUI::RuntimeUIService::SetActiveUI(s_EditorApp->m_UIEditorWindow->m_EditorUI,
				s_EditorApp->m_UIEditorWindow->m_EditorUIHandle);
		}
		

		AppTickService::ClearGenerators();

		m_SceneState = SceneState::Edit;
	}

	void MainWindow::OnPause()
	{
		if (m_SceneState == SceneState::Edit) { return; }

		m_IsPaused = true;
	}


	void MainWindow::OpenWarningMessage(const char* message)
	{
		// Open generic message
		m_GeneralWarningMessage = message;
		m_GeneralWarningSpec.m_OpenPopup = true;
	}

	void MainWindow::OpenImportFileDialog(const std::filesystem::path& importFileLocation, Assets::AssetType assetType)
	{
		// Ensure a file is provided
		if (!Utility::FileSystem::HasFileExtension(importFileLocation))
		{
			KG_WARN("Attempt to import a file that does have a file extension");
			return;
		}

		// Fill import asset location
		m_ImportSourceFilePath = importFileLocation;

		// Fill export file path location
		m_ImportNewFileLocation.m_CurrentOption = importFileLocation.parent_path();

		// Add default name to name widget
		m_ImportNewAssetName.m_CurrentOption = "NewAsset";

		// Store asset type
		m_ImportAssetType = assetType;

		// Open Popup and change title
		m_ImportAssetPopup.m_OpenPopup = true;
		m_ImportAssetPopup.m_Label = "Import " + Utility::AssetTypeToString(assetType);
	}

	bool MainWindow::OnApplicationEvent(Events::Event* event)
	{
		bool handled{ false };

		if (m_SceneState == SceneState::Play)
		{
			if (event->GetEventType() == Events::EventType::AppClose)
			{
				handled = OnApplicationClose(*(Events::ApplicationCloseEvent*)event);
			}
			else if (event->GetEventType() == Events::EventType::AppResize)
			{
				handled = OnApplicationResize(*(Events::ApplicationResizeEvent*)event);
			}
		}
		return handled;
	}

	bool MainWindow::OnUpdateUserCount(Events::UpdateOnlineUsers event)
	{
		Assets::AssetHandle scriptHandle = Projects::ProjectService::GetActiveOnUpdateUserCountHandle();
		if (scriptHandle != Assets::EmptyHandle)
		{
			Utility::CallWrappedVoidUInt32(Assets::AssetService::GetScript(scriptHandle)->m_Function, event.GetUserCount());
		}

		return false;
	}

	bool MainWindow::OnApproveJoinSession(Events::ApproveJoinSession event)
	{
		Assets::AssetHandle scriptHandle = Projects::ProjectService::GetActiveOnApproveJoinSessionHandle();
		if (scriptHandle != Assets::EmptyHandle)
		{
			Utility::CallWrappedVoidUInt16(Assets::AssetService::GetScript(scriptHandle)->m_Function, event.GetUserSlot());
		}

		return false;
	}

	bool MainWindow::OnUpdateSessionUserSlot(Events::UpdateSessionUserSlot event)
	{
		Assets::AssetHandle scriptHandle = Projects::ProjectService::GetActiveOnUpdateSessionUserSlotHandle();
		if (scriptHandle != Assets::EmptyHandle)
		{
			Utility::CallWrappedVoidUInt16(Assets::AssetService::GetScript(scriptHandle)->m_Function, event.GetUserSlot());
		}

		return false;
	}

	bool MainWindow::OnUserLeftSession(Events::UserLeftSession event)
	{
		Assets::AssetHandle scriptHandle = Projects::ProjectService::GetActiveOnUserLeftSessionHandle();
		if (scriptHandle != Assets::EmptyHandle)
		{
			Utility::CallWrappedVoidUInt16(Assets::AssetService::GetScript(scriptHandle)->m_Function, event.GetUserSlot());
		}
		return false;
	}

	bool MainWindow::OnCurrentSessionInit(Events::CurrentSessionInit event)
	{
		Assets::AssetHandle scriptHandle = Projects::ProjectService::GetActiveOnCurrentSessionInitHandle();
		if (scriptHandle != Assets::EmptyHandle)
		{
			Utility::CallWrappedVoidNone(Assets::AssetService::GetScript(scriptHandle)->m_Function);
		}
		return false;
	}

	bool MainWindow::OnConnectionTerminated(Events::ConnectionTerminated event)
	{
		Assets::AssetHandle scriptHandle = Projects::ProjectService::GetActiveOnConnectionTerminatedHandle();
		if (scriptHandle != Assets::EmptyHandle)
		{
			Utility::CallWrappedVoidNone(Assets::AssetService::GetScript(scriptHandle)->m_Function);
		}
		return false;
	}

	bool MainWindow::OnStartSession(Events::StartSession event)
	{
		Assets::AssetHandle scriptHandle = Projects::ProjectService::GetActiveOnStartSessionHandle();
		if (scriptHandle != Assets::EmptyHandle)
		{
			Utility::CallWrappedVoidNone(Assets::AssetService::GetScript(scriptHandle)->m_Function);
		}
		return false;
	}

	bool MainWindow::OnSessionReadyCheckConfirm(Events::SessionReadyCheckConfirm event)
	{
		Assets::AssetHandle scriptHandle = Projects::ProjectService::GetActiveOnSessionReadyCheckConfirmHandle();
		if (scriptHandle != Assets::EmptyHandle)
		{
			Utility::CallWrappedVoidNone(Assets::AssetService::GetScript(scriptHandle)->m_Function);
		}
		return false;
	}

	bool MainWindow::OnReceiveSignal(Events::ReceiveSignal event)
	{
		Assets::AssetHandle scriptHandle = Projects::ProjectService::GetActiveOnReceiveSignalHandle();
		if (scriptHandle != Assets::EmptyHandle)
		{
			Utility::CallWrappedVoidUInt16(Assets::AssetService::GetScript(scriptHandle)->m_Function, event.GetSignal());
		}
		return false;
	}

	bool MainWindow::OnKeyPressedRuntime(Events::KeyPressedEvent event)
	{
		KG_PROFILE_FUNCTION();
		bool handled = RuntimeUI::RuntimeUIService::OnKeyPressedEvent(event);

		if (!handled)
		{
			Input::InputMapService::OnKeyPressed(event);
		}
		
		return handled;
	}

	bool MainWindow::OnKeyTypedRuntime(Events::KeyTypedEvent event)
	{
		RuntimeUI::RuntimeUIService::OnKeyTypedEvent(event);
		return false;
	}

	bool MainWindow::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		if (event.IsRepeat()) { return false; }

		// Handle panel specific key pressed events
		FixedString32 focusedWindow = EditorUI::EditorUIService::GetFocusedWindowName();
		if (m_PanelToKeyboardInput.contains(focusedWindow.CString()))
		{
			if (m_PanelToKeyboardInput.at(focusedWindow.CString())(event))
			{
				return true;
			}
		}

		// Handle general keyboard input chords
		bool control = Input::InputService::IsKeyPressed(Key::LeftControl) || Input::InputService::IsKeyPressed(Key::RightControl);
		bool shift = Input::InputService::IsKeyPressed(Key::LeftShift) || Input::InputService::IsKeyPressed(Key::RightShift);
		bool alt = Input::InputService::IsKeyPressed(Key::LeftAlt) || Input::InputService::IsKeyPressed(Key::RightAlt);

		switch (event.GetKeyCode())
		{

		case Key::N:
		{
			if (control) { NewSceneDialog(); }
			break;
		}
		case Key::O:
		{
			if (control) 
			{ 
				s_EditorApp->OpenProject(); 
			}
			break;
		}
		case Key::S:
		{
			if (control)
			{
				SaveScene();
				s_EditorApp->SaveProject();
			}
			break;
		}
		case Key::F11:
		{
			EngineService::GetActiveWindow().ToggleMaximized();
			break;
		}

		// Scene Commands

		case Key::D:
		{
			if (control) { OnDuplicateEntity(); }
			break;
		}

		// Gizmos
		case Key::R:
		{
			if (!ImGuizmo::IsUsing()) { m_ViewportPanel->m_GizmoType = ImGuizmo::OPERATION::SCALE; }
			break;
		}
		case Key::Delete:
		{
			if (EditorUI::EditorUIService::GetActiveWidgetID() == 0)
			{
				ECS::Entity selectedEntity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
				if (selectedEntity)
				{
					m_EditorScene->DestroyEntity(selectedEntity);
					m_SceneEditorPanel->SetSelectedEntity({});
				}
			}
			break;

		}
		default:
		{
			break;
		}
		}
		return false;
	}

	bool MainWindow::OnMouseButtonPressed(Events::MouseButtonPressedEvent event)
	{
		// Refocus window if right click is used
		FixedString32 focusedWindow = EditorUI::EditorUIService::GetFocusedWindowName();
		if (event.GetMouseButton() == Mouse::ButtonRight)
		{
			if (const char* hoveredName = EditorUI::EditorUIService::GetHoveredWindowName())
			{
				EditorUI::EditorUIService::SetFocusedWindow(hoveredName);
				focusedWindow = hoveredName;
			}
		}

		// Handle panel specific mouse pressed events
		if (m_PanelToMousePressedInput.contains(focusedWindow.CString()))
		{
			if (m_PanelToMousePressedInput.at(focusedWindow.CString())(event))
			{
				return true;
			}
		}
		return false;
	}

	bool MainWindow::OnMouseButtonReleasedRuntime(const Events::MouseButtonReleasedEvent& event)
	{
		if (RuntimeUI::RuntimeUIService::GetActiveUI())
		{
			RuntimeUI::RuntimeUIService::OnMouseButtonReleasedEvent(event);
		}
		return false;
	}

	static void TransferSceneData(Events::ManageAsset& event, Ref<Scenes::Scene> currentScene)
	{
		// Get reallocation instructions
		KG_ASSERT(event.GetProvidedData());
		Assets::FieldReallocationInstructions& reallocationInstructions = *(Assets::FieldReallocationInstructions*)event.GetProvidedData().get();

		// Check if original data source is already empty. If it is empty, all there is to do is create a new registry
		if (reallocationInstructions.m_OldDataLocations.size() == 0)
		{
			// Create/Overwrite scene registry using the new archetype of field data
			currentScene->AddProjectComponentRegistry(event.GetAssetID());
			return;
		}

		// Check if new data layout is empty. Simply remove the current registry.
		if (reallocationInstructions.m_NewDataLocations.size() == 0)
		{
			// Create/Overwrite scene registry using the new archetype of field data
			currentScene->ClearProjectComponentRegistry(event.GetAssetID());
			return;
		}

		// Generate and store new entity component data
		std::unordered_map<UUID, std::vector<uint8_t>> entityToNewDataMap{};
		for (auto& [entityID, enttID] : currentScene->m_EntityRegistry.m_EntityMap)
		{
			ECS::Entity entity = currentScene->GetEntityByEnttID(enttID);
			if (entity.HasProjectComponentData(event.GetAssetID()))
			{
				// Get old data buffer reference and generate new data buffer
				uint8_t* oldComponentData = (uint8_t*)entity.GetProjectComponentData(event.GetAssetID());
				std::vector<uint8_t> newComponentData;
				newComponentData.resize(reallocationInstructions.m_NewDataSize);

				// Transfer old data into new buffer and initialize indicated data
				for (size_t iteration{ 0 }; iteration < reallocationInstructions.m_FieldTransferDirections.size(); iteration++)
				{
					if (reallocationInstructions.m_FieldTransferDirections.at(iteration) == Assets::k_NewAllocationIndex)
					{
						// Initialize data
						Utility::InitializeDataForWrappedVarBuffer(reallocationInstructions.m_NewDataTypes.at(iteration), newComponentData.data() + reallocationInstructions.m_NewDataLocations.at(iteration));
					}
					else
					{
						// Transfer data
						Utility::TransferDataForWrappedVarBuffer(reallocationInstructions.m_NewDataTypes.at(iteration),
							oldComponentData + reallocationInstructions.m_OldDataLocations.at(reallocationInstructions.m_FieldTransferDirections.at(iteration)),
							newComponentData.data() + reallocationInstructions.m_NewDataLocations.at(iteration));
					}
				}

				// Store new data into buffer map
				entityToNewDataMap.insert_or_assign(entityID, newComponentData);
			}
		}

		// Clear old scene registry data for this project component type
		currentScene->ClearProjectComponentRegistry(event.GetAssetID());

		// Create/Overwrite scene registry using the new archetype of field data
		currentScene->AddProjectComponentRegistry(event.GetAssetID());

		// Write stored entity component data into new registry
		for (auto& [entityID, dataBuffer] : entityToNewDataMap)
		{
			ECS::Entity currentEntity = currentScene->GetEntityByUUID(entityID);
			if (!currentEntity.HasProjectComponentData(event.GetAssetID()))
			{
				currentEntity.AddProjectComponentData(event.GetAssetID());
			}
			uint8_t* currentData = (uint8_t*)currentEntity.GetProjectComponentData(event.GetAssetID());
			memcpy(currentData, dataBuffer.data(), dataBuffer.size()); // Note that the data buffer is a vector of bytes, which means size == byte size
		}
	}

	bool MainWindow::OnUpdateProjectComponent(Events::ManageAsset& event)
	{
		// Modify entity component data for all scenes in the scene asset registry and save changes to disk
		Assets::FieldReallocationInstructions& reallocationInstructions = *(Assets::FieldReallocationInstructions*)event.GetProvidedData().get();
		for (size_t iteration{ 0 }; iteration < reallocationInstructions.m_OldSceneHandles.size(); iteration++)
		{
			TransferSceneData(event, reallocationInstructions.m_OldScenes.at(iteration));
			// Save scene asset on-disk 
			Assets::AssetService::SaveScene(reallocationInstructions.m_OldSceneHandles.at(iteration), reallocationInstructions.m_OldScenes.at(iteration));
		}

		// Reset scenes
		for (Ref<Scenes::Scene> scene : reallocationInstructions.m_OldScenes)
		{
			scene.reset();
		}

		// Modify entity component data for editor scene
		if (m_EditorScene)
		{
			TransferSceneData(event, m_EditorScene);
		}

		return false;
	}

}
