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
		Widget,
		UserInterface
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
		void InitializeUIOptions();
		void InitializeWindowOptions();
		void InitializeWidgetGeneralOptions();
		void InitializeTextWidgetOptions();
		void InitializeButtonWidgetOptions();
		void InitializeImageWidgetOptions();
		void InitializeImageButtonWidgetOptions();
		void InitializeWidgetLocationOptions();

	public:
		//=========================
		// On Event Functions
		//=========================
		void OnEditorUIRender();

		//=========================
		// Draw Properties Panel Functions
		//=========================
		void DrawUIOptions();
		void DrawWindowOptions();
		void DrawWidgetOptions();
		void DrawTextWidgetOptions();
		void DrawButtonWidgetOptions();
		void DrawImageWidgetOptions();
		void DrawImageButtonWidgetOptions();
		void DrawSpecificWidgetOptions();

		//=========================
		// Modify Panel Data
		//=========================
		void ClearPanelData();

	private:

		//=========================
		// Change UI Data
		//=========================
		// Modify font
		void OnModifyUIFont(const EditorUI::OptionEntry& entry);
		void OnOpenUIFontPopup();
		// Modify on move function
		void OnModifyUIOnMove(const EditorUI::OptionEntry& entry);
		void OnOpenUIOnMovePopup();
		void OnOpenTooltipForUIOnMove();
		// Modify selection color
		void OnModifyUISelectionColor(EditorUI::EditVec4Spec& spec);

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
		// Modify widget size
		void OnModifyWidgetSizePixelOrPercent();
		void OnModifyWidgetPercentSize(EditorUI::EditVec2Spec& spec);
		void OnModifyWidgetPixelSize(EditorUI::EditIVec2Spec& spec);
		// Modify widget location
		void OnModifyWidgetXLocationRelOrAbs();
		void OnModifyWidgetYLocationRelOrAbs();
		void OnModifyWidgetXLocationPixelOrPercent();
		void OnModifyWidgetYLocationPixelOrPercent();
		void OnModifyWidgetXConstraint(const EditorUI::OptionEntry& entry);
		void OnModifyWidgetYConstraint(const EditorUI::OptionEntry& entry);
		void OnOpenWidgetXConstraint();
		void OnOpenWidgetYConstraint();
		void OnModifyWidgetXPixelLocation(EditorUI::EditIntegerSpec& spec);
		void OnModifyWidgetYPixelLocation(EditorUI::EditIntegerSpec& spec);
		void OnModifyWidgetXPercentLocation(EditorUI::EditFloatSpec& spec);
		void OnModifyWidgetYPercentLocation(EditorUI::EditFloatSpec& spec);
		
		//=========================
		// Change Text Widget Data
		//=========================
		// Modify text widget's text
		void OnModifyTextWidgetText(EditorUI::EditMultiLineTextSpec& spec);
		// Modify text widget's text size
		void OnModifyTextWidgetTextSize(EditorUI::EditFloatSpec& spec);
		// Modify text widget's text color
		void OnModifyTextWidgetTextColor(EditorUI::EditVec4Spec& spec);
		// Modify if text widget's text is aligned
		void OnModifyTextWidgetAlignment(const EditorUI::OptionEntry& entry);
		void OnOpenTextWidgetAlignmentPopup();
		// Modify if text widget's text is wrapped
		void OnModifyTextWidgetWrapped(EditorUI::CheckboxSpec& spec);

		//=========================
		// Change Button Widget Data
		//=========================
		// Modify the button widget's text
		void OnModifyButtonWidgetText(EditorUI::EditTextSpec& spec);
		// Modify button widget's text size
		void OnModifyButtonWidgetButtonTextSize(EditorUI::EditFloatSpec& spec);
		// Modify button widget's text color
		void OnModifyButtonWidgetButtonTextColor(EditorUI::EditVec4Spec& spec);
		// Modify if button widget's text is aligned
		void OnModifyButtonWidgetTextAlignment(const EditorUI::OptionEntry& entry);
		void OnOpenButtonWidgetTextAlignmentPopup();
		// Modify if button widget is selectable
		void OnModifyButtonWidgetSelectable(EditorUI::CheckboxSpec& spec);
		// Modify widget on press
		void OnModifyButtonWidgetOnPress(const EditorUI::OptionEntry& entry);
		void OnOpenButtonWidgetOnPressPopup();
		void OnOpenTooltipForButtonWidgetOnPress();
		// Modify widget background color
		void OnModifyButtonWidgetBackgroundColor(EditorUI::EditVec4Spec& spec);

		//=========================
		// Change Image Widget Data
		//=========================
		// Modify image widget's image
		void OnModifyImageWidgetImage(const EditorUI::OptionEntry& entry);
		void OnOpenImageWidgetImagePopup();
		// Modify image widget's fixed aspect ratio option
		void OnModifyImageWidgetFixedAspectRatio(EditorUI::CheckboxSpec& spec);

		//=========================
		// Change Image Widget Data
		//=========================
		// Modify image button widget's image
		void OnModifyImageButtonWidgetImage(const EditorUI::OptionEntry& entry);
		void OnOpenImageButtonWidgetImagePopup();
		// Modify image button widget's fixed aspect ratio option
		void OnModifyImageButtonWidgetFixedAspectRatio(EditorUI::CheckboxSpec& spec);
		// Modify if button widget is selectable
		void OnModifyImageButtonWidgetSelectable(EditorUI::CheckboxSpec& spec);
		// Modify widget on press
		void OnModifyImageButtonWidgetOnPress(const EditorUI::OptionEntry& entry);
		void OnOpenImageButtonWidgetOnPressPopup();
		void OnOpenTooltipForImageButtonWidgetOnPress();
		// Modify widget background color
		void OnModifyImageButtonWidgetBackgroundColor(EditorUI::EditVec4Spec& spec);

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
		
		// Edit UI Options
		EditorUI::CollapsingHeaderSpec m_UIHeader{};
		EditorUI::SelectOptionSpec m_UISelectFont{};
		EditorUI::SelectOptionSpec m_UIOnMove{};
		EditorUI::EditVec4Spec m_UISelectionColor{};

		// Edit Window Options
		EditorUI::CollapsingHeaderSpec m_WindowHeader{};
		EditorUI::EditTextSpec m_WindowTag{};
		EditorUI::SelectOptionSpec m_WindowDefaultWidget{};
		EditorUI::CheckboxSpec m_WindowDisplay{};
		EditorUI::EditVec3Spec m_WindowLocation{};
		EditorUI::EditVec2Spec m_WindowSize{};
		EditorUI::EditVec4Spec m_WindowBackgroundColor{};

		// Edit Widget Options
		EditorUI::CollapsingHeaderSpec m_WidgetGeneralHeader{};
		EditorUI::EditTextSpec m_WidgetTag{};
		EditorUI::RadioSelectorSpec m_WidgetPixelOrPercentSize{};
		EditorUI::EditVec2Spec m_WidgetPercentSize{};
		EditorUI::EditIVec2Spec m_WidgetPixelSize{};

		// Edit Widget Location Options
		EditorUI::CollapsingHeaderSpec m_WidgetLocationHeader{};
		EditorUI::RadioSelectorSpec m_WidgetXPixelOrPercentLocation{};
		EditorUI::RadioSelectorSpec m_WidgetYPixelOrPercentLocation{};
		EditorUI::RadioSelectorSpec m_WidgetXRelOrAbsLocation{};
		EditorUI::RadioSelectorSpec m_WidgetYRelOrAbsLocation{};
		EditorUI::SelectOptionSpec m_WidgetXConstraintLocation{};
		EditorUI::SelectOptionSpec m_WidgetYConstraintLocation{};
		EditorUI::EditIntegerSpec m_WidgetXPixelLocation{};
		EditorUI::EditIntegerSpec m_WidgetYPixelLocation{};
		EditorUI::EditFloatSpec m_WidgetXPercentLocation{};
		EditorUI::EditFloatSpec m_WidgetYPercentLocation{};

		// Edit Text Widget Options
		EditorUI::CollapsingHeaderSpec m_TextWidgetHeader{};
		EditorUI::EditMultiLineTextSpec m_TextWidgetText{};
		EditorUI::EditFloatSpec m_TextWidgetTextSize{};
		EditorUI::EditVec4Spec m_TextWidgetTextColor{};
		EditorUI::SelectOptionSpec m_TextWidgetTextAlignment{};
		EditorUI::CheckboxSpec m_TextWidgetTextWrapped{};

		// Edit Button Widget Options
		EditorUI::CollapsingHeaderSpec m_ButtonWidgetHeader{};
		EditorUI::EditTextSpec m_ButtonWidgetText{};
		EditorUI::EditFloatSpec m_ButtonWidgetTextSize{};
		EditorUI::EditVec4Spec m_ButtonWidgetTextColor{};
		EditorUI::SelectOptionSpec m_ButtonWidgetTextAlignment{};
		EditorUI::EditVec4Spec m_ButtonWidgetBackgroundColor{};
		EditorUI::SelectOptionSpec m_ButtonWidgetOnPress{};
		EditorUI::CheckboxSpec m_ButtonWidgetSelectable{};

		// Edit Image Widget Options
		EditorUI::CollapsingHeaderSpec m_ImageWidgetHeader{};
		EditorUI::SelectOptionSpec m_ImageWidgetImage{};
		EditorUI::CheckboxSpec m_ImageWidgetFixedAspectRatio{};

		// Edit Image Button Widget Options
		EditorUI::CollapsingHeaderSpec m_ImageButtonWidgetHeader{};
		EditorUI::SelectOptionSpec m_ImageButtonWidgetImage{};
		EditorUI::CheckboxSpec m_ImageButtonWidgetFixedAspectRatio{};
		EditorUI::EditVec4Spec m_ImageButtonWidgetBackgroundColor{};
		EditorUI::SelectOptionSpec m_ImageButtonWidgetOnPress{};
		EditorUI::CheckboxSpec m_ImageButtonWidgetSelectable{};
	};
}
