#include "Windows/MainWindow/TestingPanel.h"
#include "Windows/MainWindow/ViewportPanel.h"

#include "EditorApp.h"
#include "Kargono/Utility/Timers.h"
#include "Kargono/Scripting/ScriptCompilerService.h"
#include "Kargono/Utility/Random.h"

static Kargono::EditorApp* s_EditorApp { nullptr };
static Kargono::Windows::MainWindow* s_MainWindow{ nullptr };

namespace Kargono::Panels
{

	static EditorUI::EditTextSpec s_TestText {};
	static EditorUI::EditFloatSpec s_TimerTime{};
	static EditorUI::EditIntegerSpec s_RandomTestInteger{};

// TODO: Testing Splines
#if 0
	static std::vector<EditorUI::EditVec3Spec> s_ControlPointWidgets;
#endif

	static FixedString256 newString;
	static EditorUI::TooltipSpec testTooltip{};

	enum class TestTypes : uint32_t
	{
		None = 0,
		Display
	};

	
	TestingPanel::TestingPanel()
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_MainWindow = s_EditorApp->m_MainWindow.get();
		s_MainWindow->m_PanelToKeyboardInput.insert_or_assign(m_PanelName.CString(),
			KG_BIND_CLASS_FN(TestingPanel::OnKeyPressedEditor));

		s_TestText.m_Label = "File to Compile";
		s_TestText.m_CurrentOption = "test.kgscript";

		s_TimerTime.m_Label = "Timer Time";
		s_TimerTime.m_CurrentFloat = 3.0f;

		s_RandomTestInteger.m_Label = "Intenger Time";
		s_RandomTestInteger.m_CurrentInteger = 5;

		m_TestHeader.m_Label = "directory/directory/file.txt";
		newString = "Hahahaha";

		// Test tooltip api
		testTooltip.m_Label = "Test Tooltip";
		// Create menu items
		EditorUI::TooltipEntry newEntry{ "First Test Entry", [](EditorUI::TooltipEntry& entry) 
		{
			KG_WARN("Meow");
		} };

		testTooltip.AddTooltipEntry(std::move(newEntry));
// TODO Testing Splines
#if 0
		// TODO: Please Remove
		Math::Spline testSpline;
		testSpline.m_Points.push_back({ 10.0f, 41.0f, 0.0f });
		testSpline.m_Points.push_back({ 40.0f, 41.0f, 0.0f });
		testSpline.m_Points.push_back({ 70.0f, 41.0f, 0.0f });
		testSpline.m_Points.push_back({ 100.0f, 41.0f, 0.0f });
		testSpline.m_Points.push_back({ 130.0f, 41.0f, 0.0f });
		testSpline.m_Points.push_back({ 160.0f, 41.0f, 0.0f });
		testSpline.m_Looped = true;
		s_EditorApp->m_MainWindow->m_ViewportPanel->m_DebugSplines.push_back(testSpline);

