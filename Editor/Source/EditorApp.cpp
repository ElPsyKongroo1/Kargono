#include "kgpch.h"

#include "EditorApp.h"

#include "API/EditorUI/ImGuiBackendAPI.h"

namespace Kargono
{

	void EditorApp::InitializeExportProjectWidgets()
	{
		m_ExportProjectSpec.Label = "Export Project";
		m_ExportProjectSpec.PopupWidth = 420.0f;
		m_ExportProjectSpec.PopupContents = [&]()
		{
			EditorUI::EditorUIService::ChooseDirectory(m_ExportProjectLocation);
			EditorUI::EditorUIService::Checkbox(m_ExportProjectServer);
		};
		m_ExportProjectSpec.ConfirmAction = [&]()
		{
			Projects::ProjectService::ExportProject(m_ExportProjectLocation.CurrentOption, m_ExportProjectServer.CurrentBoolean);
		};

		m_ExportProjectLocation.Label = "Export Location";
		m_ExportProjectLocation.CurrentOption = std::filesystem::current_path().parent_path() / "Projects";

		m_ExportProjectServer.Label = "Export Server";
		m_ExportProjectServer.CurrentBoolean = true;
	}

	void EditorApp::InitializeImportAssetWidgets()
	{

		// Initialize import asset popup data
		m_ImportAssetPopup.Label = "Import Asset";
		m_ImportAssetPopup.PopupContents = [&]() 
		{
			EditorUI::EditorUIService::LabeledText("Source File:", m_ImportSourceFilePath.string());
			EditorUI::EditorUIService::ChooseDirectory(m_ImportNewFileLocation);
			EditorUI::EditorUIService::EditText(m_ImportNewAssetName);
		};
		m_ImportAssetPopup.ConfirmAction = [&]() 
		{
			// Actually import the asset
			switch (m_ImportAssetType)
			{
			case Assets::AssetType::Font:
				Assets::AssetService::ImportFontFromFile(m_ImportSourceFilePath, m_ImportNewAssetName.CurrentOption.c_str(), m_ImportNewFileLocation.CurrentOption);
				break;
			case Assets::AssetType::Audio:
				Assets::AssetService::ImportAudioBufferFromFile(m_ImportSourceFilePath, m_ImportNewAssetName.CurrentOption.c_str(), m_ImportNewFileLocation.CurrentOption);
				break;
			case Assets::AssetType::Texture:
				Assets::AssetService::ImportTexture2DFromFile(m_ImportSourceFilePath, m_ImportNewAssetName.CurrentOption.c_str(), m_ImportNewFileLocation.CurrentOption);
				break;
			default:
				KG_ERROR("Unsupported asset type attempting to be imported");
				break;
			}
		};

		// Initialize internal popup widgets
		m_ImportNewAssetName.Label = "New Asset Name:";
		m_ImportNewAssetName.CurrentOption = "NewAsset";

		m_ImportNewFileLocation.Label = "Destination Folder:";
		m_ImportNewFileLocation.ConfirmAction = [&](const std::string& path)
		{
			if (!Utility::FileSystem::DoesPathContainSubPath(Projects::ProjectService::GetActiveAssetDirectory(), path))
			{
				KG_WARN("Cannot create an asset outside of the project's asset directory.");
				m_ImportNewFileLocation.CurrentOption = Projects::ProjectService::GetActiveAssetDirectory();
			}
		};
	}

	EditorApp* EditorApp::s_EditorApp = nullptr;

	EditorApp::EditorApp(const std::filesystem::path& projectPath)
		: Application("EditorLayer"), m_InitProjectPath(projectPath)
	{
		KG_ASSERT(!m_InitProjectPath.empty(), "Attempt to open editor without valid project path!");
		s_EditorApp = this;

		InitializeExportProjectWidgets();
		InitializeImportAssetWidgets();

		m_GeneralWarningSpec.Label = "Warning";
		m_GeneralWarningSpec.PopupContents = [&]()
		{
			EditorUI::EditorUIService::LabeledText("Warning Message:", m_GeneralWarningMessage.CString());
		};
	}


