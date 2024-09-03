#include "Panels/TestingPanel.h"

#include "EditorApp.h"
#include "Kargono.h"

static Kargono::EditorApp* s_EditorApp { nullptr };

namespace Kargono::Panels
{

	static EditorUI::EditTextSpec s_TestText {};

	TestingPanel::TestingPanel()
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_EditorApp->m_PanelToKeyboardInput.insert_or_assign(m_PanelName,
			KG_BIND_CLASS_FN(TestingPanel::OnKeyPressedEditor));

		s_TestText.Label = "File to Compile";
		s_TestText.CurrentOption = "NewScripting/test.kgscript";
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
			Scripting::ScriptCompiler::CompileScriptFile("./../Projects/Pong/Assets/" + s_TestText.CurrentOption);
		}

		if (ImGui::Button("Test Popup"))
		{
			ImGui::OpenPopup("The Test Popup");
			/*ImGui::SetNextWindowPos(ImGui::GetWindowPos(), ImGuiCond_Once);
			ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiCond_Once);*/
		}

		//ImGuiWindowFlags_None = 0,
		//	ImGuiWindowFlags_NoTitleBar = 1 << 0,   // Disable title-bar
		//	ImGuiWindowFlags_NoResize = 1 << 1,   // Disable user resizing with the lower-right grip
		//	ImGuiWindowFlags_NoMove = 1 << 2,   // Disable user moving the window
		//	ImGuiWindowFlags_NoScrollbar = 1 << 3,   // Disable scrollbars (window can still scroll with mouse or programmatically)
		//	ImGuiWindowFlags_NoScrollWithMouse = 1 << 4,   // Disable user vertically scrolling with mouse wheel. On child window, mouse wheel will be forwarded to the parent unless NoScrollbar is also set.
		//	ImGuiWindowFlags_NoCollapse = 1 << 5,   // Disable user collapsing window by double-clicking on it. Also referred to as Window Menu Button (e.g. within a docking node).
		//	ImGuiWindowFlags_AlwaysAutoResize = 1 << 6,   // Resize every window to its content every frame
		//	ImGuiWindowFlags_NoBackground = 1 << 7,   // Disable drawing background color (WindowBg, etc.) and outside border. Similar as using SetNextWindowBgAlpha(0.0f).
		//	ImGuiWindowFlags_NoSavedSettings = 1 << 8,   // Never load/save settings in .ini file
		//	ImGuiWindowFlags_NoMouseInputs = 1 << 9,   // Disable catching mouse, hovering test with pass through.
		//	ImGuiWindowFlags_MenuBar = 1 << 10,  // Has a menu-bar
		//	ImGuiWindowFlags_HorizontalScrollbar = 1 << 11,  // Allow horizontal scrollbar to appear (off by default). You may use SetNextWindowContentSize(ImVec2(width,0.0f)); prior to calling Begin() to specify width. Read code in imgui_demo in the "Horizontal Scrolling" section.
		//	ImGuiWindowFlags_NoFocusOnAppearing = 1 << 12,  // Disable taking focus when transitioning from hidden to visible state
		//	ImGuiWindowFlags_NoBringToFrontOnFocus = 1 << 13,  // Disable bringing window to front when taking focus (e.g. clicking on it or programmatically giving it focus)
		//	ImGuiWindowFlags_AlwaysVerticalScrollbar = 1 << 14,  // Always show vertical scrollbar (even if ContentSize.y < Size.y)
		//	ImGuiWindowFlags_AlwaysHorizontalScrollbar = 1 << 15,  // Always show horizontal scrollbar (even if ContentSize.x < Size.x)
		//	ImGuiWindowFlags_AlwaysUseWindowPadding = 1 << 16,  // Ensure child windows without border uses style.WindowPadding (ignored by default for non-bordered child windows, because more convenient)
		//	ImGuiWindowFlags_NoNavInputs = 1 << 18,  // No gamepad/keyboard navigation within the window
		//	ImGuiWindowFlags_NoNavFocus = 1 << 19,  // No focusing toward this window with gamepad/keyboard navigation (e.g. skipped by CTRL+TAB)
		//	ImGuiWindowFlags_UnsavedDocument = 1 << 20,  // Display a dot next to the title. When used in a tab/docking context, tab is selected when clicking the X + closure is not assumed (will wait for user to stop submitting the tab). Otherwise closure is assumed when pressing the X, so if you keep submitting the tab may reappear at end of tab bar.
		//	ImGuiWindowFlags_NoDocking

		//ImGui::ImGuiWindowFlag

		ImGuiWindowFlags popupFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoFocusOnAppearing;

		if (ImGui::BeginPopup("The Test Popup", popupFlags))
		{
			EditorUI::EditorUIService::BringCurrentWindowToFront();
			
			ImGui::Text("Ayooo, the popup is open");
			ImGui::EndPopup();
		}

		EditorUI::EditorUIService::EndWindow();
	}
	bool TestingPanel::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		return false;
	}
}
