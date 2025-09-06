#pragma once
#include "Modules/Events/KeyEvent.h"
#include "Kargono/Core/FixedString.h"
#include "Modules/EditorUI/EditorUIInclude.h"

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
		EditorUI::CheckboxWidget s_EditTestBool_1;
		EditorUI::CheckboxWidget s_EditTestBool_2;
		EditorUI::CheckboxWidget s_EditTestBool_3;

		EditorUI::EditFloatWidget s_EditTestFloat_1;
		EditorUI::EditFloatWidget s_EditTestFloat_2;
		EditorUI::EditFloatWidget s_EditTestFloat_3;

		EditorUI::EditIntegerWidget s_EditTestInt_1;
		EditorUI::EditIntegerWidget s_EditTestInt_2;
		EditorUI::EditIntegerWidget s_EditTestInt_3;

		EditorUI::EditIntegerWidget s_EditTestUInt_1;
		EditorUI::EditIntegerWidget s_EditTestUInt_2;
		EditorUI::EditIntegerWidget s_EditTestUInt_3;

		EditorUI::EditVec2Widget s_EditTestVec2_1;
		EditorUI::EditVec2Widget s_EditTestVec2_2;
		EditorUI::EditVec2Widget s_EditTestVec2_3;

		EditorUI::EditVec3Widget s_EditTestVec3_1;
		EditorUI::EditVec3Widget s_EditTestVec3_2;
		EditorUI::EditVec3Widget s_EditTestVec3_3;

		EditorUI::EditVec4Widget s_EditTestVec4_1;
		EditorUI::EditVec4Widget s_EditTestVec4_2;
		EditorUI::EditVec4Widget s_EditTestVec4_3;

#endif
	};
}
