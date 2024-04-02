#include "kgpch.h"

#include "EditorLayer.h"

namespace Kargono
{

	EditorLayer* EditorLayer::s_EditorLayer = nullptr;

	EditorLayer::EditorLayer()
		: Layer("EditorLayer")
	{
		s_EditorLayer = this;
	}

	void EditorLayer::OnAttach()
	{
		Script::ScriptEngine::Init();
		Scripting::ScriptCore::Init();
		Audio::AudioEngine::Init();

		m_SceneHierarchyPanel = CreateScope<SceneHierarchyPanel>();

		m_EditorScene = CreateRef<Scene>();
		Scene::SetActiveScene(m_EditorScene);
		m_SceneState = SceneState::Edit;

		if (!OpenProject())
		{
			Application::GetCurrentApp().Close();
			return;
		}
		UI::Editor::Init();

		m_LogPanel = CreateScope<LogPanel>();
		m_StatisticsPanel = CreateScope<StatisticsPanel>();
		m_ProjectPanel = CreateScope<ProjectPanel>();
		m_SettingsPanel = CreateScope<SettingsPanel>();
		m_ToolbarPanel = CreateScope<ToolbarPanel>();
		m_UIEditorPanel = CreateScope<UIEditorPanel>();
		m_ViewportPanel = CreateScope<ViewportPanel>();
		m_ScriptEditorPanel = CreateScope<ScriptEditorPanel>();
		m_EntityClassEditor = CreateScope<EntityClassEditor>();
		m_TextEditorPanel = CreateScope<TextEditorPanel>();
		m_GameStatePanel = CreateScope<GameStatePanel>();

		m_ViewportPanel->InitializeFrameBuffer();

		Renderer::Init();
		Renderer::SetLineWidth(1.0f);
		UI::Text::Init();
		UI::Runtime::Init();

		m_ViewportPanel->m_EditorCamera = EditorCamera(30.0f, 1.778f, 0.1f, 1000.0f);
		
		m_ViewportPanel->InitializeOverlayData();

		Application::GetCurrentApp().GetWindow().SetVisible(true);
	}

	void EditorLayer::OnDetach()
	{
		auto allAudioComponents = m_EditorScene->GetAllEntitiesWith<AudioComponent>();
		for (auto& entity : allAudioComponents)
		{
			Entity e = { entity, m_EditorScene.get()};
			auto& audioComponent = e.GetComponent<AudioComponent>();
			audioComponent.Audio.reset();
		}
		auto allMultiAudioComponents = m_EditorScene->GetAllEntitiesWith<MultiAudioComponent>();
		for (auto& entity : allMultiAudioComponents)
		{
			Entity e = { entity, m_EditorScene.get() };
			auto& multiAudioComponent = e.GetComponent<MultiAudioComponent>();
			for (auto& [key, component] : multiAudioComponent.AudioComponents)
			{
				component.Audio.reset();
			}
		}

		UI::Editor::Terminate();
	}

	void EditorLayer::OnUpdate(Timestep ts)
	{
		KG_PROFILE_FUNCTION();

		m_ViewportPanel->OnUpdate(ts);

		OnEditorUIRender();
	}

	void EditorLayer::Step(int frames)
	{
		m_StepFrames = frames;
	}


