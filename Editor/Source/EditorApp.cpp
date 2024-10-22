#include "kgpch.h"

#include "EditorApp.h"

#include "API/EditorUI/ImGuiBackendAPI.h"

namespace Kargono
{
	static EditorUI::GenericPopupSpec s_ExportProjectSpec {};
	static EditorUI::ChooseDirectorySpec s_ExportProjectLocation {};
	static EditorUI::CheckboxSpec s_ExportProjectServer {};

	static void InitializeStaticResources()
	{
		s_ExportProjectSpec.Label = "Export Project";
		s_ExportProjectSpec.PopupWidth = 420.0f;
		s_ExportProjectSpec.PopupContents = [&]()
		{
			EditorUI::EditorUIService::ChooseDirectory(s_ExportProjectLocation);
			EditorUI::EditorUIService::Checkbox(s_ExportProjectServer);
		};
		s_ExportProjectSpec.ConfirmAction = [&]()
		{
			Projects::ProjectService::ExportProject(s_ExportProjectLocation.CurrentOption, s_ExportProjectServer.CurrentBoolean);
		};

		s_ExportProjectLocation.Label = "Export Location";
		s_ExportProjectLocation.CurrentOption = std::filesystem::current_path().parent_path() / "Projects";

		s_ExportProjectServer.Label = "Export Server";
		s_ExportProjectServer.CurrentBoolean = true;
	}

	EditorApp* EditorApp::s_EditorApp = nullptr;

	EditorApp::EditorApp(const std::filesystem::path& projectPath)
		: Application("EditorLayer"), m_InitProjectPath(projectPath)
	{
		KG_ASSERT(!m_InitProjectPath.empty(), "Attempt to open editor without valid project path!");
		s_EditorApp = this;

		InitializeStaticResources();
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
		m_ScriptEditorPanel = CreateScope<Panels::ScriptEditorPanel>();
		m_TextEditorPanel = CreateScope<Panels::TextEditorPanel>();
		m_GameStatePanel = CreateScope<Panels::GameStatePanel>();
		m_InputModePanel = CreateScope<Panels::InputModePanel>();
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
					s_ExportProjectSpec.PopupActive = true;
				}

				ImGui::Separator();

				if (ImGui::MenuItem("New Scene"))
				{
					NewScene();
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
				ImGui::MenuItem("Input Mode Editor", NULL, &m_ShowInputModeEditor);
				ImGui::MenuItem("Script Editor", NULL, &m_ShowScriptEditor);
				ImGui::MenuItem("Text Editor", NULL, &m_ShowTextEditor);
				ImGui::MenuItem("Game State Editor", NULL, &m_ShowGameStateEditor);
				ImGui::MenuItem("Component Editor", NULL, &m_ShowProjectComponent);
				ImGui::MenuItem("AI State Editor", NULL, &m_ShowAIStateEditor);
				ImGui::Separator();
				ImGui::MenuItem("Project Settings", NULL, &m_ShowProject);
				ImGui::EndMenu();
			}

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
		if (m_ShowInputModeEditor) { m_InputModePanel->OnEditorUIRender(); }
		if (m_ShowProperties) { m_PropertiesPanel->OnEditorUIRender(); }
		if (m_ShowDemoWindow) { ImGui::ShowDemoWindow(&m_ShowDemoWindow); }
		if (m_ShowTesting) { m_TestingPanel->OnEditorUIRender(); }
		if (m_ShowAIStateEditor) { m_AIStatePanel->OnEditorUIRender(); }

		EditorUI::EditorUIService::GenericPopup(s_ExportProjectSpec);

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
		return Input::InputModeService::OnKeyPressed(event);
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
		std::string focusedWindow = EditorUI::EditorUIService::GetFocusedWindowName();
		if (focusedWindow == m_ViewportPanel->m_PanelName)
		{
			m_ViewportPanel->OnInputEvent(event);
		}

