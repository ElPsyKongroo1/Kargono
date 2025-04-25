
#include "Launcher/LauncherApp.h"

#include "EditorUIModule/EditorUI.h"
#include "Kargono/Projects/Project.h"
#include "Kargono/Core/Engine.h"
#include "Kargono/Utility/OSCommands.h"
#include "Kargono/Utility/FileDialogs.h"
#include "RenderModule/Texture.h"

#include "EditorUIModule/ExternalAPI/ImGuiBackendAPI.h"

namespace Kargono
{
	static LauncherApp* s_LauncherApp { nullptr };

	// Create Project Panel
	static EditorUI::GenericPopupSpec s_CreateProjectSpec {};
	static EditorUI::EditTextSpec s_CreateProjectName {};
	static EditorUI::ChooseDirectorySpec s_CreateProjectLocation {};

	static void InitializeStaticResources()
	{
		s_CreateProjectSpec.m_Label = "Create New Project";
		s_CreateProjectSpec.m_PopupWidth = 420.0f;
		s_CreateProjectSpec.m_PopupContents = [&]()
		{
			EditorUI::EditorUIService::EditText(s_CreateProjectName);
			EditorUI::EditorUIService::ChooseDirectory(s_CreateProjectLocation);
		};
		s_CreateProjectSpec.m_ConfirmAction = [&]()
		{
			s_LauncherApp->SetSelectedProject(Projects::ProjectService::CreateNewProject(s_CreateProjectName.m_CurrentOption, s_CreateProjectLocation.m_CurrentOption));
			if (!s_LauncherApp->GetSelectedProject().empty())
			{
				EngineService::EndRun();
			}
		};

		s_CreateProjectName.m_Label = "Project Title";
		s_CreateProjectName.m_CurrentOption = "NewProject";

		s_CreateProjectLocation.m_Label = "Project Location";
		s_CreateProjectLocation.m_CurrentOption = std::filesystem::current_path().parent_path() / "Projects";
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

	bool LauncherApp::OnLogEvent(Events::Event* /*event*/)
	{

		// TODO: Figure out better solution. This currently looks super wonkyyyyy
		//Events::LogEvent* logEvent = (Events::LogEvent*)event;

		//switch (logEvent->GetEventLevel())
		//{
		//case Events::LogEventLevel::Info:
		//	EditorUI::EditorUIService::CreateInfoNotification(logEvent->GetEventText().c_str(), 5000);
		//	break;
		//case Events::LogEventLevel::Warning:
		//	EditorUI::EditorUIService::CreateWarningNotification(logEvent->GetEventText().c_str(), 5000);
		//	break;
		//case Events::LogEventLevel::Critical:
		//	EditorUI::EditorUIService::CreateCriticalNotification(logEvent->GetEventText().c_str(), 8000);
		//	break;
		//case Events::LogEventLevel::None:
		//default:
		//	// TODO: Yee, I realize this is a potential circular issue. ehh...
		//	KG_ERROR("Invalid log event type provided to OnLogEvent()")
		//		break;
		//}
		return false;
	}

	Math::vec2 LauncherApp::GetMouseViewportPosition()
	{
		KG_WARN("Attempt to retrieve mouse location from unimplemented function in Launcher Application");
		return Math::vec2();
	}

	ViewportData* LauncherApp::GetViewportData()
	{
		KG_WARN("Attempt to retrieve mouse location from unimplemented function in Launcher Application");
		return nullptr;
	}


	void LauncherApp::OnUpdate(Timestep /*ts*/)
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
			s_CreateProjectSpec.m_OpenPopup = true;
		});

		EditorUI::EditorUIService::GenericPopup(s_CreateProjectSpec);



		ImGui::PushStyleColor(ImGuiCol_Button, EditorUI::EditorUIService::s_PureEmpty);
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec2 initialScreenCursorPos = ImGui::GetWindowPos() + ImGui::GetCursorStartPos();
		ImVec2 initialCursorPos = ImGui::GetCursorStartPos();

		
		// Draw Background
		ImVec2 windowSize = ImGui::GetWindowSize();
		Ref<Rendering::Texture2D> icon {nullptr};
		draw_list->AddRectFilled(ImVec2(initialScreenCursorPos.x + windowSize.x - 30.0f, initialScreenCursorPos.y),
			ImVec2(initialScreenCursorPos.x + (windowSize.x), initialScreenCursorPos.y + 30.0f),
			ImColor(EditorUI::EditorUIService::s_DarkBackgroundColor), 12.0f, ImDrawFlags_RoundCornersBottomLeft);

		// Generate Download Image
		icon = EditorUI::EditorUIService::s_IconDown;
		ImGui::SetCursorPos(ImVec2(initialCursorPos.x + windowSize.x - 25, initialCursorPos.y + 4));
		if (ImGui::ImageButton("Download Samples Button",
			(ImTextureID)(uint64_t)icon->GetRendererID(),
			ImVec2(14, 14), ImVec2{ 0, 1 }, ImVec2{ 1, 0 },
			EditorUI::EditorUIService::s_PureEmpty,
			EditorUI::EditorUIService::s_HighlightColor1))
		{
			ImGui::OpenPopup("Download Samples Popup");
		}
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::TextColored(EditorUI::EditorUIService::s_HighlightColor1, "Get Sample Projects");
			ImGui::EndTooltip();
		}

		
		if (ImGui::BeginPopup("Download Samples Popup"))
		{
			if (ImGui::MenuItem("Get Pong"))
			{
				Utility::FileSystem::CreateNewDirectory("../Projects/");
				Utility::OSCommands::DownloadGitProject("../Projects/Pong", "https://github.com/ElPsyKongroo1/Pong.git");
			}
			ImGui::EndPopup();
		}

		ImGui::PopStyleColor();

		EditorUI::EditorUIService::EndWindow();

		EditorUI::EditorUIService::EndRendering();
	}
	void LauncherApp::SelectProject()
	{
		std::filesystem::path initialDirectory = std::filesystem::current_path().parent_path() / "Projects";
		if (!Utility::FileSystem::PathExists(initialDirectory))
		{
			initialDirectory = "";
		}
		m_SelectedProject = Utility::FileDialogs::OpenFile("Kargono Project (*.kproj)\0*.kproj\0", initialDirectory.string().c_str());
	}
}
