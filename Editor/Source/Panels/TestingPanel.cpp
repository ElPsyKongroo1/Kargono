#include "Panels/TestingPanel.h"

#include "EditorApp.h"
#include "Kargono.h"
#include "Kargono/Utility/Timers.h"
#include "Kargono/Scripting/ScriptCompilerService.h"

static Kargono::EditorApp* s_EditorApp { nullptr };

namespace Kargono::Panels
{

	static EditorUI::EditTextSpec s_TestText {};
	static EditorUI::EditFloatSpec s_TimerTime{};
	static EditorUI::EditIntegerSpec s_RandomTestInteger{};

	TestingPanel::TestingPanel()
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_EditorApp->m_PanelToKeyboardInput.insert_or_assign(m_PanelName,
			KG_BIND_CLASS_FN(TestingPanel::OnKeyPressedEditor));

		s_TestText.Label = "File to Compile";
		s_TestText.CurrentOption = "test.kgscript";

		s_TimerTime.Label = "Timer Time";
		s_TimerTime.CurrentFloat = 3.0f;

		s_RandomTestInteger.Label = "Intenger Time";
		s_RandomTestInteger.CurrentInteger = 5;
	}


	void TestingPanel::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION();
		EditorUI::EditorUIService::StartWindow(m_PanelName, &s_EditorApp->m_ShowTesting);
		// Exit window early if window is not visible
		if (!EditorUI::EditorUIService::IsCurrentWindowVisible())
		{
			EditorUI::EditorUIService::EndWindow();
			return;
		}

		EditorUI::EditorUIService::EditText(s_TestText);

		if (ImGui::Button("Compile File"))
		{
			KG_TRACE_CRITICAL(Scripting::ScriptCompilerService::CompileScriptFile("./../Projects/Pong/Assets/" + s_TestText.CurrentOption));
		}

		if (ImGui::Button("Test Popup"))
		{
			ImGui::OpenPopup("The Test Popup");
			/*ImGui::SetNextWindowPos(ImGui::GetWindowPos(), ImGuiCond_Once);
			ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiCond_Once);*/
		}

		EditorUI::EditorUIService::EditFloat(s_TimerTime);
		EditorUI::EditorUIService::EditInteger(s_RandomTestInteger);

		if (ImGui::Button("Start Timer"))
		{
			Utility::PassiveTimer::CreateTimer(s_TimerTime.CurrentFloat, []()
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

		EditorUI::EditorUIService::EndWindow();
	}
	bool TestingPanel::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		return false;
	}
}
