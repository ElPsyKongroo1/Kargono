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
			Projects::ProjectService::ExportProject(s_ExportProjectLocation.CurrentOption, s_ExportProjectServer.ToggleBoolean);
		};

		s_ExportProjectLocation.Label = "Export Location";
		s_ExportProjectLocation.CurrentOption = std::filesystem::current_path().parent_path() / "Projects";

		s_ExportProjectServer.Label = "Export Server";
		s_ExportProjectServer.ToggleBoolean = true;
	}

	EditorApp* EditorApp::s_EditorApp = nullptr;

	EditorApp::EditorApp(const std::filesystem::path& projectPath)
		: Application("EditorLayer"), m_InitProjectPath(projectPath)
	{
		KG_ASSERT(!m_InitProjectPath.empty(), "Attempt to open editor without valid project path!");
		s_EditorApp = this;

		InitializeStaticResources();
		KG_INFO(Scripting::ScriptCompiler::CompileScriptFile("./../Projects/Pong/Assets/NewScripting/UpdateSessionUserSlot.kgscript"));
	}

	void EditorApp::Init()
	{
		Scripting::ScriptService::Init();
		Audio::AudioService::Init();
		Scenes::SceneService::Init();

		m_SceneHierarchyPanel = CreateScope<Panels::SceneHierarchyPanel>();

		m_EditorScene = CreateRef<Scenes::Scene>();
		Scenes::SceneService::SetActiveScene(m_EditorScene, m_EditorSceneHandle);
		m_SceneState = SceneState::Edit;

		OpenProject(m_InitProjectPath);

		EditorUI::EditorUIService::Init();

		m_LogPanel = CreateScope<Panels::LogPanel>();
		m_StatisticsPanel = CreateScope<Panels::StatisticsPanel>();
		m_ProjectPanel = CreateScope<Panels::ProjectPanel>();
		m_UIEditorPanel = CreateScope<Panels::UIEditorPanel>();
		m_ViewportPanel = CreateScope<Panels::ViewportPanel>();
		m_ScriptEditorPanel = CreateScope<Panels::ScriptEditorPanel>();
		m_EntityClassEditor = CreateScope<Panels::EntityClassEditor>();
		m_TextEditorPanel = CreateScope<Panels::TextEditorPanel>();
		m_GameStatePanel = CreateScope<Panels::GameStatePanel>();
		m_InputModePanel = CreateScope<Panels::InputModePanel>();
		m_ContentBrowserPanel = CreateScope<Panels::ContentBrowserPanel>();
		m_PropertiesPanel = CreateScope<Panels::PropertiesPanel>();

		m_ViewportPanel->InitializeFrameBuffer();

		Rendering::RenderingService::Init();
		Rendering::RenderingService::SetLineWidth(1.0f);
		RuntimeUI::FontService::Init();
		RuntimeUI::RuntimeUIService::Init();

		m_ViewportPanel->m_EditorCamera = Rendering::EditorCamera(30.0f, 1.778f, 0.1f, 1000.0f);
		
		m_ViewportPanel->InitializeOverlayData();

		EngineService::GetActiveWindow().SetVisible(true);
	}

	void EditorApp::Terminate()
	{
		EditorUI::EditorUIService::Terminate();
		Audio::AudioService::Terminate();
		Scripting::ScriptService::Terminate();
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
				ImGui::MenuItem("Content Browser", NULL, &m_ShowContentBrowser);
				ImGui::MenuItem("Scene Hierarchy", NULL, &m_ShowSceneHierarchy);
				ImGui::MenuItem("Viewport", NULL, &m_ShowViewport);
				ImGui::MenuItem("Properties", NULL, &m_ShowProperties);
				ImGui::Separator();
				ImGui::MenuItem("User Interface Editor", NULL, &m_ShowUserInterfaceEditor);
				ImGui::MenuItem("Input Mode Editor", NULL, &m_ShowInputModeEditor);
				ImGui::MenuItem("Script Editor", NULL, &m_ShowScriptEditor);
				ImGui::MenuItem("Text Editor", NULL, &m_ShowTextEditor);
				ImGui::MenuItem("Class Editor", NULL, &m_ShowClassEditor);
				ImGui::MenuItem("Game State Editor", NULL, &m_ShowGameStateEditor);
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
		if (m_ShowSceneHierarchy) { m_SceneHierarchyPanel->OnEditorUIRender(); }
		if (m_ShowContentBrowser) { m_ContentBrowserPanel->OnEditorUIRender(); }
		if (m_ShowLog) { m_LogPanel->OnEditorUIRender(); }
		if (m_ShowStats) { m_StatisticsPanel->OnEditorUIRender(); }
		if (m_ShowViewport) { m_ViewportPanel->OnEditorUIRender(); }
		if (m_ShowUserInterfaceEditor) { m_UIEditorPanel->OnEditorUIRender(); }
		if (m_ShowProject) { m_ProjectPanel->OnEditorUIRender(); }
		if (m_ShowScriptEditor) { m_ScriptEditorPanel->OnEditorUIRender(); }
		if (m_ShowClassEditor) { m_EntityClassEditor->OnEditorUIRender(); }
		if (m_ShowTextEditor) { m_TextEditorPanel->OnEditorUIRender(); }
		if (m_ShowGameStateEditor) { m_GameStatePanel->OnEditorUIRender(); }
		if (m_ShowInputModeEditor) { m_InputModePanel->OnEditorUIRender(); }
		if (m_ShowProperties) { m_PropertiesPanel->OnEditorUIRender(); }
		if (m_ShowDemoWindow) { ImGui::ShowDemoWindow(&m_ShowDemoWindow); }

		EditorUI::EditorUIService::GenericPopup(s_ExportProjectSpec);

		EditorUI::EditorUIService::EndWindow();

		EditorUI::EditorUIService::HighlightFocusedWindow();

		EditorUI::EditorUIService::EndRendering();
	}


	void EditorApp::OnEvent(Events::Event& event)
	{
		EditorUI::EditorUIService::OnEvent(event);
		if (event.Handled)
		{
			return;
		}
		std::string focusedWindow = EditorUI::EditorUIService::GetFocusedWindowName();
		if (focusedWindow == m_ViewportPanel->m_PanelName)
		{
			m_ViewportPanel->OnEvent(event);
		}

		Events::EventDispatcher dispatcher(event);
		if (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Simulate)
		{
			dispatcher.Dispatch<Events::KeyPressedEvent>(KG_BIND_CLASS_FN(EditorApp::OnKeyPressedEditor));
			//dispatcher.Dispatch<Events::MouseButtonPressedEvent>(KG_BIND_CLASS_FN(EditorApp::OnMouseButtonPressed));
			if (event.GetEventType() == Events::EventType::KeyReleased)
			{
				m_ViewportPanel->m_EditorCamera.OnKeyReleased(*(Events::KeyReleasedEvent*)& event);
			}
		}
		dispatcher.Dispatch<Events::PhysicsCollisionEvent>(KG_BIND_CLASS_FN(EditorApp::OnPhysicsCollision));
		dispatcher.Dispatch<Events::PhysicsCollisionEnd>(KG_BIND_CLASS_FN(EditorApp::OnPhysicsCollisionEnd));

		if (m_SceneState == SceneState::Play && m_IsPaused)
		{
			//dispatcher.Dispatch<Events::MouseButtonPressedEvent>(KG_BIND_CLASS_FN(EditorApp::OnMouseButtonPressed));
		}
		
		if (m_SceneState == SceneState::Play)
		{
			dispatcher.Dispatch<Events::KeyPressedEvent>(KG_BIND_CLASS_FN(EditorApp::OnKeyPressedRuntime));
			dispatcher.Dispatch<Events::ApplicationCloseEvent>(KG_BIND_CLASS_FN(EditorApp::OnApplicationClose));
			dispatcher.Dispatch<Events::UpdateOnlineUsers>(KG_BIND_CLASS_FN(EditorApp::OnUpdateUserCount));
			dispatcher.Dispatch<Events::ApproveJoinSession>(KG_BIND_CLASS_FN(EditorApp::OnApproveJoinSession));
			dispatcher.Dispatch<Events::UserLeftSession>(KG_BIND_CLASS_FN(EditorApp::OnUserLeftSession));
			dispatcher.Dispatch<Events::CurrentSessionInit>(KG_BIND_CLASS_FN(EditorApp::OnCurrentSessionInit));
			dispatcher.Dispatch<Events::ConnectionTerminated>(KG_BIND_CLASS_FN(EditorApp::OnConnectionTerminated));
			dispatcher.Dispatch<Events::UpdateSessionUserSlot>(KG_BIND_CLASS_FN(EditorApp::OnUpdateSessionUserSlot));
			dispatcher.Dispatch<Events::StartSession>(KG_BIND_CLASS_FN(EditorApp::OnStartSession));
			dispatcher.Dispatch<Events::SessionReadyCheckConfirm>(KG_BIND_CLASS_FN(EditorApp::OnSessionReadyCheckConfirm));
			dispatcher.Dispatch<Events::ReceiveSignal>(KG_BIND_CLASS_FN(EditorApp::OnReceiveSignal));
		}
	}

	bool EditorApp::OnApplicationClose(Events::ApplicationCloseEvent event)
	{
		OnStop();
		return false;
	}

	bool EditorApp::OnKeyPressedRuntime(Events::KeyPressedEvent event)
	{
		KG_PROFILE_FUNCTION()

		Scenes::SceneService::GetActiveScene()->OnKeyPressed(event);
		
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
				Scenes::Entity selectedEntity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
				if (selectedEntity)
				{
					m_EditorScene->DestroyEntity(selectedEntity);
					m_SceneHierarchyPanel->SetSelectedEntity({});
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
					m_SceneHierarchyPanel->SetSelectedEntity(*Scenes::SceneService::GetActiveScene()->GetHoveredEntity());
					// Algorithm to enable double clicking for an entity!
					static float previousTime{ 0.0f };
					static Scenes::Entity previousEntity{};
					float currentTime = Utility::Time::GetTime();
					if (std::fabs(currentTime - previousTime) < 0.2f && *Scenes::SceneService::GetActiveScene()->GetHoveredEntity() == previousEntity)
					{
						auto& transformComponent = Scenes::SceneService::GetActiveScene()->GetHoveredEntity()->GetComponent<Scenes::TransformComponent>();
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


	bool EditorApp::OnPhysicsCollision(Events::PhysicsCollisionEvent event)
	{
		Ref<Scenes::Scene> activeScene = Scenes::SceneService::GetActiveScene();
		UUID entityOneID = event.GetEntityOne();
		Scenes::Entity entityOne = activeScene->GetEntityByUUID(entityOneID);
		UUID entityTwoID = event.GetEntityTwo();
		Scenes::Entity entityTwo = activeScene->GetEntityByUUID(entityTwoID);

		KG_ASSERT(entityOne);
		KG_ASSERT(entityTwo);

		bool collisionHandled = false;
		if (entityOne.HasComponent<Scenes::ClassInstanceComponent>())
		{
			Scenes::ClassInstanceComponent& component = entityOne.GetComponent<Scenes::ClassInstanceComponent>();
			Assets::AssetHandle scriptHandle = component.ClassReference->GetScripts().OnPhysicsCollisionStartHandle;
			Scripting::Script* script = component.ClassReference->GetScripts().OnPhysicsCollisionStart;
			if (scriptHandle != Assets::EmptyHandle)
			{
				collisionHandled = ((WrappedBoolUInt64UInt64*)script->m_Function.get())->m_Value(entityOneID, entityTwoID);
			}
		}

		if (!collisionHandled && entityTwo.HasComponent<Scenes::ClassInstanceComponent>())
		{
			Scenes::ClassInstanceComponent& component = entityTwo.GetComponent<Scenes::ClassInstanceComponent>();
			Assets::AssetHandle scriptHandle = component.ClassReference->GetScripts().OnPhysicsCollisionStartHandle;
			Scripting::Script* script = component.ClassReference->GetScripts().OnPhysicsCollisionStart;
			if (scriptHandle != Assets::EmptyHandle)
			{
				collisionHandled = ((WrappedBoolUInt64UInt64*)script->m_Function.get())->m_Value(entityTwoID, entityOneID);
			}
		}
		return false;
	}

	bool EditorApp::OnPhysicsCollisionEnd(Events::PhysicsCollisionEnd event)
	{
		Ref<Scenes::Scene> activeScene = Scenes::SceneService::GetActiveScene();
		UUID entityOneID = event.GetEntityOne();
		Scenes::Entity entityOne = activeScene->GetEntityByUUID(entityOneID);
		UUID entityTwoID = event.GetEntityTwo();
		Scenes::Entity entityTwo = activeScene->GetEntityByUUID(entityTwoID);

		KG_ASSERT(entityOne);
		KG_ASSERT(entityTwo);

		bool collisionHandled = false;
		if (entityOne.HasComponent<Scenes::ClassInstanceComponent>())
		{
			Scenes::ClassInstanceComponent& component = entityOne.GetComponent<Scenes::ClassInstanceComponent>();
			Assets::AssetHandle scriptHandle = component.ClassReference->GetScripts().OnPhysicsCollisionEndHandle;
			Scripting::Script* script = component.ClassReference->GetScripts().OnPhysicsCollisionEnd;
			if (scriptHandle != Assets::EmptyHandle)
			{
				collisionHandled = ((WrappedBoolUInt64UInt64*)script->m_Function.get())->m_Value(entityOneID, entityTwoID);
			}
		}

		if (!collisionHandled && entityOne.HasComponent<Scenes::ClassInstanceComponent>())
		{
			Scenes::ClassInstanceComponent& component = entityTwo.GetComponent<Scenes::ClassInstanceComponent>();
			Assets::AssetHandle scriptHandle = component.ClassReference->GetScripts().OnPhysicsCollisionEndHandle;
			Scripting::Script* script = component.ClassReference->GetScripts().OnPhysicsCollisionEnd;
			if (scriptHandle != Assets::EmptyHandle)
			{
				collisionHandled = ((WrappedBoolUInt64UInt64*)script->m_Function.get())->m_Value(entityTwoID, entityOneID);
			}
		}
		return false;
	}

	bool EditorApp::OnUpdateUserCount(Events::UpdateOnlineUsers event)
	{
		Assets::AssetHandle scriptHandle = Projects::ProjectService::GetActiveOnUpdateUserCount();
		if (scriptHandle != Assets::EmptyHandle)
		{
			((WrappedVoidUInt32*)Assets::AssetManager::GetScript(scriptHandle)->m_Function.get())->m_Value(event.GetUserCount());
		}

		return false;
	}

	bool EditorApp::OnApproveJoinSession(Events::ApproveJoinSession event)
	{
		Assets::AssetHandle scriptHandle = Projects::ProjectService::GetActiveOnApproveJoinSession();
		if (scriptHandle != Assets::EmptyHandle)
		{
			((WrappedVoidUInt16*)Assets::AssetManager::GetScript(scriptHandle)->m_Function.get())->m_Value(event.GetUserSlot());
		}

		return false;
	}

	bool EditorApp::OnUpdateSessionUserSlot(Events::UpdateSessionUserSlot event)
	{
		Assets::AssetHandle scriptHandle = Projects::ProjectService::GetActiveOnUpdateSessionUserSlot();
		if (scriptHandle != Assets::EmptyHandle)
		{
			((WrappedVoidUInt16*)Assets::AssetManager::GetScript(scriptHandle)->m_Function.get())->m_Value(event.GetUserSlot());
		}

		return false;
	}

	bool EditorApp::OnUserLeftSession(Events::UserLeftSession event)
	{
		Assets::AssetHandle scriptHandle = Projects::ProjectService::GetActiveOnUserLeftSession();
		if (scriptHandle != Assets::EmptyHandle)
		{
			((WrappedVoidUInt16*)Assets::AssetManager::GetScript(scriptHandle)->m_Function.get())->m_Value(event.GetUserSlot());
		}
		return false;
	}

	bool EditorApp::OnCurrentSessionInit(Events::CurrentSessionInit event)
	{
		Assets::AssetHandle scriptHandle = Projects::ProjectService::GetActiveOnCurrentSessionInit();
		if (scriptHandle != Assets::EmptyHandle)
		{
			((WrappedVoidNone*)Assets::AssetManager::GetScript(scriptHandle)->m_Function.get())->m_Value();
		}
		return false;
	}

	bool EditorApp::OnConnectionTerminated(Events::ConnectionTerminated event)
	{
		Assets::AssetHandle scriptHandle = Projects::ProjectService::GetActiveOnConnectionTerminated();
		if (scriptHandle != Assets::EmptyHandle)
		{
			((WrappedVoidNone*)Assets::AssetManager::GetScript(scriptHandle)->m_Function.get())->m_Value();
		}
		return false;
	}

	bool EditorApp::OnStartSession(Events::StartSession event)
	{
		Assets::AssetHandle scriptHandle = Projects::ProjectService::GetActiveOnStartSession();
		if (scriptHandle != Assets::EmptyHandle)
		{
			((WrappedVoidNone*)Assets::AssetManager::GetScript(scriptHandle)->m_Function.get())->m_Value();
		}
		return false;
	}

	bool EditorApp::OnSessionReadyCheckConfirm(Events::SessionReadyCheckConfirm event)
	{
		Assets::AssetHandle scriptHandle = Projects::ProjectService::GetActiveOnSessionReadyCheckConfirm();
		if (scriptHandle != Assets::EmptyHandle)
		{
			((WrappedVoidNone*)Assets::AssetManager::GetScript(scriptHandle)->m_Function.get())->m_Value();
		}
		return false;
	}

	bool EditorApp::OnReceiveSignal(Events::ReceiveSignal event)
	{
		Assets::AssetHandle scriptHandle = Projects::ProjectService::GetActiveOnReceiveSignal();
		if (scriptHandle != Assets::EmptyHandle)
		{
			((WrappedVoidUInt16*)Assets::AssetManager::GetScript(scriptHandle)->m_Function.get())->m_Value(event.GetSignal());
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
		m_EntityClassEditor->ResetPanelResources();
		m_ScriptEditorPanel->ResetPanelResources();
		m_ProjectPanel->ResetPanelResources();
		Scenes::GameStateService::ClearActiveGameState();
		Input::InputModeService::ClearActiveInputMode();

		return true;
	}

	void EditorApp::OpenProject(const std::filesystem::path& path)
	{
		if (Assets::AssetManager::OpenProject(path))
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
			Assets::AssetManager::ClearAll();
			Assets::AssetManager::DeserializeAll();
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
		Assets::AssetManager::SaveActiveProject((Projects::ProjectService::GetActiveProjectDirectory() / Projects::ProjectService::GetActiveProjectName()).replace_extension(".kproj"));
	}

	void EditorApp::NewScene()
	{
		std::filesystem::path initialDirectory = Projects::ProjectService::GetActiveAssetDirectory() / "Scenes";
		std::filesystem::path filepath = Utility::FileDialogs::SaveFile("Kargono Scene (*.kgscene)\0*.kgscene\0", initialDirectory.string().c_str());
		if (filepath.empty()) { return; }
		if (Assets::AssetManager::CheckSceneExists(filepath.stem().string()))
		{
			KG_WARN("Attempt to create scene with duplicate name!");
			return;
		}
		m_EditorSceneHandle = Assets::AssetManager::CreateNewScene(filepath.stem().string());

		*Scenes::SceneService::GetActiveScene()->GetHoveredEntity() = {};
		m_EditorScene = Assets::AssetManager::GetScene(m_EditorSceneHandle);
		Scenes::SceneService::SetActiveScene(m_EditorScene, m_EditorSceneHandle);
	}

	void EditorApp::NewScene(const std::string& sceneName)
	{
		std::filesystem::path filepath = Projects::ProjectService::GetActiveAssetDirectory() / ("Scenes/" + sceneName + ".kgscene");
		if (Assets::AssetManager::CheckSceneExists(filepath.stem().string()))
		{
			KG_WARN("Attempt to create scene with duplicate name!");
			return;
		}
		m_EditorSceneHandle = Assets::AssetManager::CreateNewScene(filepath.stem().string());

		*Scenes::SceneService::GetActiveScene()->GetHoveredEntity() = {};
		m_EditorScene = Assets::AssetManager::GetScene(m_EditorSceneHandle);
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
		auto [sceneHandle, newScene] = Assets::AssetManager::GetScene(path);

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

		Ref<Scenes::Scene> newScene = Assets::AssetManager::GetScene(sceneHandle);
		if (!newScene) { newScene = CreateRef<Scenes::Scene>(); }

		m_EditorScene = newScene;
		Scenes::SceneService::SetActiveScene(m_EditorScene, m_EditorSceneHandle);
		m_EditorSceneHandle = sceneHandle;
	}

	void EditorApp::SaveScene()
	{
		SerializeScene(m_EditorScene);
	}

	void EditorApp::SerializeScene(Ref<Scenes::Scene> scene)
	{
		Assets::AssetManager::SaveScene(m_EditorSceneHandle, scene);
		
	}

	void EditorApp::OnPlay()
	{
		// Cache Current UserInterface/InputMode in editor
		if (!RuntimeUI::RuntimeUIService::GetActiveUI()) { m_EditorUIObject = nullptr; }
		else
		{
			RuntimeUI::RuntimeUIService::SaveCurrentUIIntoUIObject();
			m_EditorUIObject = RuntimeUI::RuntimeUIService::GetActiveUI();
			m_EditorUIObjectHandle = RuntimeUI::RuntimeUIService::GetActiveUIHandle();
		}

		if (!Input::InputModeService::GetActiveInputMode()) { m_EditorInputMode = nullptr; }
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
				Assets::AssetManager::GetGameState(Projects::ProjectService::GetActiveStartGameState()),
				Projects::ProjectService::GetActiveStartGameState());
		}

		*Scenes::SceneService::GetActiveScene()->GetHoveredEntity() = {};
		if (m_SceneState == SceneState::Simulate) { OnStop(); }

		m_SceneState = SceneState::Play;
		Scenes::SceneService::SetActiveScene(Scenes::SceneService::CreateSceneCopy(m_EditorScene), m_EditorSceneHandle);
		Scenes::SceneService::GetActiveScene()->OnRuntimeStart();
		Assets::AssetHandle scriptHandle = Projects::ProjectService::GetActiveOnRuntimeStart();
		if (scriptHandle != 0)
		{
			((WrappedVoidNone*)Assets::AssetManager::GetScript(scriptHandle)->m_Function.get())->m_Value();
		}

		if (Projects::ProjectService::GetActiveAppIsNetworked())
		{
			Network::ClientService::SetActiveClient(CreateRef<Network::Client>());
			Network::ClientService::SetActiveNetworkThread(CreateRef<std::thread>(&Network::Client::RunClient, Network::ClientService::GetActiveClient().get()));
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
		Scenes::SceneService::GetActiveScene()->OnSimulationStart();
	}
	void EditorApp::OnStop()
	{
		*Scenes::SceneService::GetActiveScene()->GetHoveredEntity() = {};
		KG_ASSERT(m_SceneState == SceneState::Play || m_SceneState == SceneState::Simulate, "Unknown Scene State Given to OnSceneStop")

		if (m_SceneState == SceneState::Play) { Scenes::SceneService::GetActiveScene()->OnRuntimeStop(); }
		else if (m_SceneState == SceneState::Simulate) { Scenes::SceneService::GetActiveScene()->OnSimulationStop(); }

		Scenes::SceneService::GetActiveScene()->DestroyAllEntities();
		Scenes::SceneService::SetActiveScene(m_EditorScene, m_EditorSceneHandle);
		Audio::AudioService::StopAllAudio();

		// Clear UIObjects during runtime.
		if (m_EditorUIObject)
		{
			RuntimeUI::RuntimeUIService::SetActiveUI(m_EditorUIObject, m_EditorUIObjectHandle);
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

			Network::ClientService::GetActiveClient()->StopClient();
			Network::ClientService::GetActiveNetworkThread()->join();
			Network::ClientService::GetActiveNetworkThread().reset();
			Network::ClientService::GetActiveClient().reset();
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

		Scenes::Entity selectedEntity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
		if (selectedEntity)
		{
			Scenes::Entity newEntity = m_EditorScene->DuplicateEntity(selectedEntity);
			m_SceneHierarchyPanel->SetSelectedEntity(newEntity);
		}
	}

}
