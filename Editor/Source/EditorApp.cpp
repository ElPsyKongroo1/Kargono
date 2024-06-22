#include "kgpch.h"

#include "EditorApp.h"

#include "API/EditorUI/ImGuiBackendAPI.h"

namespace Kargono
{

	EditorApp* EditorApp::s_EditorApp = nullptr;

	EditorApp::EditorApp()
		: Application("EditorLayer")
	{
		s_EditorApp = this;
	}

	void EditorApp::OnAttach()
	{
		Script::ScriptEngine::Init();
		Scripting::ScriptCore::Init();
		Audio::AudioEngine::Init();
		Scenes::SceneEngine::Init();

		m_SceneHierarchyPanel = CreateScope<Panels::SceneHierarchyPanel>();

		m_EditorScene = CreateRef<Scenes::Scene>();
		Scenes::Scene::SetActiveScene(m_EditorScene, m_EditorSceneHandle);
		m_SceneState = SceneState::Edit;

		if (!OpenProject())
		{
			EngineCore::GetCurrentEngineCore().Close();
			return;
		}
		EditorUI::Editor::Init();

		m_LogPanel = CreateScope<Panels::LogPanel>();
		m_StatisticsPanel = CreateScope<Panels::StatisticsPanel>();
		m_ProjectPanel = CreateScope<Panels::ProjectPanel>();
		m_UIEditorPanel = CreateScope<Panels::UIEditorPanel>();
		m_ViewportPanel = CreateScope<Panels::ViewportPanel>();
		m_ScriptEditorPanel = CreateScope<Panels::ScriptEditorPanel>();
		m_OldInputEditorPanel = CreateScope<Panels::OldInputEditorPanel>();
		m_EntityClassEditor = CreateScope<Panels::EntityClassEditor>();
		m_TextEditorPanel = CreateScope<Panels::TextEditorPanel>();
		m_GameStatePanel = CreateScope<Panels::GameStatePanel>();
		m_InputModePanel = CreateScope<Panels::InputModePanel>();

		m_ViewportPanel->InitializeFrameBuffer();

		Rendering::RenderingEngine::Init();
		Rendering::RenderingEngine::SetLineWidth(1.0f);
		RuntimeUI::Text::Init();
		RuntimeUI::Runtime::Init();

		m_ViewportPanel->m_EditorCamera = Rendering::EditorCamera(30.0f, 1.778f, 0.1f, 1000.0f);
		
		m_ViewportPanel->InitializeOverlayData();

		EngineCore::GetActiveWindow().SetVisible(true);
	}

	void EditorApp::OnDetach()
	{
		auto allAudioComponents = m_EditorScene->GetAllEntitiesWith<Scenes::AudioComponent>();
		for (auto& entity : allAudioComponents)
		{
			Scenes::Entity e = { entity, m_EditorScene.get()};
			auto& audioComponent = e.GetComponent<Scenes::AudioComponent>();
			audioComponent.Audio.reset();
		}
		auto allMultiAudioComponents = m_EditorScene->GetAllEntitiesWith<Scenes::MultiAudioComponent>();
		for (auto& entity : allMultiAudioComponents)
		{
			Scenes::Entity e = { entity, m_EditorScene.get() };
			auto& multiAudioComponent = e.GetComponent<Scenes::MultiAudioComponent>();
			for (auto& [key, component] : multiAudioComponent.AudioComponents)
			{
				component.Audio.reset();
			}
		}

		EditorUI::Editor::Terminate();
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
		EditorUI::Editor::StartRendering();

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
		EditorUI::Editor::StartWindow("DockSpace", &dockspaceOpen, window_flags);
		if (!opt_padding)
			ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// Set up Menu Toolbar
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open Project ...", "Ctrl+O"))
				{
					OpenProject();
				}

				if (ImGui::MenuItem("Save Project", "Ctrl+S"))
				{
					SaveScene();
					SaveProject();
				}

				ImGui::Separator();

				if (ImGui::MenuItem("New Scene", "Ctrl+N"))
				{
					NewScene();
				}

				ImGui::Separator();

				if (ImGui::MenuItem("Reload Script Module"))
				{
					Scripting::ScriptCore::LoadActiveScriptModule();
				}
				if (ImGui::MenuItem("Rebuild Script Module"))
				{
					Scripting::ScriptModuleBuilder::CreateScriptModule();
				}

				ImGui::Separator();

				if (ImGui::MenuItem("Exit"))
				{
					EngineCore::GetCurrentEngineCore().Close();
				}
				ImGui::EndMenu();

			}

