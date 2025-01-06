#include "Windows/EmitterConfig/EmitterConfigPropertiesPanel.h"

#include "EditorApp.h"

static Kargono::EditorApp* s_EditorApp{ nullptr };
static Kargono::Windows::MainWindow* s_MainWindow{ nullptr };
static Kargono::Windows::EmitterConfigWindow* s_EmitterConfigWindow{ nullptr };

namespace Kargono::Panels
{
	EmitterConfigPropertiesPanel::EmitterConfigPropertiesPanel()
	{
		// Get the main window and UI window
		s_EditorApp = EditorApp::GetCurrentApp();
		s_MainWindow = s_EditorApp->m_MainWindow.get();
		s_EmitterConfigWindow = s_EditorApp->m_EmitterConfigEditorWindow.get();

		// Initialize properties panel widget resources
		InitializeEmitterConfigOptions();
	}
	void EmitterConfigPropertiesPanel::InitializeEmitterConfigOptions()
	{
		//// Set up header for window options
		//m_UIHeader.m_Label = "User Interface Options";
		//m_UIHeader.m_Flags |= EditorUI::CollapsingHeaderFlags::CollapsingHeader_UnderlineTitle;
		//m_UIHeader.m_Expanded = true;

		//// Set up widget to modify the UI's font
		//m_UISelectFont.m_Label = "Font";
		//m_UISelectFont.m_Flags |= EditorUI::SelectOption_Indented;
		//m_UISelectFont.m_PopupAction = KG_BIND_CLASS_FN(OnOpenUIFontPopup);
		//m_UISelectFont.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyUIFont);

		//// Set up widget to modify the UI's OnMove functions
		//m_UIOnMove.m_Label = "On Move";
		//m_UIOnMove.m_Flags |= EditorUI::SelectOption_Indented | EditorUI::SelectOption_HandleEditButtonExternally;
		//m_UIOnMove.m_PopupAction = KG_BIND_CLASS_FN(OnOpenUIOnMovePopup);
		//m_UIOnMove.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyUIOnMove);
		//m_UIOnMove.m_OnEdit = KG_BIND_CLASS_FN(OnOpenTooltipForUIOnMove);

		//// Set up widget to modify the UI's selection background color
		//m_UISelectionColor.m_Label = "Selection Color";
		//m_UISelectionColor.m_Flags |= EditorUI::EditVec4_Indented | EditorUI::EditVec4_RGBA;
		//m_UISelectionColor.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyUISelectionColor);
	}
	void EmitterConfigPropertiesPanel::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION();
		EditorUI::EditorUIService::StartWindow(m_PanelName, &s_EmitterConfigWindow->m_ShowProperties);

		// Early out if the window is not visible
		if (!EditorUI::EditorUIService::IsCurrentWindowVisible())
		{
			EditorUI::EditorUIService::EndWindow();
			return;
		}

		// Draw header
		EditorUI::EditorUIService::PanelHeader(s_EmitterConfigWindow->m_MainHeader);

		// Draw configuration options
		DrawEmitterConfigOptions();

		// End the window
		EditorUI::EditorUIService::EndWindow();
	}

	void EmitterConfigPropertiesPanel::DrawEmitterConfigOptions()
	{
		//// Draw main header for UI options
		//EditorUI::EditorUIService::CollapsingHeader(m_UIHeader);

		//// Draw options for UI
		//if (m_UIHeader.m_Expanded)
		//{
		//	// Edit font for current UI
		//	Assets::AssetHandle fontHandle = s_EmitterConfigWindow->m_EditorUI->m_FontHandle;
		//	m_UISelectFont.m_CurrentOption =
		//	{
		//		fontHandle == Assets::EmptyHandle ? "None" : Assets::AssetService::GetFontInfo(fontHandle).Data.FileLocation.stem().string(),
		//		fontHandle
		//	};
		//	EditorUI::EditorUIService::SelectOption(m_UISelectFont);

		//	// Edit on move for current UI
		//	Assets::AssetHandle onMoveHandle = s_EmitterConfigWindow->m_EditorUI->m_FunctionPointers.m_OnMoveHandle;
		//	m_UIOnMove.m_CurrentOption =
		//	{
		//		onMoveHandle == Assets::EmptyHandle ? "None" : Assets::AssetService::GetScriptInfo(onMoveHandle).Data.FileLocation.stem().string(),
		//		onMoveHandle
		//	};
		//	EditorUI::EditorUIService::SelectOption(m_UIOnMove);

		//	// Edit UI's selection color
		//	m_UISelectionColor.m_CurrentVec4 = s_EmitterConfigWindow->m_EditorUI->m_SelectColor;
		//	EditorUI::EditorUIService::EditVec4(m_UISelectionColor);
		//}
	}

	void EmitterConfigPropertiesPanel::ClearPanelData()
	{
	}
}

