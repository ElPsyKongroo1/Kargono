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
		void InitializeCheckboxWidgetOptions();
		void InitializeInputTextWidgetOptions();
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
		void DrawGeneralWidgetOptions();
		void DrawTextWidgetOptions();
		void DrawButtonWidgetOptions();
		void DrawImageWidgetOptions();
		void DrawImageButtonWidgetOptions();
		void DrawCheckboxWidgetOptions();
		void DrawInputTextWidgetOptions();
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
		void OnOpenUIFontPopup(EditorUI::SelectOptionSpec& spec);
		// Modify on move function
		void OnModifyUIOnMove(const EditorUI::OptionEntry& entry);
		void OnOpenUIOnMovePopup(EditorUI::SelectOptionSpec& spec);
		void OnOpenTooltipForUIOnMove(EditorUI::SelectOptionSpec& spec);
		// Modify selection color
		void OnModifyUISelectionColor(EditorUI::EditVec4Spec& spec);
		void OnModifyUIHoveredColor(EditorUI::EditVec4Spec& spec);

		//=========================
		// Validation Functions
		//=========================
		bool ValidateActiveWindowAndWidget();
		bool ValidateActiveWindow();

		//=========================
		// Change Window Data
		//=========================
		// Modify tag
		void OnModifyWindowTag(EditorUI::EditTextSpec& spec);
		// Modify default widget
		void OnModifyWindowDefaultWidget(const EditorUI::OptionEntry& entry);
		void OnOpenWindowDefaultWidgetPopup(EditorUI::SelectOptionSpec& spec);
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
		void OnOpenWidgetXConstraint(EditorUI::SelectOptionSpec& spec);
		void OnOpenWidgetYConstraint(EditorUI::SelectOptionSpec& spec);
		void OnModifyWidgetXPixelLocation(EditorUI::EditIntegerSpec& spec);
		void OnModifyWidgetYPixelLocation(EditorUI::EditIntegerSpec& spec);
		void OnModifyWidgetXPercentLocation(EditorUI::EditFloatSpec& spec);
		void OnModifyWidgetYPercentLocation(EditorUI::EditFloatSpec& spec);

		//=========================
		// Change Button Widget Data
		//=========================
		// Modify widget on press
		void OnOpenTooltipForButtonWidgetOnPress(EditorUI::SelectOptionSpec& spec);

		//=========================
		// Change Image Button Widget Data
		//=========================
		// Modify widget on press
		void OnOpenTooltipForImageButtonWidgetOnPress(EditorUI::SelectOptionSpec& spec);

		//=========================
		// Change Checkbox Data
		//=========================
		// Modify if button widget is selectable
		void OnModifyCheckboxWidgetChecked(EditorUI::CheckboxSpec& spec);
		// Modify checkbox widget's checked image
		void OnModifyCheckboxWidgetCheckedImage(const EditorUI::OptionEntry& entry);
		void OnOpenCheckboxWidgetCheckedImagePopup(EditorUI::SelectOptionSpec& spec);
		// Modify checkbox widget's unchecked image
		void OnModifyCheckboxWidgetUnCheckedImage(const EditorUI::OptionEntry& entry);
		void OnOpenCheckboxWidgetUnCheckedImagePopup(EditorUI::SelectOptionSpec& spec);
		// Modify checkbox widget's fixed aspect ratio option
		void OnModifyCheckboxWidgetFixedAspectRatio(EditorUI::CheckboxSpec& spec);
		// Modify widget on press
		void OnOpenTooltipForCheckboxWidgetOnPress(EditorUI::SelectOptionSpec& spec);

		//=========================
		// Change Input Text Widget Data
		//=========================
		// Modify widget on press
		void OnOpenTooltipForInputTextWidgetOnPress(EditorUI::SelectOptionSpec& spec);

		//=========================
		// Modify UI Component Structs (Common data between widgets)
		//=========================
		// Generic text data
		void OnModifyTextDataTextSize(EditorUI::EditFloatSpec& spec);
		void OnModifyTextDataTextColor(EditorUI::EditVec4Spec& spec);
		void OnModifyTextDataAlignment(const EditorUI::OptionEntry& entry);
		void OnOpenTextDataAlignmentPopup(EditorUI::SelectOptionSpec& spec);
		// Single line text data
		void OnModifySingleLineDataText(EditorUI::EditTextSpec& spec);
		// Multi line text data
		void OnModifyMultiLineDataText(EditorUI::EditMultiLineTextSpec& spec);
		void OnModifyTextDataWrapped(EditorUI::CheckboxSpec& spec);
		// Selection data
		void OnModifySelectionDataSelectable(EditorUI::CheckboxSpec& spec);
		void OnModifySelectionDataOnPress(const EditorUI::OptionEntry& entry);
		void OnOpenSelectionDataOnPressPopup(EditorUI::SelectOptionSpec& spec);
		void OnModifySelectionDataBackgroundColor(EditorUI::EditVec4Spec& spec);
		// Image data
		void OnModifyImageDataImage(const EditorUI::OptionEntry& entry);
		void OnOpenImageDataImagePopup(EditorUI::SelectOptionSpec& spec);
		void OnModifyImageDataFixedAspectRatio(EditorUI::CheckboxSpec& spec);

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
		EditorUI::EditVec4Spec m_UIHoveredColor{};

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

		// Edit Checkbox Widget Options
		EditorUI::CollapsingHeaderSpec m_CheckboxWidgetHeader{};
		EditorUI::CheckboxSpec m_CheckboxWidgetChecked{};
		EditorUI::SelectOptionSpec m_CheckboxWidgetCheckedImage{};
		EditorUI::SelectOptionSpec m_CheckboxWidgetUnCheckedImage{};
		EditorUI::CheckboxSpec m_CheckboxWidgetFixedAspectRatio{};
		EditorUI::EditVec4Spec m_CheckboxWidgetBackgroundColor{};
		EditorUI::SelectOptionSpec m_CheckboxWidgetOnPress{};
		EditorUI::CheckboxSpec m_CheckboxWidgetSelectable{};

		// Edit InputText Widget Options
		EditorUI::CollapsingHeaderSpec m_InputTextWidgetHeader{};
		EditorUI::EditTextSpec m_InputTextWidgetText{};
		EditorUI::EditFloatSpec m_InputTextWidgetTextSize{};
		EditorUI::EditVec4Spec m_InputTextWidgetTextColor{};
		EditorUI::SelectOptionSpec m_InputTextWidgetTextAlignment{};
		EditorUI::EditVec4Spec m_InputTextWidgetBackgroundColor{};
		EditorUI::SelectOptionSpec m_InputTextWidgetOnPress{};
		EditorUI::CheckboxSpec m_InputTextWidgetSelectable{};
	};
}