		if (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Simulate)
		{
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
		if (event->GetEventType() == Events::EventType::ManageAsset)
		{
			Events::ManageAsset* manageAsset = (Events::ManageAsset*)event;

			// Handle adding a project component to the active editor scene
			if (manageAsset->GetAssetType() == Assets::AssetType::ProjectComponent &&
				manageAsset->GetAction() == Events::ManageAssetAction::Create)
			{
				// Create project component inside scene registry
				if (m_EditorScene)
				{
					m_EditorScene->AddProjectComponentRegistry(manageAsset->GetAssetID());
				}
			}
			// Handle editing a project component by modifying entity component data inside the Assets::AssetService::SceneRegistry and the active editor scene
			if (manageAsset->GetAssetType() == Assets::AssetType::ProjectComponent &&
				manageAsset->GetAction() == Events::ManageAssetAction::Update)
			{
				OnUpdateProjectComponent(*manageAsset);
			}
			// Handle deleting a project component by removing entity data from all scenes
			if (manageAsset->GetAssetType() == Assets::AssetType::ProjectComponent &&
				manageAsset->GetAction() == Events::ManageAssetAction::Delete)
			{
				for (auto& [sceneHandle, sceneAsset] : Assets::AssetService::GetSceneRegistry())
				{
					// Get scene
					Ref<Scenes::Scene> currentScene = Assets::AssetService::GetScene(sceneHandle);
					KG_ASSERT(currentScene);

					// Clear component registry
					currentScene->ClearProjectComponentRegistry(manageAsset->GetAssetID());

					// Save scene asset on-disk 
					Assets::AssetService::SaveScene(sceneHandle, currentScene);
				}
			}
		}

		m_SceneEditorPanel->OnAssetEvent(event);
		m_AssetViewerPanel->OnAssetEvent(event);

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

		std::string focusedWindow = EditorUI::EditorUIService::GetFocusedWindowName();
		if (m_PanelToKeyboardInput.contains(focusedWindow))
		{
			if (m_PanelToKeyboardInput.at(focusedWindow)(event))
			{
				return true;
			}
		}

		bool control = Input::InputService::IsKeyPressed(Key::LeftControl) || Input::InputService::IsKeyPressed(Key::RightControl);
		bool shift = Input::InputService::IsKeyPressed(Key::LeftShift) || Input::InputService::IsKeyPressed(Key::RightShift);
		bool alt = Input::InputService::IsKeyPressed(Key::LeftAlt) || Input::InputService::IsKeyPressed(Key::RightAlt);

		switch (event.GetKeyCode())
		{

			case Key::N:
			{
				if (control) { NewScene(); }
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
		if (event.GetMouseButton() == Mouse::ButtonLeft)
		{
			if (m_ViewportPanel->m_ViewportHovered && !ImGuizmo::IsOver() && !Input::InputService::IsKeyPressed(Key::LeftAlt))
			{
				if (*Scenes::SceneService::GetActiveScene()->GetHoveredEntity())
				{
					m_SceneEditorPanel->SetSelectedEntity(*Scenes::SceneService::GetActiveScene()->GetHoveredEntity());
					s_EditorApp->m_SceneEditorPanel->SetDisplayedComponent(ECS::ComponentType::None);
					// Algorithm to enable double clicking for an entity!
					static float previousTime{ 0.0f };
					static ECS::Entity previousEntity{};
					float currentTime = Utility::Time::GetTime();
					if (std::fabs(currentTime - previousTime) < 0.2f && *Scenes::SceneService::GetActiveScene()->GetHoveredEntity() == previousEntity)
					{
						auto& transformComponent = Scenes::SceneService::GetActiveScene()->GetHoveredEntity()->GetComponent<ECS::TransformComponent>();
						m_ViewportPanel->m_EditorCamera.SetFocalPoint(transformComponent.Translation);
						m_ViewportPanel->m_EditorCamera.SetDistance(std::max({ transformComponent.Scale.x, transformComponent.Scale.y, transformComponent.Scale.z }) * 2.5f);
						m_ViewportPanel->m_EditorCamera.SetMovementType(Rendering::EditorCamera::MovementType::ModelView);
					}
					previousTime = currentTime;
					previousEntity = *Scenes::SceneService::GetActiveScene()->GetHoveredEntity();
				}
				

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
		m_InputModePanel->ResetPanelResources();
		m_GameStatePanel->ResetPanelResources();
		m_ScriptEditorPanel->ResetPanelResources();
		m_ProjectPanel->ResetPanelResources();
		Scenes::GameStateService::ClearActiveGameState();
		Input::InputModeService::ClearActiveInputMode();

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

	void EditorApp::NewScene()
	{
		std::filesystem::path initialDirectory = Projects::ProjectService::GetActiveAssetDirectory() / "Scenes";
		std::filesystem::path filepath = Utility::FileDialogs::SaveFile("Kargono Scene (*.kgscene)\0*.kgscene\0", initialDirectory.string().c_str());
		if (filepath.empty()) { return; }
		if (Assets::AssetService::HasScene(filepath.stem().string()))
		{
			KG_WARN("Attempt to create scene with duplicate name!");
			return;
		}
		m_EditorSceneHandle = Assets::AssetService::CreateScene(filepath.stem().string());

		*Scenes::SceneService::GetActiveScene()->GetHoveredEntity() = {};
		m_EditorScene = Assets::AssetService::GetScene(m_EditorSceneHandle);
		Scenes::SceneService::SetActiveScene(m_EditorScene, m_EditorSceneHandle);
	}

	void EditorApp::NewScene(const std::string& sceneName)
	{
		std::filesystem::path filepath = Projects::ProjectService::GetActiveAssetDirectory() / ("Scenes/" + sceneName + ".kgscene");
		if (Assets::AssetService::HasScene(filepath.stem().string()))
		{
			KG_WARN("Attempt to create scene with duplicate name!");
			return;
		}
		m_EditorSceneHandle = Assets::AssetService::CreateScene(filepath.stem().string());

		*Scenes::SceneService::GetActiveScene()->GetHoveredEntity() = {};
		m_EditorScene = Assets::AssetService::GetScene(m_EditorSceneHandle);
		Scenes::SceneService::SetActiveScene(m_EditorScene, m_EditorSceneHandle);
	}

	void EditorApp::OpenScene()
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
		Scenes::SceneService::SetActiveScene(m_EditorScene, m_EditorSceneHandle);
		m_EditorSceneHandle = sceneHandle;

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
		// Cache Current InputMode in editor
		if (!Input::InputModeService::GetActiveInputMode())
		{ 
			m_EditorInputMode = nullptr; 
		}
		else
		{
			m_EditorInputMode = Input::InputModeService::GetActiveInputMode();
			m_EditorInputModeHandle = Input::InputModeService::GetActiveInputModeHandle();
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

		// Clear InputModes during runtime.
		if (m_EditorInputMode)
		{
			Input::InputModeService::SetActiveInputMode(m_EditorInputMode, m_EditorInputModeHandle);
		}
		else
		{
			Input::InputModeService::SetActiveInputMode(nullptr, Assets::EmptyHandle);
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
