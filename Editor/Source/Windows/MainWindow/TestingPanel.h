#pragma once
#include "Kargono/Events/KeyEvent.h"
#include "Kargono/Core/FixedString.h"
#include "Kargono/EditorUI/EditorUI.h"

#include <string>

namespace Kargono::Panels
{
	class TestingPanel
	{
	public:
		//=========================
		// Constructor/Destructor
		//=========================
		TestingPanel();

		//=========================
		// Lifecycle Functions
		//=========================
		void InitializeDebugGlobalsWidgets();
		void InitializeGeneralTestingWidgets();

		//=========================
		// On Event
		//=========================
		void OnEditorUIRender();
		bool OnInputEvent(Events::Event* event);
		bool OnKeyPressedEditor(Events::KeyPressedEvent event);

	private:
		// Helper functions
		void DrawDebugGlobalWidgets();
		void DrawGeneralTestingWidgets();
	private:
		FixedString32 m_PanelName{ "Testing" };

		//==================================
		// Content browser widget test
		//==================================
	public:

#if defined(KG_DEBUG)
		// Debug global widgets
		EditorUI::CheckboxSpec s_EditTestBool_1;
		EditorUI::CheckboxSpec s_EditTestBool_2;
		EditorUI::CheckboxSpec s_EditTestBool_3;

		EditorUI::EditFloatSpec s_EditTestFloat_1;
		EditorUI::EditFloatSpec s_EditTestFloat_2;
		EditorUI::EditFloatSpec s_EditTestFloat_3;

		EditorUI::EditIntegerSpec s_EditTestInt_1;
		EditorUI::EditIntegerSpec s_EditTestInt_2;
		EditorUI::EditIntegerSpec s_EditTestInt_3;

		EditorUI::EditIntegerSpec s_EditTestUInt_1;
		EditorUI::EditIntegerSpec s_EditTestUInt_2;
		EditorUI::EditIntegerSpec s_EditTestUInt_3;

		EditorUI::EditVec2Spec s_EditTestVec2_1;
		EditorUI::EditVec2Spec s_EditTestVec2_2;
		EditorUI::EditVec2Spec s_EditTestVec2_3;

		EditorUI::EditVec3Spec s_EditTestVec3_1;
		EditorUI::EditVec3Spec s_EditTestVec3_2;
		EditorUI::EditVec3Spec s_EditTestVec3_3;

		EditorUI::EditVec4Spec s_EditTestVec4_1;
		EditorUI::EditVec4Spec s_EditTestVec4_2;
		EditorUI::EditVec4Spec s_EditTestVec4_3;

#endif
	};
}
