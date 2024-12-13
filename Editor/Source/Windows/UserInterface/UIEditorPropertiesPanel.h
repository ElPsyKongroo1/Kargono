#pragma once
#include "Kargono/Events/KeyEvent.h"
#include "Kargono/EditorUI/EditorUI.h"
#include "Kargono/Core/FixedString.h"
#include "Kargono/RuntimeUI/RuntimeUI.h"

#include <string>

namespace Kargono::Panels
{
	//=========================
	// UI Properties Display Enum
	//=========================
	enum class UIPropertiesDisplay : uint16_t
	{
		None = 0,
		Window,
		Widget
	};

	class UIEditorPropertiesPanel
	{
	public:
		//=========================
		// Constructor/Destructor
		//=========================
		UIEditorPropertiesPanel();

	private:
		//=========================
		// Internal Initialization Functions
		//=========================
		void InitializeWindowOptions();
		void InitializeWidgetOptions();

	public:
		//=========================
		// On Event Functions
		//=========================
		void OnEditorUIRender();

		//=========================
		// Draw Properties Panel Functions
		//=========================
		void DrawWindowOptions();
		void DrawWidgetOptions();

		//=========================
		// Modify Panel Data
		//=========================
		void ClearPanelData();

	private:
		//=========================
		// Change Window Data
		//=========================
		// Modify tag
		void OnModifyWindowTag(EditorUI::EditTextSpec& spec);
		// Modify default widget
		void OnModifyWindowDefaultWidget(const EditorUI::OptionEntry& entry);
		void OnOpenWindowDefaultWidgetPopup();
		// Modify if window is displayed
		void OnModifyWindowDisplay(EditorUI::CheckboxSpec& spec);
		// Modify window location
		void OnModifyWindowLocation(EditorUI::EditVec3Spec& spec);
		// Modify window size
		void OnModifyWindowSize(EditorUI::EditVec2Spec& spec);
		// Modify window background color
		void OnModifyWindowBackgroundColor(EditorUI::EditVec4Spec& spec);

		//=========================
		// Change Widget Base Data
		//=========================
		// Modify widget tag
		void OnModifyWidgetTag(EditorUI::EditTextSpec& spec);
		// Modify widget location
		void OnModifyWidgetLocation(EditorUI::EditVec2Spec& spec);
		// Modify widget size
		void OnModifyWidgetSize(EditorUI::EditVec2Spec& spec);
		// Modify widget background color
		void OnModifyWidgetBackgroundColor(EditorUI::EditVec4Spec& spec);
		// Modify widget on press
		void OnModifyWidgetOnPress(const EditorUI::OptionEntry& entry);
		void OnOpenWidgetOnPressPopup();
		void OnOpenTooltipForWidgetOnPress();

		//=========================
		// Change Text Widget Data
		//=========================
		// Modify widget text
		void OnModifyTextWidgetText(EditorUI::EditTextSpec& spec);
		// Modify widget text size
		void OnModifyTextWidgetTextSize(EditorUI::EditFloatSpec& spec);
		// Modify widget text color
		void OnModifyTextWidgetTextColor(EditorUI::EditVec4Spec& spec);
		// Modify if widget text is centered
		void OnModifyTextWidgetCentered(EditorUI::CheckboxSpec& spec);

	public:
		//=========================
		// Core Panel Data
		//=========================
		FixedString32 m_PanelName{ "User Interface Properties" };
		UIPropertiesDisplay m_CurrentDisplay{ UIPropertiesDisplay::None };
		RuntimeUI::Widget* m_ActiveWidget{ nullptr };
		RuntimeUI::Window* m_ActiveWindow{ nullptr };

	public:
		//=========================
		// Widgets
		//=========================
		// Edit Window Options
		EditorUI::CollapsingHeaderSpec m_WindowHeader{};
		EditorUI::EditTextSpec m_WindowTag{};
		EditorUI::SelectOptionSpec m_WindowDefaultWidget{};
		EditorUI::CheckboxSpec m_WindowDisplay{};
		EditorUI::EditVec3Spec m_WindowLocation{};
		EditorUI::EditVec2Spec m_WindowSize{};
		EditorUI::EditVec4Spec m_WindowBackgroundColor{};

		// Edit Widget Options
		EditorUI::CollapsingHeaderSpec m_WidgetHeader{};
		EditorUI::EditTextSpec m_WidgetTag{};
		EditorUI::EditVec2Spec m_WidgetLocation{};
		EditorUI::EditVec2Spec m_WidgetSize{};
		EditorUI::EditVec4Spec m_WidgetBackgroundColor{};
		EditorUI::SelectOptionSpec m_WidgetOnPress{};
		EditorUI::EditTextSpec m_WidgetText{};
		EditorUI::EditFloatSpec m_WidgetTextSize{};
		EditorUI::EditVec4Spec m_WidgetTextColor{};
		EditorUI::CheckboxSpec m_WidgetCentered{};
	};
}