	void EditorApp::Init()
	{
		Scripting::ScriptService::Init();
		Audio::AudioService::Init();
		Scenes::SceneService::Init();

		m_EditorScene = CreateRef<Scenes::Scene>();
		Scenes::SceneService::SetActiveScene(m_EditorScene, m_EditorSceneHandle);
		m_SceneState = SceneState::Edit;

		OpenProject(m_InitProjectPath);

		EditorUI::EditorUIService::Init();
		AI::AIService::Init();

		m_SceneEditorPanel = CreateScope<Panels::SceneEditorPanel>();
		m_AssetViewerPanel = CreateScope<Panels::AssetViewerPanel>();
		m_LogPanel = CreateScope<Panels::LogPanel>();
		m_StatisticsPanel = CreateScope<Panels::StatisticsPanel>();
		m_ProjectPanel = CreateScope<Panels::ProjectPanel>();
		m_ProjectComponentPanel = CreateScope<Panels::ProjectComponentPanel>();
		m_UIEditorPanel = CreateScope<Panels::UIEditorPanel>();
		m_ViewportPanel = CreateScope<Panels::ViewportPanel>();
		m_TextEditorPanel = CreateScope<Panels::TextEditorPanel>();
		m_ScriptEditorPanel = CreateScope<Panels::ScriptEditorPanel>();
		m_GameStatePanel = CreateScope<Panels::GameStatePanel>();
		m_InputMapPanel = CreateScope<Panels::InputMapPanel>();
		m_ContentBrowserPanel = CreateScope<Panels::ContentBrowserPanel>();
		m_PropertiesPanel = CreateScope<Panels::PropertiesPanel>();
		m_AIStatePanel = CreateScope<Panels::AIStateEditorPanel>();
		m_TestingPanel = CreateScope<Panels::TestingPanel>();

		m_ViewportPanel->InitializeFrameBuffer();

		Rendering::RenderingService::Init();
		Rendering::RenderingService::SetLineWidth(1.0f);
		RuntimeUI::FontService::Init();
		RuntimeUI::RuntimeUIService::Init();

		m_ViewportPanel->m_EditorCamera = Rendering::EditorCamera(30.0f, 1.778f, 0.1f, 1000.0f);

		m_ViewportPanel->m_EditorCamera.SetFocalPoint({ 58.0f, 27.0f, 93.0f });
		m_ViewportPanel->m_EditorCamera.SetDistance(1.0f);
		m_ViewportPanel->m_EditorCamera.SetPitch(0.195f);
		m_ViewportPanel->m_EditorCamera.SetYaw(-0.372f);
		
		m_ViewportPanel->InitializeOverlayData();

		EngineService::GetActiveWindow().SetVisible(true);
	}

	void EditorApp::Terminate()
	{
		EditorUI::EditorUIService::Terminate();
		Audio::AudioService::Terminate();
		Scripting::ScriptService::Terminate();
		AI::AIService::Terminate();
	}

	void EditorApp::OnUpdate(Timestep ts)
	{
		KG_PROFILE_FUNCTION();

		m_ViewportPanel->OnUpdate(ts);

		OnEditorUIRender();
	}

	void EditorApp::Step(int frames)
	{
		m_StepFrames = frames;
	}


	void EditorApp::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION();
		EditorUI::EditorUIService::StartRendering();

