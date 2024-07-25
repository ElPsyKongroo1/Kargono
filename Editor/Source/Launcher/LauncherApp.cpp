
#include "Launcher/LauncherApp.h"

#include "imgui_internal.h"

namespace Kargono
{
	static LauncherApp* s_LauncherApp { nullptr };

	// Create Project Panel
	static EditorUI::GenericPopupSpec s_CreateProjectSpec {};
	static EditorUI::TextInputSpec s_CreateProjectName {};
	static EditorUI::ChooseDirectorySpec s_CreateProjectLocation {};

	static void InitializeStaticResources()
	{
		s_CreateProjectSpec.Label = "Create New Project";
		s_CreateProjectSpec.PopupWidth = 420.0f;
		s_CreateProjectSpec.PopupContents = [&]()
		{
			EditorUI::EditorUIService::TextInputPopup(s_CreateProjectName);
			EditorUI::EditorUIService::ChooseDirectory(s_CreateProjectLocation);
		};
		s_CreateProjectSpec.ConfirmAction = [&]()
		{
			s_LauncherApp->SetSelectedProject(Assets::AssetManager::CreateNewProject(s_CreateProjectName.CurrentOption, s_CreateProjectLocation.CurrentOption));
			if (!s_LauncherApp->GetSelectedProject().empty())
			{
				EngineService::EndRun();
			}
		};

		s_CreateProjectName.Label = "Project Title";
		s_CreateProjectName.CurrentOption = "NewProject";

		s_CreateProjectLocation.Label = "Project Location";
		s_CreateProjectLocation.CurrentOption = std::filesystem::current_path().parent_path() / "Projects";
	}

	void LauncherApp::Init()
	{
		s_LauncherApp = this;
		InitializeStaticResources();

		EngineService::GetActiveWindow().Init();

		EditorUI::EditorUIService::Init();

		EngineService::GetActiveWindow().SetResizable(false);
		EngineService::GetActiveWindow().CenterWindow();

		EngineService::GetActiveWindow().SetVisible(true);
	}
	void LauncherApp::Terminate()
	{
		EditorUI::EditorUIService::Terminate();
	}
	void LauncherApp::OnUpdate(Timestep ts)
	{
		EditorUI::EditorUIService::StartRendering();

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
		
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		EditorUI::EditorUIService::StartWindow("Launcher Screen", nullptr, window_flags);
		ImGui::PopStyleVar(3);

		EditorUI::EditorUIService::NewItemScreen(
			"Open Existing Project", [&]()
		{
			SelectProject();
			if (!m_SelectedProject.empty())
			{
				EngineService::EndRun();
			}
		}, 
		"Create New Project", [&]()
		{
			s_CreateProjectSpec.PopupActive = true;
		});

		EditorUI::EditorUIService::GenericPopup(s_CreateProjectSpec);

		EditorUI::EditorUIService::EndWindow();

		EditorUI::EditorUIService::EndRendering();
	}
	void LauncherApp::SelectProject()
	{
		std::filesystem::path initialDirectory = std::filesystem::current_path().parent_path() / "Projects";
		if (!std::filesystem::exists(initialDirectory))
		{
			initialDirectory = "";
		}
		m_SelectedProject = Utility::FileDialogs::OpenFile("Kargono Project (*.kproj)\0*.kproj\0", initialDirectory.string().c_str());
	}
}