	void EditorLayer::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION();
		UI::Editor::StartRendering();

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

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
		if (!opt_padding)
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace", &dockspaceOpen, window_flags);
		if (!opt_padding)
			ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

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
			if (m_ShowToolbar) { m_ToolbarPanel->OnEditorUIRender(); }
			UI::Editor::EndWindow();
			UI::Editor::EndRendering();
			return;
		}

		// Set up Menu Toolbar
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open Project ...", "Ctrl+O"))
				{
					OpenProject();
				}

				ImGui::Separator();

				if (ImGui::MenuItem("New Scene", "Ctrl+N"))
				{
					NewScene();
				}

				if (ImGui::MenuItem("Save", "Ctrl+S"))
				{
					SaveScene();
					SaveProject();
				}

				if (ImGui::MenuItem("Exit"))
				{
					Application::GetCurrentApp().Close();
				}
				ImGui::EndMenu();

			}

			if (ImGui::BeginMenu("Script"))
			{
				if (ImGui::MenuItem("Reload Assembly", "Ctrl+R"))
				{
					if (m_SceneState != SceneState::Edit) { OnStop(); }
					Script::ScriptEngine::ReloadAssembly();
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Panels"))
			{
				ImGui::MenuItem("Content Browser", NULL, &m_ShowContentBrowser);
				ImGui::MenuItem("Scene Hierarchy", NULL, &m_ShowSceneHierarchy);
				ImGui::MenuItem("Viewport", NULL, &m_ShowViewport);
				ImGui::MenuItem("Toolbar", NULL, &m_ShowToolbar);
				ImGui::Separator();
				ImGui::MenuItem("User Interface Editor", NULL, &m_ShowUserInterfaceEditor);
				ImGui::MenuItem("Input Mode Editor", NULL, &m_ShowInputEditor);
				ImGui::MenuItem("Script Editor", NULL, &m_ShowScriptEditor);
				ImGui::MenuItem("Text Editor", NULL, &m_ShowTextEditor);
				ImGui::MenuItem("Class Editor", NULL, &m_ShowClassEditor);
				ImGui::MenuItem("Game State Editor", NULL, &m_GameStateEditor);
				ImGui::Separator();
				ImGui::MenuItem("Settings", NULL, &m_ShowSettings);
				ImGui::MenuItem("Project", NULL, &m_ShowProject);
				ImGui::Separator();
				ImGui::MenuItem("Stats", NULL, &m_ShowStats);
				ImGui::MenuItem("Log", NULL, &m_ShowLog);
				ImGui::MenuItem("ImGui Demo", NULL, &m_ShowDemoWindow);
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		// Display other panels
		if (m_ShowSceneHierarchy) { m_SceneHierarchyPanel->OnEditorUIRender(); }
		if (m_ShowContentBrowser) { m_ContentBrowserPanel->OnEditorUIRender(); }
		if (m_ShowLog) { m_LogPanel->OnEditorUIRender(); }
		if (m_ShowStats) { m_StatisticsPanel->OnEditorUIRender(); }
		if (m_ShowSettings) { m_SettingsPanel->OnEditorUIRender(); }
		if (m_ShowViewport) { m_ViewportPanel->OnEditorUIRender(); }
		if (m_ShowUserInterfaceEditor) { m_UIEditorPanel->OnEditorUIRender(); }
		if (m_ShowInputEditor) { m_InputEditorPanel->OnEditorUIRender(); }
		if (m_ShowToolbar) { m_ToolbarPanel->OnEditorUIRender(); }
		if (m_ShowProject) { m_ProjectPanel->OnEditorUIRender(); }
		if (m_ShowScriptEditor) { m_ScriptEditorPanel->OnEditorUIRender(); }
		if (m_ShowClassEditor) { m_EntityClassEditor->OnEditorUIRender(); }
		if (m_ShowTextEditor) { m_TextEditorPanel->OnEditorUIRender(); }
		if (m_GameStateEditor) { m_GameStatePanel->OnEditorUIRender(); }
		if (m_ShowDemoWindow) { ImGui::ShowDemoWindow(); }

		UI::Editor::EndWindow();

		UI::Editor::EndRendering();
	}


	void EditorLayer::OnEvent(Events::Event& event)
	{
		UI::Editor::OnEvent(event);
		if (event.Handled)
		{
			return;
		}

		Events::EventDispatcher dispatcher(event);
		if (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Simulate)
		{
			m_ViewportPanel->m_EditorCamera.OnEvent(event);
			dispatcher.Dispatch<Events::KeyPressedEvent>(KG_BIND_EVENT_FN(EditorLayer::OnKeyPressedEditor));
			dispatcher.Dispatch<Events::MouseButtonPressedEvent>(KG_BIND_EVENT_FN(EditorLayer::OnMouseButtonPressed));
		}
		dispatcher.Dispatch<Events::PhysicsCollisionEvent>(KG_BIND_EVENT_FN(EditorLayer::OnPhysicsCollision));
		dispatcher.Dispatch<Events::PhysicsCollisionEnd>(KG_BIND_EVENT_FN(EditorLayer::OnPhysicsCollisionEnd));

		if (m_SceneState == SceneState::Play && m_IsPaused) { dispatcher.Dispatch<Events::MouseButtonPressedEvent>(KG_BIND_EVENT_FN(EditorLayer::OnMouseButtonPressed)); }

		if (m_SceneState == SceneState::Play)
		{
			dispatcher.Dispatch<Events::KeyPressedEvent>(KG_BIND_EVENT_FN(EditorLayer::OnKeyPressedRuntime));
			dispatcher.Dispatch<Events::ApplicationCloseEvent>(KG_BIND_EVENT_FN(EditorLayer::OnApplicationClose));
			dispatcher.Dispatch<Events::UpdateOnlineUsers>(KG_BIND_EVENT_FN(EditorLayer::OnUpdateUserCount));
			dispatcher.Dispatch<Events::ApproveJoinSession>(KG_BIND_EVENT_FN(EditorLayer::OnApproveJoinSession));
			dispatcher.Dispatch<Events::UserLeftSession>(KG_BIND_EVENT_FN(EditorLayer::OnUserLeftSession));
			dispatcher.Dispatch<Events::CurrentSessionInit>(KG_BIND_EVENT_FN(EditorLayer::OnCurrentSessionInit));
			dispatcher.Dispatch<Events::ConnectionTerminated>(KG_BIND_EVENT_FN(EditorLayer::OnConnectionTerminated));
			dispatcher.Dispatch<Events::UpdateSessionUserSlot>(KG_BIND_EVENT_FN(EditorLayer::OnUpdateSessionUserSlot));
			dispatcher.Dispatch<Events::StartSession>(KG_BIND_EVENT_FN(EditorLayer::OnStartSession));
			dispatcher.Dispatch<Events::SessionReadyCheckConfirm>(KG_BIND_EVENT_FN(EditorLayer::OnSessionReadyCheckConfirm));
			dispatcher.Dispatch<Events::ReceiveSignal>(KG_BIND_EVENT_FN(EditorLayer::OnReceiveSignal));
		}
	}

	bool EditorLayer::OnApplicationClose(Events::ApplicationCloseEvent event)
	{
		OnStop();
		return false;
	}

	bool EditorLayer::OnKeyPressedRuntime(Events::KeyPressedEvent event)
	{
		KG_PROFILE_FUNCTION()

		Script::ScriptEngine::OnKeyPressed(event);
		return false;
	}

	bool EditorLayer::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		if (event.IsRepeat()) { return false; }

		bool control = InputPolling::IsKeyPressed(Key::LeftControl) || InputPolling::IsKeyPressed(Key::RightControl);
		bool shift = InputPolling::IsKeyPressed(Key::LeftShift) || InputPolling::IsKeyPressed(Key::RightShift);
		bool alt = InputPolling::IsKeyPressed(Key::LeftAlt) || InputPolling::IsKeyPressed(Key::RightAlt);

		switch (event.GetKeyCode())
		{
		case Key::Escape:
		{
			m_SceneHierarchyPanel->SetSelectedEntity({});
			break;
		}
		case Key::Tab:
		{
			m_ViewportPanel->m_EditorCamera.ToggleMovementType();
			break;
		}

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

		// Scene Commands

		case Key::D:
		{
			if (control) { OnDuplicateEntity(); }
			break;
		}

		// Gizmos
		case Key::Q:
			{
			if (!ImGuizmo::IsUsing() && !alt) { m_ViewportPanel->m_GizmoType = -1; }
				break;
			}
		case Key::W:
		{
			if (!ImGuizmo::IsUsing() && !alt) { m_ViewportPanel->m_GizmoType = ImGuizmo::OPERATION::TRANSLATE; }
			break;
		}
		case Key::E:
		{
			if (!ImGuizmo::IsUsing() && !alt) { m_ViewportPanel->m_GizmoType = ImGuizmo::OPERATION::ROTATE; }
			break;
		}
		case Key::R:
		{
			if (control)
			{
				if (m_SceneState != SceneState::Edit) { OnStop(); }
				Script::ScriptEngine::ReloadAssembly();
			}

			if (!ImGuizmo::IsUsing()) { m_ViewportPanel->m_GizmoType = ImGuizmo::OPERATION::SCALE; }
			break;
		}
		case Key::Delete:
			{
			if (UI::Editor::GetActiveWidgetID() == 0)
			{
				Entity selectedEntity = *Scene::GetActiveScene()->GetSelectedEntity();
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

	bool EditorLayer::OnMouseButtonPressed(Events::MouseButtonPressedEvent event)
	{
		if (event.GetMouseButton() == Mouse::ButtonLeft)
		{
			if (m_ViewportPanel->m_ViewportHovered && !ImGuizmo::IsOver() && !InputPolling::IsKeyPressed(Key::LeftAlt) && *Scene::GetActiveScene()->GetHoveredEntity())
			{
				m_SceneHierarchyPanel->SetSelectedEntity(*Scene::GetActiveScene()->GetHoveredEntity());
				// Algorithm to enable double clicking for an entity!
				static float previousTime{ 0.0f };
				static Entity previousEntity {};
				float currentTime = Utility::Time::GetTime();
				if (std::fabs(currentTime - previousTime) < 0.2f && *Scene::GetActiveScene()->GetHoveredEntity() == previousEntity)
				{
					auto& transformComponent = Scene::GetActiveScene()->GetHoveredEntity()->GetComponent<TransformComponent>();
					m_ViewportPanel->m_EditorCamera.SetFocalPoint(transformComponent.Translation);
					m_ViewportPanel->m_EditorCamera.SetDistance(std::max({ transformComponent.Scale.x, transformComponent.Scale.y, transformComponent.Scale.z }) * 2.5f);
					m_ViewportPanel->m_EditorCamera.SetMovementType(EditorCamera::MovementType::ModelView);
				}
				previousTime = currentTime;
				previousEntity = *Scene::GetActiveScene()->GetHoveredEntity();

			}
		}
		return false;
	}


	bool EditorLayer::OnPhysicsCollision(Events::PhysicsCollisionEvent event)
	{
		Script::ScriptEngine::OnPhysicsCollision(event);
		return false;
	}

	bool EditorLayer::OnPhysicsCollisionEnd(Events::PhysicsCollisionEnd event)
	{
		Script::ScriptEngine::OnPhysicsCollisionEnd(event);
		return false;
	}

	bool EditorLayer::OnUpdateUserCount(Events::UpdateOnlineUsers event)
	{
		Assets::AssetHandle scriptHandle = Projects::Project::GetOnUpdateUserCount();
		if (scriptHandle != 0)
		{
			((WrappedVoidUInt32*)Assets::AssetManager::GetScript(scriptHandle)->m_Function.get())->m_Value(event.GetUserCount());
		}

		return false;
	}

	bool EditorLayer::OnApproveJoinSession(Events::ApproveJoinSession event)
	{
		uint16_t userSlot = event.GetUserSlot();
		void* param = &userSlot;
		Script::ScriptEngine::RunCustomCallsFunction(Projects::Project::GetProjectOnApproveJoinSession(), &param);
		return false;
	}

	bool EditorLayer::OnUpdateSessionUserSlot(Events::UpdateSessionUserSlot event)
	{
		Assets::AssetHandle scriptHandle = Projects::Project::GetOnUpdateSessionUserSlot();
		if (scriptHandle != 0)
		{
			((WrappedVoidUInt16*)Assets::AssetManager::GetScript(scriptHandle)->m_Function.get())->m_Value(event.GetUserSlot());
		}

		return false;
	}

	bool EditorLayer::OnUserLeftSession(Events::UserLeftSession event)
	{
		Assets::AssetHandle scriptHandle = Projects::Project::GetOnUserLeftSession();
		if (scriptHandle != 0)
		{
			((WrappedVoidUInt16*)Assets::AssetManager::GetScript(scriptHandle)->m_Function.get())->m_Value(event.GetUserSlot());
		}
		return false;
	}

	bool EditorLayer::OnCurrentSessionInit(Events::CurrentSessionInit event)
	{
		Assets::AssetHandle scriptHandle = Projects::Project::GetOnCurrentSessionInit();
		if (scriptHandle != 0)
		{
			((WrappedVoidNone*)Assets::AssetManager::GetScript(scriptHandle)->m_Function.get())->m_Value();
		}
		return false;
	}

	bool EditorLayer::OnConnectionTerminated(Events::ConnectionTerminated event)
	{
		Assets::AssetHandle scriptHandle = Projects::Project::GetOnConnectionTerminated();
		if (scriptHandle != 0)
		{
			((WrappedVoidNone*)Assets::AssetManager::GetScript(scriptHandle)->m_Function.get())->m_Value();
		}
		return false;
	}

	bool EditorLayer::OnStartSession(Events::StartSession event)
	{
		Assets::AssetHandle scriptHandle = Projects::Project::GetOnStartSession();
		if (scriptHandle != 0)
		{
			((WrappedVoidNone*)Assets::AssetManager::GetScript(scriptHandle)->m_Function.get())->m_Value();
		}
		return false;
	}

	bool EditorLayer::OnSessionReadyCheckConfirm(Events::SessionReadyCheckConfirm event)
	{
		Script::ScriptEngine::RunCustomCallsFunction(Projects::Project::GetProjectOnSessionReadyCheckConfirm());
		return false;
	}

	bool EditorLayer::OnReceiveSignal(Events::ReceiveSignal event)
	{
		uint16_t signal = event.GetSignal();
		void* param = &signal;
		Script::ScriptEngine::RunCustomCallsFunction(Projects::Project::GetProjectOnReceiveSignal(), &param);
		return false;
	}

	void EditorLayer::NewProject()
	{
		Assets::AssetManager::NewProject();
	}

	bool EditorLayer::OpenProject()
	{
		*Scene::GetActiveScene()->GetHoveredEntity() = {};
		std::filesystem::path initialDirectory = std::filesystem::current_path().parent_path() / "Projects";
		if (!std::filesystem::exists(initialDirectory))
		{
			initialDirectory = "";
		}
		std::filesystem::path filepath = Utility::FileDialogs::OpenFile("Kargono Project (*.kproj)\0*.kproj\0", initialDirectory.string().c_str());
		if (filepath.empty()) { return false; }

		OpenProject(filepath);
		return true;
	}

	void EditorLayer::OpenProject(const std::filesystem::path& path)
	{
		if (Assets::AssetManager::OpenProject(path))
		{
			if (!Application::GetCurrentApp().GetWindow().GetNativeWindow())
			{
				
				Application::GetCurrentApp().GetWindow().Init();
				RenderCommand::Init();
			}
			auto startSceneHandle = Projects::Project::GetStartSceneHandle();

			Scripting::ScriptCore::OpenDll();

			if (Script::ScriptEngine::AppDomainExists()){ Script::ScriptEngine::ReloadAssembly(); }
			else { Script::ScriptEngine::InitialAssemblyLoad(); }
			if (m_EditorScene)
			{
				auto view = m_EditorScene->GetAllEntitiesWith<AudioComponent>();
				for (auto& entity : view)
				{
					Entity e = { entity, m_EditorScene.get() };
					auto& audioComponent = e.GetComponent<AudioComponent>();
					audioComponent.Audio.reset();
				}
				m_EditorScene->DestroyAllEntities();
			}
			Assets::AssetManager::ClearAll();
			Assets::AssetManager::DeserializeAll();

			OpenScene(startSceneHandle);

			m_ContentBrowserPanel = CreateScope<ContentBrowserPanel>();
		}
	}

	void EditorLayer::SaveProject()
	{
		Assets::AssetManager::SaveActiveProject((Projects::Project::GetProjectDirectory() / Projects::Project::GetProjectName()).replace_extension(".kproj"));
	}

	void EditorLayer::NewScene()
	{
		std::filesystem::path initialDirectory = Projects::Project::GetAssetDirectory() / "Scenes";
		std::filesystem::path filepath = Utility::FileDialogs::SaveFile("Kargono Scene (*.kgscene)\0*.kgscene\0", initialDirectory.string().c_str());
		if (filepath.empty()) { return; }
		if (Assets::AssetManager::CheckSceneExists(filepath.stem().string()))
		{
			KG_WARN("Attempt to create scene with duplicate name!");
			return;
		}
		m_EditorSceneHandle = Assets::AssetManager::CreateNewScene(filepath.stem().string());

		*Scene::GetActiveScene()->GetHoveredEntity() = {};
		m_EditorScene = Assets::AssetManager::GetScene(m_EditorSceneHandle);
		Scene::SetActiveScene(m_EditorScene);
	}

	void EditorLayer::OpenScene()
	{
		std::filesystem::path initialDirectory = Projects::Project::GetAssetDirectory();
		std::filesystem::path filepath = Utility::FileDialogs::OpenFile("Kargono Scene (*.kgscene)\0*.kgscene\0", initialDirectory.string().c_str());
		
		if (!filepath.empty())
		{
			OpenScene(filepath);
		}
	}

	void EditorLayer::OpenScene(const std::filesystem::path& path)
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
		Scene::SetActiveScene(m_EditorScene);
		m_EditorSceneHandle = sceneHandle;

	}

	void EditorLayer::OpenScene(Assets::AssetHandle sceneHandle)
	{
		if (m_SceneState != SceneState::Edit)
		{
			OnStop();
		}

		Ref<Scene> newScene = Assets::AssetManager::GetScene(sceneHandle);
		if (!newScene) { newScene = CreateRef<Scene>(); }

		m_EditorScene = newScene;
		Scene::SetActiveScene(m_EditorScene);
		m_EditorSceneHandle = sceneHandle;
	}

	void EditorLayer::SaveScene()
	{
		SerializeScene(m_EditorScene);
	}

	void EditorLayer::SerializeScene(Ref<Scene> scene)
	{
		Assets::AssetManager::SaveScene(m_EditorSceneHandle, scene);
		
	}

	void EditorLayer::OnPlay()
	{
		// Cache Current UIObject/InputMode in editor
		if (!UI::Runtime::GetCurrentUIObject()) { m_EditorUIObject = nullptr; }
		else
		{
			UI::Runtime::SaveCurrentUIIntoUIObject();
			m_EditorUIObject = UI::Runtime::GetCurrentUIObject();
			m_EditorUIObjectHandle = UI::Runtime::GetCurrentUIHandle();
		}

		if (!InputMode::s_InputMode) { m_EditorInputMode = nullptr; }
		else
		{
			m_EditorInputMode = InputMode::s_InputMode;
			m_EditorInputModeHandle = InputMode::s_InputModeHandle;
		}

		*Scene::GetActiveScene()->GetHoveredEntity() = {};
		if (m_SceneState == SceneState::Simulate) { OnStop(); }

		m_SceneState = SceneState::Play;
		Scene::SetActiveScene(Scene::Copy(m_EditorScene));
		Scene::GetActiveScene()->OnRuntimeStart();
		Assets::AssetHandle scriptHandle = Projects::Project::GetOnRuntimeStart();
		if (scriptHandle != 0)
		{
			((WrappedVoidNone*)Assets::AssetManager::GetScript(scriptHandle)->m_Function.get())->m_Value();
		}

		if (Projects::Project::GetAppIsNetworked())
		{
			Network::Client::SetActiveClient(CreateRef<Network::Client>());
			Network::Client::SetActiveNetworkThread(CreateRef<std::thread>(&Network::Client::RunClient, Network::Client::GetActiveClient().get()));
		}

		AppTickEngine::LoadProjectGenerators();
		Application::GetCurrentApp().SetAppStartTime();
	}

	void EditorLayer::OnSimulate()
	{
		*Scene::GetActiveScene()->GetHoveredEntity() = {};
		if (m_SceneState == SceneState::Play) { OnStop(); }

		m_SceneState = SceneState::Simulate;
		Scene::SetActiveScene(Scene::Copy(m_EditorScene));
		Scene::GetActiveScene()->OnSimulationStart();
	}
	void EditorLayer::OnStop()
	{
		*Scene::GetActiveScene()->GetHoveredEntity() = {};
		KG_ASSERT(m_SceneState == SceneState::Play || m_SceneState == SceneState::Simulate, "Unknown Scene State Given to OnSceneStop")

		if (m_SceneState == SceneState::Play) { Scene::GetActiveScene()->OnRuntimeStop(); }
		else if (m_SceneState == SceneState::Simulate) { Scene::GetActiveScene()->OnSimulationStop(); }

		Scene::GetActiveScene()->DestroyAllEntities();
		Scene::SetActiveScene(m_EditorScene);
		Audio::AudioEngine::StopAllAudio();

		// Clear UIObjects during runtime.
		if (m_EditorUIObject)
		{
			UI::Runtime::LoadUIObject(m_EditorUIObject, m_EditorUIObjectHandle);
		}
		else
		{
			UI::Runtime::ClearUIEngine();
		}

		// Clear InputModes during runtime.
		if (m_EditorInputMode)
		{
			InputMode::LoadInputMode(m_EditorInputMode, m_EditorInputModeHandle);
		}
		else
		{
			InputMode::s_InputMode = nullptr;
			InputMode::s_InputModeHandle = 0;
		}

		if (Projects::Project::GetAppIsNetworked() && m_SceneState == SceneState::Play)
		{

			Network::Client::GetActiveClient()->StopClient();
			Network::Client::GetActiveNetworkThread()->join();
			Network::Client::GetActiveNetworkThread().reset();
			Network::Client::GetActiveClient().reset();
		}

		AppTickEngine::ClearGenerators();

		m_SceneState = SceneState::Edit;
	}

	void EditorLayer::OnPause()
	{
		if (m_SceneState == SceneState::Edit) { return; }

		m_IsPaused = true;
	}

	void EditorLayer::OnDuplicateEntity()
	{
		if (m_SceneState != SceneState::Edit)
			return;

		Entity selectedEntity = *Scene::GetActiveScene()->GetSelectedEntity();
		if (selectedEntity)
		{
			Entity newEntity = m_EditorScene->DuplicateEntity(selectedEntity);
			m_SceneHierarchyPanel->SetSelectedEntity(newEntity);
		}
	}

}