			if (ImGui::BeginMenu("Panels"))
			{
				ImGui::MenuItem("Content Browser", NULL, &m_ShowContentBrowser);
				ImGui::MenuItem("Scene Hierarchy", NULL, &m_ShowSceneHierarchy);
				ImGui::MenuItem("Viewport", NULL, &m_ShowViewport);
				ImGui::Separator();
				ImGui::MenuItem("User Interface Editor", NULL, &m_ShowUserInterfaceEditor);
				ImGui::MenuItem("Old Input Mode Editor", NULL, &m_ShowOldInputEditor);
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
				ImGui::MenuItem("ImGui Demo", NULL, &m_ShowDemoWindow);
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
			EditorUI::Editor::EndWindow();
			EditorUI::Editor::EndRendering();
			return;
		}

		// Display other panels
		if (m_ShowSceneHierarchy) { m_SceneHierarchyPanel->OnEditorUIRender(); }
		if (m_ShowContentBrowser) { m_ContentBrowserPanel->OnEditorUIRender(); }
		if (m_ShowLog) { m_LogPanel->OnEditorUIRender(); }
		if (m_ShowStats) { m_StatisticsPanel->OnEditorUIRender(); }
		if (m_ShowViewport) { m_ViewportPanel->OnEditorUIRender(); }
		if (m_ShowUserInterfaceEditor) { m_UIEditorPanel->OnEditorUIRender(); }
		if (m_ShowOldInputEditor) { m_OldInputEditorPanel->OnEditorUIRender(); }
		if (m_ShowProject) { m_ProjectPanel->OnEditorUIRender(); }
		if (m_ShowScriptEditor) { m_ScriptEditorPanel->OnEditorUIRender(); }
		if (m_ShowClassEditor) { m_EntityClassEditor->OnEditorUIRender(); }
		if (m_ShowTextEditor) { m_TextEditorPanel->OnEditorUIRender(); }
		if (m_ShowGameStateEditor) { m_GameStatePanel->OnEditorUIRender(); }
		if (m_ShowInputModeEditor) { m_InputModePanel->OnEditorUIRender(); }
		if (m_ShowDemoWindow) { ImGui::ShowDemoWindow(&m_ShowDemoWindow); }
		

		EditorUI::Editor::EndWindow();

		EditorUI::Editor::HighlightFocusedWindow();