		static bool dockspaceOpen = true;
		static bool opt_fullscreen = true;
		static bool opt_padding = false;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}
		else
		{
			dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
		}

		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
		// and handle the pass-thru hole, so we ask Begin() to not render a background.
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		dockspace_flags |= ImGuiDockNodeFlags_NoCloseButton;
		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
		if (!opt_padding)
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		EditorUI::EditorUIService::StartWindow("DockSpace", &dockspaceOpen, window_flags);
		if (!opt_padding)
			ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// Set up Menu Toolbar
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open Project"))
				{
					OpenProject();
				}

				if (ImGui::MenuItem("Save Project", "Ctrl+S"))
				{
					SaveScene();
					SaveProject();
				}

				if (ImGui::MenuItem("Export Project"))
				{
					m_ExportProjectSpec.PopupActive = true;
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

			if (ImGui::BeginMenu("Panels"))
			{
				ImGui::MenuItem("Asset Viewer", NULL, &m_ShowAssetViewer);
				ImGui::MenuItem("Content Browser", NULL, &m_ShowContentBrowser);
				ImGui::MenuItem("Scene Hierarchy", NULL, &m_ShowSceneHierarchy);
				ImGui::MenuItem("Viewport", NULL, &m_ShowViewport);
				ImGui::MenuItem("Properties", NULL, &m_ShowProperties);
				ImGui::Separator();
				ImGui::MenuItem("User Interface Editor", NULL, &m_ShowUserInterfaceEditor);
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
			// TODO: Fullscreen for select panels
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

				ImGui::MenuItem("Testing Window",NULL, &m_ShowTesting);
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

			ImGui::EndMenuBar();
		}

		// Submit the DockSpace
		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		float minWinSizeX = style.WindowMinSize.x;
		style.WindowMinSize.x = 420.0f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		style.WindowMinSize.x = minWinSizeX;

		if (m_RuntimeFullscreen && (m_SceneState == SceneState::Play || m_SceneState == SceneState::Simulate) && !m_IsPaused)
		{
			if (m_ShowViewport) { m_ViewportPanel->OnEditorUIRender(); }
			EditorUI::EditorUIService::EndWindow();
			EditorUI::EditorUIService::EndRendering();
			return;
		}
		
#if 0
		/*if (m_ContentBrowserFullscreen)
		{
			m_ContentBrowserPanel->OnEditorUIRender();
			EditorUI::EditorUIService::EndWindow();
			EditorUI::EditorUIService::EndRendering();
			return;
		}*/
#endif

		// Display other panels
		if (m_ShowAssetViewer) { m_AssetViewerPanel->OnEditorUIRender(); }
		if (m_ShowSceneHierarchy) { m_SceneEditorPanel->OnEditorUIRender(); }
		if (m_ShowContentBrowser) { m_ContentBrowserPanel->OnEditorUIRender(); }
		if (m_ShowLog) { m_LogPanel->OnEditorUIRender(); }
		if (m_ShowStats) { m_StatisticsPanel->OnEditorUIRender(); }
		if (m_ShowViewport) { m_ViewportPanel->OnEditorUIRender(); }
		if (m_ShowUserInterfaceEditor) { m_UIEditorPanel->OnEditorUIRender(); }
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

		EditorUI::EditorUIService::GenericPopup(m_ExportProjectSpec);
		EditorUI::EditorUIService::GenericPopup(m_ImportAssetPopup);
		EditorUI::EditorUIService::WarningPopup(m_GeneralWarningSpec);

		EditorUI::EditorUIService::EndWindow();

		EditorUI::EditorUIService::HighlightFocusedWindow();

		EditorUI::EditorUIService::EndRendering();
	}

	bool EditorApp::OnApplicationClose(Events::ApplicationCloseEvent event)
	{
		OnStop();
		return false;
	}

	bool EditorApp::OnKeyPressedRuntime(Events::KeyPressedEvent event)
	{
		KG_PROFILE_FUNCTION()
		return Input::InputMapService::OnKeyPressed(event);
	}

	bool EditorApp::OnApplicationEvent(Events::Event* event)
	{
		bool handled = false;

		if (m_SceneState == SceneState::Play)
		{
			if (event->GetEventType() == Events::EventType::AppClose)
			{
				handled = OnApplicationClose(*(Events::ApplicationCloseEvent*)event);
			}
		}
		return handled;
	}

	bool EditorApp::OnNetworkEvent(Events::Event* event)
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

	bool EditorApp::OnInputEvent(Events::Event* event)
	{
		bool handled = false;
		handled = EditorUI::EditorUIService::OnInputEvent(event);
		if (handled)
		{
			return true;
		}
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

			if (event->GetEventType() == Events::EventType::KeyPressed)
			{
				handled = OnKeyPressedRuntime(*(Events::KeyPressedEvent*)event);
			}
		}

		return handled;
	}

	bool EditorApp::OnPhysicsEvent(Events::Event* event)
	{
		bool handled = false;
		switch (event->GetEventType())
		{
		case Events::EventType::PhysicsCollisionStart:
			handled = OnPhysicsCollisionStart(*(Events::PhysicsCollisionStart*)event);
			break;
		case Events::EventType::PhysicsCollisionEnd:
			handled = OnPhysicsCollisionEnd(*(Events::PhysicsCollisionEnd*)event);
			break;
		}
		return handled;
	}

	bool EditorApp::OnSceneEvent(Events::Event* event)
	{
		return m_SceneEditorPanel->OnSceneEvent(event);
	}

	bool EditorApp::OnAssetEvent(Events::Event* event)
	{
		Events::ManageAsset& manageAsset = *(Events::ManageAsset*)event;

		// Handle adding a project component to the active editor scene
		if (manageAsset.GetAssetType() == Assets::AssetType::ProjectComponent &&
			manageAsset.GetAction() == Events::ManageAssetAction::Create)
		{
			// Create project component inside scene registry
			if (m_EditorScene)
			{
				m_EditorScene->AddProjectComponentRegistry(manageAsset.GetAssetID());
			}
		}
		// Handle editing a project component by modifying entity component data inside the Assets::AssetService::SceneRegistry and the active editor scene
		if (manageAsset.GetAssetType() == Assets::AssetType::ProjectComponent &&
			manageAsset.GetAction() == Events::ManageAssetAction::UpdateAsset)
		{
			OnUpdateProjectComponent(manageAsset);
		}
		// Handle deleting a project component by removing entity data from all scenes
		if (manageAsset.GetAssetType() == Assets::AssetType::ProjectComponent &&
			manageAsset.GetAction() == Events::ManageAssetAction::Delete)
		{
			for (auto& [sceneHandle, sceneAsset] : Assets::AssetService::GetSceneRegistry())
			{
				// Get scene
				Ref<Scenes::Scene> currentScene = Assets::AssetService::GetScene(sceneHandle);
				KG_ASSERT(currentScene);

				// Clear component registry
				currentScene->ClearProjectComponentRegistry(manageAsset.GetAssetID());

				// Save scene asset on-disk 
				Assets::AssetService::SaveScene(sceneHandle, currentScene);
			}
		}


		m_SceneEditorPanel->OnAssetEvent(event);
		m_AssetViewerPanel->OnAssetEvent(event);

		if (manageAsset.GetAssetType() == Assets::AssetType::Scene && 
			manageAsset.GetAssetID() == m_EditorSceneHandle)
		{
			if (manageAsset.GetAction() == Events::ManageAssetAction::Delete)
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

		switch (manageAsset.GetAssetType())
		{
		case Assets::AssetType::GameState:
			m_GameStatePanel->OnAssetEvent(event);
			break;
		case Assets::AssetType::AIState:
			m_AIStatePanel->OnAssetEvent(event);
			break;
		case Assets::AssetType::InputMap:
			m_InputMapPanel->OnAssetEvent(event);
			break;
		case Assets::AssetType::ProjectComponent:
			m_ProjectComponentPanel->OnAssetEvent(event);
			break;
		case Assets::AssetType::Script:
			m_ScriptEditorPanel->OnAssetEvent(event);
			break;
		case Assets::AssetType::UserInterface:
			m_UIEditorPanel->OnAssetEvent(event);
			break;
		default:
			break;
		}
		//NewScene("NewScene");
		

		return false;
	}

	bool EditorApp::OnEditorEvent(Events::Event* event)
	{
		m_ViewportPanel->OnEditorEvent(event);
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

	bool EditorApp::OnUpdateProjectComponent(Events::ManageAsset& event)
	{
		// Modify entity component data for all scenes in the scene asset registry and save changes to disk
		Assets::FieldReallocationInstructions& reallocationInstructions = *(Assets::FieldReallocationInstructions*)event.GetProvidedData().get();
		for (size_t iteration{0}; iteration < reallocationInstructions.m_OldSceneHandles.size(); iteration++)
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

	bool EditorApp::OnKeyPressedEditor(Events::KeyPressedEvent event)
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
				if (control) { OpenProject(); }
				break;
			}
			case Key::S:
			{
				if (control)
				{
					SaveScene();
					SaveProject();
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

	bool EditorApp::OnMouseButtonPressed(Events::MouseButtonPressedEvent event)
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


	bool EditorApp::OnPhysicsCollisionStart(Events::PhysicsCollisionStart event)
	{
		Ref<Scenes::Scene> activeScene = Scenes::SceneService::GetActiveScene();
		UUID entityOneID = event.GetEntityOne();
		ECS::Entity entityOne = activeScene->GetEntityByUUID(entityOneID);
		UUID entityTwoID = event.GetEntityTwo();
		ECS::Entity entityTwo = activeScene->GetEntityByUUID(entityTwoID);

		KG_ASSERT(entityOne);
		KG_ASSERT(entityTwo);

		bool collisionHandled = false;
		if (entityOne.HasComponent<ECS::Rigidbody2DComponent>())
		{
			ECS::Rigidbody2DComponent& component = entityOne.GetComponent<ECS::Rigidbody2DComponent>();
			Assets::AssetHandle scriptHandle = component.OnCollisionStartScriptHandle;
			Scripting::Script* script = component.OnCollisionStartScript.get();
			if (scriptHandle != Assets::EmptyHandle)
			{
				collisionHandled = Utility::CallWrappedBoolUInt64UInt64(script->m_Function, entityOneID, entityTwoID);
			}
		}

		if (!collisionHandled && entityTwo.HasComponent<ECS::Rigidbody2DComponent>())
		{
			ECS::Rigidbody2DComponent& component = entityTwo.GetComponent<ECS::Rigidbody2DComponent>();
			Assets::AssetHandle scriptHandle = component.OnCollisionStartScriptHandle;
			Scripting::Script* script = component.OnCollisionStartScript.get();
			if (scriptHandle != Assets::EmptyHandle)
			{
				collisionHandled = Utility::CallWrappedBoolUInt64UInt64(script->m_Function, entityTwoID, entityOneID);
			}
		}
		return false;
	}

	bool EditorApp::OnPhysicsCollisionEnd(Events::PhysicsCollisionEnd event)
	{
		Ref<Scenes::Scene> activeScene = Scenes::SceneService::GetActiveScene();
		UUID entityOneID = event.GetEntityOne();
		ECS::Entity entityOne = activeScene->GetEntityByUUID(entityOneID);
		UUID entityTwoID = event.GetEntityTwo();
		ECS::Entity entityTwo = activeScene->GetEntityByUUID(entityTwoID);

		KG_ASSERT(entityOne);
		KG_ASSERT(entityTwo);

		bool collisionHandled = false;
		if (entityOne.HasComponent<ECS::Rigidbody2DComponent>())
		{
			ECS::Rigidbody2DComponent& component = entityOne.GetComponent<ECS::Rigidbody2DComponent>();
			Assets::AssetHandle scriptHandle = component.OnCollisionEndScriptHandle;
			Scripting::Script* script = component.OnCollisionEndScript.get();
			if (scriptHandle != Assets::EmptyHandle)
			{
				Utility::CallWrappedBoolUInt64UInt64(script->m_Function, entityOneID, entityTwoID);
			}
		}

		if (!collisionHandled && entityOne.HasComponent<ECS::Rigidbody2DComponent>())
		{
			ECS::Rigidbody2DComponent& component = entityTwo.GetComponent<ECS::Rigidbody2DComponent>();
			Assets::AssetHandle scriptHandle =  component.OnCollisionEndScriptHandle;
			Scripting::Script* script = component.OnCollisionEndScript.get();
			if (scriptHandle != Assets::EmptyHandle)
			{
			 	collisionHandled = Utility::CallWrappedBoolUInt64UInt64(script->m_Function, entityTwoID, entityOneID);
			}
		}
		return false;
	}

	bool EditorApp::OnUpdateUserCount(Events::UpdateOnlineUsers event)
	{
		Assets::AssetHandle scriptHandle = Projects::ProjectService::GetActiveOnUpdateUserCount();
		if (scriptHandle != Assets::EmptyHandle)
		{
			Utility::CallWrappedVoidUInt32(Assets::AssetService::GetScript(scriptHandle)->m_Function, event.GetUserCount());
		}

		return false;
	}

	bool EditorApp::OnApproveJoinSession(Events::ApproveJoinSession event)
	{
		Assets::AssetHandle scriptHandle = Projects::ProjectService::GetActiveOnApproveJoinSession();
		if (scriptHandle != Assets::EmptyHandle)
		{
			Utility::CallWrappedVoidUInt16(Assets::AssetService::GetScript(scriptHandle)->m_Function, event.GetUserSlot());
		}

		return false;
	}

	bool EditorApp::OnUpdateSessionUserSlot(Events::UpdateSessionUserSlot event)
	{
		Assets::AssetHandle scriptHandle = Projects::ProjectService::GetActiveOnUpdateSessionUserSlot();
		if (scriptHandle != Assets::EmptyHandle)
		{
			Utility::CallWrappedVoidUInt16(Assets::AssetService::GetScript(scriptHandle)->m_Function, event.GetUserSlot());
		}

		return false;
	}

	bool EditorApp::OnUserLeftSession(Events::UserLeftSession event)
	{
		Assets::AssetHandle scriptHandle = Projects::ProjectService::GetActiveOnUserLeftSession();
		if (scriptHandle != Assets::EmptyHandle)
		{
			Utility::CallWrappedVoidUInt16(Assets::AssetService::GetScript(scriptHandle)->m_Function, event.GetUserSlot());
		}
		return false;
	}

	bool EditorApp::OnCurrentSessionInit(Events::CurrentSessionInit event)
	{
		Assets::AssetHandle scriptHandle = Projects::ProjectService::GetActiveOnCurrentSessionInit();
		if (scriptHandle != Assets::EmptyHandle)
		{
			Utility::CallWrappedVoidNone(Assets::AssetService::GetScript(scriptHandle)->m_Function);
		}
		return false;
	}

	bool EditorApp::OnConnectionTerminated(Events::ConnectionTerminated event)
	{
		Assets::AssetHandle scriptHandle = Projects::ProjectService::GetActiveOnConnectionTerminated();
		if (scriptHandle != Assets::EmptyHandle)
		{
			Utility::CallWrappedVoidNone(Assets::AssetService::GetScript(scriptHandle)->m_Function);
		}
		return false;
	}

	bool EditorApp::OnStartSession(Events::StartSession event)
	{
		Assets::AssetHandle scriptHandle = Projects::ProjectService::GetActiveOnStartSession();
		if (scriptHandle != Assets::EmptyHandle)
		{
			Utility::CallWrappedVoidNone(Assets::AssetService::GetScript(scriptHandle)->m_Function);
		}
		return false;
	}

	bool EditorApp::OnSessionReadyCheckConfirm(Events::SessionReadyCheckConfirm event)
	{
		Assets::AssetHandle scriptHandle = Projects::ProjectService::GetActiveOnSessionReadyCheckConfirm();
		if (scriptHandle != Assets::EmptyHandle)
		{
			Utility::CallWrappedVoidNone(Assets::AssetService::GetScript(scriptHandle)->m_Function);
		}
		return false;
	}

	bool EditorApp::OnReceiveSignal(Events::ReceiveSignal event)
	{
		Assets::AssetHandle scriptHandle = Projects::ProjectService::GetActiveOnReceiveSignal();
		if (scriptHandle != Assets::EmptyHandle)
		{
			Utility::CallWrappedVoidUInt16(Assets::AssetService::GetScript(scriptHandle)->m_Function, event.GetSignal());
		}
		return false;
	}

	void EditorApp::OpenWarningMessage(const char* message)
	{
		// Open generic message
		m_GeneralWarningMessage = message;
		m_GeneralWarningSpec.PopupActive = true;
	}

	void EditorApp::OpenImportFileDialog(const std::filesystem::path& importFileLocation, Assets::AssetType assetType)
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
		m_ImportNewFileLocation.CurrentOption = importFileLocation.parent_path();

		// Add default name to name widget
		m_ImportNewAssetName.CurrentOption = "NewAsset";

		// Store asset type
		m_ImportAssetType = assetType;

		// Open Popup and change title
		m_ImportAssetPopup.PopupActive = true;
		m_ImportAssetPopup.Label = "Import " + Utility::AssetTypeToString(assetType);
	}


	bool EditorApp::OpenProject()
	{
		*Scenes::SceneService::GetActiveScene()->GetHoveredEntity() = {};
		std::filesystem::path initialDirectory = std::filesystem::current_path().parent_path() / "Projects";
		if (!std::filesystem::exists(initialDirectory))
		{
			initialDirectory = "";
		}
		std::filesystem::path filepath = Utility::FileDialogs::OpenFile("Kargono Project (*.kproj)\0*.kproj\0", initialDirectory.string().c_str());
		if (filepath.empty())
		{
			return false;
		}

		OpenProject(filepath);

		// Reset all resources
		m_ContentBrowserPanel->ResetPanelResources();
		m_TextEditorPanel->ResetPanelResources();
		m_InputMapPanel->ResetPanelResources();
		m_GameStatePanel->ResetPanelResources();
		m_ScriptEditorPanel->ResetPanelResources();
		m_ProjectPanel->ResetPanelResources();
		Scenes::GameStateService::ClearActiveGameState();
		Input::InputMapService::ClearActiveInputMap();

		return true;
	}

	void EditorApp::OpenProject(const std::filesystem::path& path)
	{
		if (Projects::ProjectService::OpenProject(path))
		{
			if (!EngineService::GetActiveWindow().GetNativeWindow())
			{
				EngineService::GetActiveWindow().Init();
				Rendering::RendererAPI::Init();
			}
			auto startSceneHandle = Projects::ProjectService::GetActiveStartSceneHandle();

			Scripting::ScriptService::LoadActiveScriptModule();

			if (m_EditorScene)
			{
				m_EditorScene->DestroyAllEntities();
			}
			Assets::AssetService::ClearAll();
			Assets::AssetService::DeserializeAll();
			if (startSceneHandle == Assets::EmptyHandle)
			{
				NewScene("NewScene");
				Projects::ProjectService::SetActiveStartingScene(m_EditorSceneHandle);
				SaveProject();
			}
			else
			{
				OpenScene(startSceneHandle);
			}
		}
	}

	void EditorApp::SaveProject()
	{
		Projects::ProjectService::SaveActiveProject((Projects::ProjectService::GetActiveProjectDirectory() / Projects::ProjectService::GetActiveProjectName()).replace_extension(".kproj"));
	}

	void EditorApp::NewSceneDialog()
	{
		NewSceneDialog(Projects::ProjectService::GetActiveAssetDirectory());
	}

	bool EditorApp::NewScene(const std::string& sceneName)
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

	void EditorApp::NewSceneDialog(const std::filesystem::path& initialDirectory)
	{
		// Provide file dialog to select location and name of scene
		std::filesystem::path filepath = Utility::FileDialogs::SaveFile("Kargono Scene (*.kgscene)\0*.kgscene\0", initialDirectory.string().c_str());
		if (filepath.empty()) { return; }
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
		
	}

	void EditorApp::OpenSceneDialog()
	{
		std::filesystem::path initialDirectory = Projects::ProjectService::GetActiveAssetDirectory();
		std::filesystem::path filepath = Utility::FileDialogs::OpenFile("Kargono Scene (*.kgscene)\0*.kgscene\0", initialDirectory.string().c_str());
		
		if (!filepath.empty())
		{
			OpenScene(filepath);
		}
	}

	void EditorApp::OpenScene(const std::filesystem::path& path)
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

	}

	void EditorApp::OpenScene(Assets::AssetHandle sceneHandle)
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
	}

	void EditorApp::SaveScene()
	{
		SerializeScene(m_EditorScene);
	}

	void EditorApp::SerializeScene(Ref<Scenes::Scene> scene)
	{
		Assets::AssetService::SaveScene(m_EditorSceneHandle, scene);
		
	}

	void EditorApp::OnPlay()
	{
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
		if (Projects::ProjectService::GetActiveStartGameState() == 0)
		{
			Scenes::GameStateService::ClearActiveGameState();
		}
		else
		{
			Scenes::GameStateService::SetActiveGameState(
				Assets::AssetService::GetGameState(Projects::ProjectService::GetActiveStartGameState()),
				Projects::ProjectService::GetActiveStartGameState());
		}

		*Scenes::SceneService::GetActiveScene()->GetHoveredEntity() = {};
		if (m_SceneState == SceneState::Simulate) { OnStop(); }

		m_SceneState = SceneState::Play;
		Scenes::SceneService::SetActiveScene(Scenes::SceneService::CreateSceneCopy(m_EditorScene), m_EditorSceneHandle);
		Physics::Physics2DService::Init(Scenes::SceneService::GetActiveScene().get(), Scenes::SceneService::GetActiveScene()->m_PhysicsSpecification);
		Scenes::SceneService::GetActiveScene()->OnRuntimeStart();
		Assets::AssetHandle scriptHandle = Projects::ProjectService::GetActiveOnRuntimeStart();
		if (scriptHandle != 0)
		{
			Utility::CallWrappedVoidNone(Assets::AssetService::GetScript(scriptHandle)->m_Function);
		}

		if (Projects::ProjectService::GetActiveAppIsNetworked())
		{
			Network::ClientService::Init();
		}

		AppTickService::LoadGeneratorsFromProject();
		EngineService::GetActiveEngine().UpdateAppStartTime();
		EditorUI::EditorUIService::SetFocusedWindow(m_ViewportPanel->m_PanelName);
	}

	void EditorApp::OnSimulate()
	{
		*Scenes::SceneService::GetActiveScene()->GetHoveredEntity() = {};
		if (m_SceneState == SceneState::Play) { OnStop(); }

		m_SceneState = SceneState::Simulate;
		Scenes::SceneService::SetActiveScene(Scenes::SceneService::CreateSceneCopy(m_EditorScene), m_EditorSceneHandle);
		Physics::Physics2DService::Init(Scenes::SceneService::GetActiveScene().get(), Scenes::SceneService::GetActiveScene()->m_PhysicsSpecification);
	}
	void EditorApp::OnStop()
	{
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

		// Clear UIObjects during runtime.
		if (m_UIEditorPanel->m_EditorUI)
		{
			RuntimeUI::RuntimeUIService::SetActiveUI(m_UIEditorPanel->m_EditorUI, m_UIEditorPanel->m_EditorUIHandle);
		}
		else
		{
			RuntimeUI::RuntimeUIService::ClearActiveUI();
		}

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

		AppTickService::ClearGenerators();

		m_SceneState = SceneState::Edit;
	}

	void EditorApp::OnPause()
	{
		if (m_SceneState == SceneState::Edit) { return; }

		m_IsPaused = true;
	}

	void EditorApp::OnDuplicateEntity()
	{
		if (m_SceneState != SceneState::Edit)
			return;

		ECS::Entity selectedEntity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
		if (selectedEntity)
		{
			ECS::Entity newEntity = m_EditorScene->DuplicateEntity(selectedEntity);
			m_SceneEditorPanel->SetSelectedEntity(newEntity);
			s_EditorApp->m_SceneEditorPanel->SetDisplayedComponent(ECS::ComponentType::None);
		}
	}

}