		std::vector<Math::vec3>& allPoints = s_EditorApp->m_MainWindow->m_ViewportPanel->m_DebugSplines.at(0).m_Points;
		size_t iteration{ 0 };
		s_ControlPointWidgets.resize(allPoints.size());
		for (Math::vec3& point : allPoints)
		{
			EditorUI::EditVec3Spec& currentSpec = s_ControlPointWidgets.at(iteration);
			currentSpec.m_Label = "Control" + std::to_string(iteration);
			currentSpec.m_ScrollSpeed = 0.5f;
			currentSpec.m_ProvidedData = CreateRef<size_t>(iteration);
			currentSpec.m_ConfirmAction = [&](EditorUI::EditVec3Spec& spec)
			{
				// Get provided data
				size_t iteration = *(size_t*)spec.m_ProvidedData.get();

				Math::Spline& spline = s_EditorApp->m_MainWindow->m_ViewportPanel->m_DebugSplines.at(0);
				spline.m_Points.at(iteration) = spec.m_CurrentVec3;
			};
			iteration++;
		}
#endif

	}


	void TestingPanel::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION();
		EditorUI::EditorUIService::StartWindow(m_PanelName, &s_MainWindow->m_ShowTesting);
		// Exit window early if window is not visible
		if (!EditorUI::EditorUIService::IsCurrentWindowVisible())
		{
			EditorUI::EditorUIService::EndWindow();
			return;
		}

		EditorUI::EditorUIService::TitleText("Welcome to the Testing Panel!");

		if (ImGui::Button("Crit Log"))
		{
			KG_CRITICAL("Hey we in this bihh {}", Utility::RandomService::GenerateRandomInteger(0, 20));
		}

		if (ImGui::Button("Warn Log"))
		{
			KG_WARN("Hey we in this bihh {}", Utility::RandomService::GenerateRandomInteger(0, 20));
		}


		EditorUI::EditorUIService::EditText(s_TestText);

		if (ImGui::Button("Compile File"))
		{
			KG_TRACE_CRITICAL(Scripting::ScriptCompilerService::CompileScriptFile("./../Projects/Pong/Assets/" + s_TestText.m_CurrentOption));
		}

		if (ImGui::Button("Test Popup"))
		{
			ImGui::OpenPopup("The Test Popup");
		}

		EditorUI::EditorUIService::EditFloat(s_TimerTime);
		EditorUI::EditorUIService::EditInteger(s_RandomTestInteger);

		if (ImGui::Button("Start Timer"))
		{
			Utility::PassiveTimer::CreateTimer(s_TimerTime.m_CurrentFloat, []()
			{
				KG_WARN("The timer has gone off");
			});
		}

		ImGuiWindowFlags popupFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoFocusOnAppearing;

		bool closePopup = false;
		if (ImGui::Button("HSDHASDJ"))
		{
			closePopup = true;
		}

		if (ImGui::BeginPopup("The Test Popup", popupFlags))
		{
			EditorUI::EditorUIService::BringCurrentWindowToFront();
			
			ImGui::Text("Ayooo, the popup is open");
			if (closePopup)
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
// TODO: Testing Splines
#if 0 
		Math::Spline& spline = s_EditorApp->m_MainWindow->m_ViewportPanel->m_DebugSplines.at(0);
		size_t iteration{ 0 };
		for (Math::vec3& point : spline.m_Points)
		{
			EditorUI::EditVec3Spec& spec = s_ControlPointWidgets.at(iteration);
			spec.m_CurrentVec3 = spline.m_Points.at(iteration);
			EditorUI::EditorUIService::EditVec3(spec);
			iteration++;
		}
#endif

#if 0
		bool backActive = m_CurrentDirectory != std::filesystem::path(m_BaseDirectory);
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		if (!backActive)
		{
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
		}
		// Draw icon for moving a directory back
		if (ImGui::ImageButton((ImTextureID)(uint64_t)EditorUI::EditorUIService::s_IconBack->GetRendererID(),
			{ 24.0f, 24.0f }, { 0, 1 }, { 1, 0 },
			-1, ImVec4(0, 0, 0, 0),
			backActive ? EditorUI::EditorUIService::s_PrimaryTextColor : EditorUI::EditorUIService::s_DisabledColor))
		{
			if (backActive)
			{
				UpdateCurrentDirectory(m_CurrentDirectory.parent_path());
			}
		}
		if (!backActive)
		{
			ImGui::PopStyleColor(2);
		}
		if (backActive && ImGui::BeginDragDropTarget())
		{
			for (auto& payloadName : EditorUI::EditorUIService::s_AllPayloadTypes)
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payloadName.c_str()))
				{
					const wchar_t* payloadPathPointer = (const wchar_t*)payload->Data;
					std::filesystem::path payloadPath(payloadPathPointer);
					Utility::FileSystem::MoveFileToDirectory(payloadPath, m_CurrentDirectory.parent_path());
					break;
				}
			}
			ImGui::EndDragDropTarget();
		}

		// Draw icon for moving a directory forward
		bool forwardActive = m_CurrentDirectory != m_LongestRecentPath && !m_LongestRecentPath.empty();
		ImGui::SameLine();
		if (!forwardActive)
		{
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
		}
		if (ImGui::ImageButton((ImTextureID)(uint64_t)EditorUI::EditorUIService::s_IconForward->GetRendererID(),
			{ 24.0f, 24.0f }, { 0, 1 }, { 1, 0 },
			-1, ImVec4(0, 0, 0, 0),
			forwardActive ? EditorUI::EditorUIService::s_PrimaryTextColor : EditorUI::EditorUIService::s_DisabledColor))
		{
			if (forwardActive && Utility::FileSystem::DoesPathContainSubPath(m_CurrentDirectory, m_LongestRecentPath))
			{
				std::filesystem::path currentIterationPath{ m_LongestRecentPath };
				std::filesystem::path recentIterationPath{ m_LongestRecentPath };
				while (currentIterationPath != m_CurrentDirectory)
				{
					recentIterationPath = currentIterationPath;
					currentIterationPath = currentIterationPath.parent_path();
				}
				UpdateCurrentDirectory(recentIterationPath);
			}
		}
		if (!forwardActive)
		{
			ImGui::PopStyleColor(2);
		}
		if (forwardActive && ImGui::BeginDragDropTarget())
		{
			for (auto& payloadName : EditorUI::EditorUIService::s_AllPayloadTypes)
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payloadName.c_str()))
				{
					const wchar_t* payloadPathPointer = (const wchar_t*)payload->Data;
					std::filesystem::path payloadPath(payloadPathPointer);
					std::filesystem::path currentIterationPath{ m_LongestRecentPath };
					std::filesystem::path recentIterationPath{ m_LongestRecentPath };
					while (currentIterationPath != m_CurrentDirectory)
					{
						recentIterationPath = currentIterationPath;
						currentIterationPath = currentIterationPath.parent_path();
					}
					Utility::FileSystem::MoveFileToDirectory(payloadPath, recentIterationPath);
					break;
				}
			}
			ImGui::EndDragDropTarget();
		}
		ImGui::PopStyleColor();

		// Current directory title
		std::filesystem::path activeDirectory = Utility::FileSystem::GetRelativePath(Projects::ProjectService::GetActiveProjectDirectory(), m_CurrentDirectory);

		std::vector<std::string> tokenizedDirectoryPath{};

		while (activeDirectory.filename() != "Assets")
		{
			tokenizedDirectoryPath.push_back(activeDirectory.filename().string());
			activeDirectory = activeDirectory.parent_path();
		}
		tokenizedDirectoryPath.push_back("Assets");

		ImGui::PushFont(EditorUI::EditorUIService::s_FontPlexBold);
		for (int32_t i = (int32_t)(tokenizedDirectoryPath.size()) - 1; i >= 0; --i)
		{
			ImGui::SameLine();
			ImGui::Text(tokenizedDirectoryPath.at(i).c_str());
			if (i != 0)
			{
				ImGui::SameLine();
				ImGui::Text("/");
			}
		}
		ImGui::PopFont();
		ImGui::Separator();
#endif

#if 0
		EditorUI::EditorUIService::NavigationHeader(m_TestHeader);

		EditorUI::EditorUIService::Tooltip(testTooltip);
#endif

		EditorUI::EditorUIService::EndWindow();
	}
	bool TestingPanel::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		return false;
	}
	bool TestingPanel::OnInputEvent(Events::Event* event)
	{
		return false;
	}
}