		EditorUI::Editor::EndRendering();
	}


	void EditorApp::OnEvent(Events::Event& event)
	{
		EditorUI::Editor::OnEvent(event);
		if (event.Handled)
		{
			return;
		}
		std::string focusedWindow = EditorUI::Editor::GetFocusedWindowName();
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
		
		Script::ScriptEngine::OnKeyPressed(event);
		
		return false;
	}

	bool EditorApp::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		if (event.IsRepeat()) { return false; }

		std::string focusedWindow = EditorUI::Editor::GetFocusedWindowName();
		if (m_PanelToKeyboardInput.contains(focusedWindow))
		{
			if (m_PanelToKeyboardInput.at(focusedWindow)(event))
			{
				return true;
			}
		}

		bool control = Input::InputPolling::IsKeyPressed(Key::LeftControl) || Input::InputPolling::IsKeyPressed(Key::RightControl);
		bool shift = Input::InputPolling::IsKeyPressed(Key::LeftShift) || Input::InputPolling::IsKeyPressed(Key::RightShift);
		bool alt = Input::InputPolling::IsKeyPressed(Key::LeftAlt) || Input::InputPolling::IsKeyPressed(Key::RightAlt);

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
				EngineCore::GetActiveWindow().ToggleMaximized();
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
			if (EditorUI::Editor::GetActiveWidgetID() == 0)
			{
				Scenes::Entity selectedEntity = *Scenes::Scene::GetActiveScene()->GetSelectedEntity();
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
			if (m_ViewportPanel->m_ViewportHovered && !ImGuizmo::IsOver() && !Input::InputPolling::IsKeyPressed(Key::LeftAlt) && *Scenes::Scene::GetActiveScene()->GetHoveredEntity())
			{
				m_SceneHierarchyPanel->SetSelectedEntity(*Scenes::Scene::GetActiveScene()->GetHoveredEntity());
				// Algorithm to enable double clicking for an entity!
				static float previousTime{ 0.0f };
				static Scenes::Entity previousEntity {};
				float currentTime = Utility::Time::GetTime();
				if (std::fabs(currentTime - previousTime) < 0.2f && *Scenes::Scene::GetActiveScene()->GetHoveredEntity() == previousEntity)
				{
					auto& transformComponent = Scenes::Scene::GetActiveScene()->GetHoveredEntity()->GetComponent<Scenes::TransformComponent>();
					m_ViewportPanel->m_EditorCamera.SetFocalPoint(transformComponent.Translation);
					m_ViewportPanel->m_EditorCamera.SetDistance(std::max({ transformComponent.Scale.x, transformComponent.Scale.y, transformComponent.Scale.z }) * 2.5f);
					m_ViewportPanel->m_EditorCamera.SetMovementType(Rendering::EditorCamera::MovementType::ModelView);
				}
				previousTime = currentTime;
				previousEntity = *Scenes::Scene::GetActiveScene()->GetHoveredEntity();

			}
		}
		return false;
	}


	bool EditorApp::OnPhysicsCollision(Events::PhysicsCollisionEvent event)
	{
		Script::ScriptEngine::OnPhysicsCollision(event);
		return false;
	}

	bool EditorApp::OnPhysicsCollisionEnd(Events::PhysicsCollisionEnd event)
	{
		Script::ScriptEngine::OnPhysicsCollisionEnd(event);
		return false;
	}

	bool EditorApp::OnUpdateUserCount(Events::UpdateOnlineUsers event)
	{
		Assets::AssetHandle scriptHandle = Projects::Project::GetOnUpdateUserCount();
		if (scriptHandle != Assets::EmptyHandle)
		{
			((WrappedVoidUInt32*)Assets::AssetManager::GetScript(scriptHandle)->m_Function.get())->m_Value(event.GetUserCount());
		}

		return false;
	}

	bool EditorApp::OnApproveJoinSession(Events::ApproveJoinSession event)
	{
		Assets::AssetHandle scriptHandle = Projects::Project::GetOnApproveJoinSession();
		if (scriptHandle != Assets::EmptyHandle)
		{
			((WrappedVoidUInt16*)Assets::AssetManager::GetScript(scriptHandle)->m_Function.get())->m_Value(event.GetUserSlot());
		}

		return false;
	}

	bool EditorApp::OnUpdateSessionUserSlot(Events::UpdateSessionUserSlot event)
	{
		Assets::AssetHandle scriptHandle = Projects::Project::GetOnUpdateSessionUserSlot();
		if (scriptHandle != Assets::EmptyHandle)
		{
			((WrappedVoidUInt16*)Assets::AssetManager::GetScript(scriptHandle)->m_Function.get())->m_Value(event.GetUserSlot());
		}

		return false;
	}

	bool EditorApp::OnUserLeftSession(Events::UserLeftSession event)
	{
		Assets::AssetHandle scriptHandle = Projects::Project::GetOnUserLeftSession();
		if (scriptHandle != Assets::EmptyHandle)
		{
			((WrappedVoidUInt16*)Assets::AssetManager::GetScript(scriptHandle)->m_Function.get())->m_Value(event.GetUserSlot());
		}
		return false;
	}

	bool EditorApp::OnCurrentSessionInit(Events::CurrentSessionInit event)
	{
		Assets::AssetHandle scriptHandle = Projects::Project::GetOnCurrentSessionInit();
		if (scriptHandle != Assets::EmptyHandle)
		{
			((WrappedVoidNone*)Assets::AssetManager::GetScript(scriptHandle)->m_Function.get())->m_Value();
		}
		return false;
	}

	bool EditorApp::OnConnectionTerminated(Events::ConnectionTerminated event)
	{
		Assets::AssetHandle scriptHandle = Projects::Project::GetOnConnectionTerminated();
		if (scriptHandle != Assets::EmptyHandle)
		{
			((WrappedVoidNone*)Assets::AssetManager::GetScript(scriptHandle)->m_Function.get())->m_Value();
		}
		return false;
	}

	bool EditorApp::OnStartSession(Events::StartSession event)
	{
		Assets::AssetHandle scriptHandle = Projects::Project::GetOnStartSession();
		if (scriptHandle != Assets::EmptyHandle)
		{
			((WrappedVoidNone*)Assets::AssetManager::GetScript(scriptHandle)->m_Function.get())->m_Value();
		}
		return false;
	}

	bool EditorApp::OnSessionReadyCheckConfirm(Events::SessionReadyCheckConfirm event)
	{
		Script::ScriptEngine::RunCustomCallsFunction(Projects::Project::GetProjectOnSessionReadyCheckConfirm());
		return false;
	}

	bool EditorApp::OnReceiveSignal(Events::ReceiveSignal event)
	{
		uint16_t signal = event.GetSignal();
		void* param = &signal;
		Script::ScriptEngine::RunCustomCallsFunction(Projects::Project::GetProjectOnReceiveSignal(), &param);
		return false;
	}

	void EditorApp::NewProject()
	{
		Assets::AssetManager::NewProject();
	}

	bool EditorApp::OpenProject()
	{
		*Scenes::Scene::GetActiveScene()->GetHoveredEntity() = {};
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

	void EditorApp::OpenProject(const std::filesystem::path& path)
	{
		if (Assets::AssetManager::OpenProject(path))
		{
			if (!EngineCore::GetCurrentEngineCore().GetWindow().GetNativeWindow())
			{
				
				EngineCore::GetActiveWindow().Init();
				Rendering::RendererAPI::Init();
			}
			auto startSceneHandle = Projects::Project::GetStartSceneHandle();

			Scripting::ScriptCore::LoadActiveScriptModule();

			if (Script::ScriptEngine::AppDomainExists()){ Script::ScriptEngine::ReloadAssembly(); }
			else { Script::ScriptEngine::InitialAssemblyLoad(); }
			if (m_EditorScene)
			{
				auto view = m_EditorScene->GetAllEntitiesWith<Scenes::AudioComponent>();
				for (auto& entity : view)
				{
					Scenes::Entity e = { entity, m_EditorScene.get() };
					auto& audioComponent = e.GetComponent<Scenes::AudioComponent>();
					audioComponent.Audio.reset();
				}
				m_EditorScene->DestroyAllEntities();
			}
			Assets::AssetManager::ClearAll();
			Assets::AssetManager::DeserializeAll();

			OpenScene(startSceneHandle);

			m_ContentBrowserPanel = CreateScope<Panels::ContentBrowserPanel>();
		}
	}

	void EditorApp::SaveProject()
	{
		Assets::AssetManager::SaveActiveProject((Projects::Project::GetProjectDirectory() / Projects::Project::GetProjectName()).replace_extension(".kproj"));
	}

	void EditorApp::NewScene()
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

		*Scenes::Scene::GetActiveScene()->GetHoveredEntity() = {};
		m_EditorScene = Assets::AssetManager::GetScene(m_EditorSceneHandle);
		Scenes::Scene::SetActiveScene(m_EditorScene, m_EditorSceneHandle);
	}

	void EditorApp::OpenScene()
	{
		std::filesystem::path initialDirectory = Projects::Project::GetAssetDirectory();
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
		Scenes::Scene::SetActiveScene(m_EditorScene, m_EditorSceneHandle);
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
		Scenes::Scene::SetActiveScene(m_EditorScene, m_EditorSceneHandle);
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
		// Cache Current UIObject/InputMode in editor
		if (!RuntimeUI::Runtime::GetCurrentUIObject()) { m_EditorUIObject = nullptr; }
		else
		{
			RuntimeUI::Runtime::SaveCurrentUIIntoUIObject();
			m_EditorUIObject = RuntimeUI::Runtime::GetCurrentUIObject();
			m_EditorUIObjectHandle = RuntimeUI::Runtime::GetCurrentUIHandle();
		}

		if (!Input::InputMode::GetActiveInputMode()) { m_EditorInputMode = nullptr; }
		else
		{
			m_EditorInputMode = Input::InputMode::GetActiveInputMode();
			m_EditorInputModeHandle = Input::InputMode::GetActiveInputModeHandle();
		}

		// Load Default Game State
		if (Projects::Project::GetStartGameState() == 0)
		{
			Scenes::GameState::s_GameState = nullptr;
			Scenes::GameState::s_GameStateHandle = 0;
		}
		else
		{
			Scenes::GameState::s_GameState = Assets::AssetManager::GetGameState(Projects::Project::GetStartGameState());
			Scenes::GameState::s_GameStateHandle = Projects::Project::GetStartGameState();
		}

		*Scenes::Scene::GetActiveScene()->GetHoveredEntity() = {};
		if (m_SceneState == SceneState::Simulate) { OnStop(); }

		m_SceneState = SceneState::Play;
		Scenes::Scene::SetActiveScene(Scenes::Scene::Copy(m_EditorScene), m_EditorSceneHandle);
		Scenes::Scene::GetActiveScene()->OnRuntimeStart();
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
		EngineCore::GetCurrentEngineCore().SetAppStartTime();
		EditorUI::Editor::SetFocusedWindow(m_ViewportPanel->m_PanelName);
	}

	void EditorApp::OnSimulate()
	{
		*Scenes::Scene::GetActiveScene()->GetHoveredEntity() = {};
		if (m_SceneState == SceneState::Play) { OnStop(); }

		m_SceneState = SceneState::Simulate;
		Scenes::Scene::SetActiveScene(Scenes::Scene::Copy(m_EditorScene), m_EditorSceneHandle);
		Scenes::Scene::GetActiveScene()->OnSimulationStart();
	}
	void EditorApp::OnStop()
	{
		*Scenes::Scene::GetActiveScene()->GetHoveredEntity() = {};
		KG_ASSERT(m_SceneState == SceneState::Play || m_SceneState == SceneState::Simulate, "Unknown Scene State Given to OnSceneStop")

		if (m_SceneState == SceneState::Play) { Scenes::Scene::GetActiveScene()->OnRuntimeStop(); }
		else if (m_SceneState == SceneState::Simulate) { Scenes::Scene::GetActiveScene()->OnSimulationStop(); }

		Scenes::Scene::GetActiveScene()->DestroyAllEntities();
		Scenes::Scene::SetActiveScene(m_EditorScene, m_EditorSceneHandle);
		Audio::AudioEngine::StopAllAudio();

		// Clear UIObjects during runtime.
		if (m_EditorUIObject)
		{
			RuntimeUI::Runtime::LoadUIObject(m_EditorUIObject, m_EditorUIObjectHandle);
		}
		else
		{
			RuntimeUI::Runtime::ClearUIEngine();
		}

		// Clear InputModes during runtime.
		if (m_EditorInputMode)
		{
			Input::InputMode::SetActiveInputMode(m_EditorInputMode, m_EditorInputModeHandle);
		}
		else
		{
			Input::InputMode::SetActiveInputMode(nullptr, Assets::EmptyHandle);
		}

		Scenes::GameState::s_GameState = nullptr;
		Scenes::GameState::s_GameStateHandle = 0;

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

	void EditorApp::OnPause()
	{
		if (m_SceneState == SceneState::Edit) { return; }

		m_IsPaused = true;
	}

	void EditorApp::OnDuplicateEntity()
	{
		if (m_SceneState != SceneState::Edit)
			return;

		Scenes::Entity selectedEntity = *Scenes::Scene::GetActiveScene()->GetSelectedEntity();
		if (selectedEntity)
		{
			Scenes::Entity newEntity = m_EditorScene->DuplicateEntity(selectedEntity);
			m_SceneHierarchyPanel->SetSelectedEntity(newEntity);
		}
	}

}
