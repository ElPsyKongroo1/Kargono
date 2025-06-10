#pragma once
#include "Modules/Events/KeyEvent.h"
#include "Modules/EditorUI/EditorUIInclude.h"
#include "Kargono/Core/FixedString.h"
#include "Modules/RuntimeUI/RuntimeUIContext.h"

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
		void InitializeContainerWidgetOptions();
		void InitializeHorizontalContainerWidgetOptions();
		void InitializeVerticalContainerWidgetOptions();
		void InitializeInputTextWidgetOptions();
		void InitializeSliderWidgetOptions();
		void InitializeDropDownWidgetOptions();
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
		void DrawContainerWidgetOptions();
		void DrawHorizontalContainerWidgetOptions();
		void DrawVerticalContainerWidgetOptions();
		void DrawInputTextWidgetOptions();
		void DrawSliderWidgetOptions();
		void DrawDropDownWidgetOptions();
		void DrawSpecificWidgetOptions();

		//=========================
		// Modify Panel Data
		//=========================
		void OnSelectWidget();
		void OpenWidgetTagDialog();
		void OpenWindowTagDialog();
		void ClearPanelData();

	private:

		//=========================
		// Change UI Data
		//=========================
		// Modify font
		void OnModifyUIFont(const EditorUI::OptionEntry& entry);
		void OnOpenUIFontPopup(EditorUI::SelectOptionWidget& spec);
		// Modify on move function
		void OnModifyUIOnMove(const EditorUI::OptionEntry& entry);
		void OnOpenUIOnMovePopup(EditorUI::SelectOptionWidget& spec);
		void OnOpenTooltipForUIOnMove(EditorUI::SelectOptionWidget& spec);
		// Modify on Hover function
		void OnModifyUIOnHover(const EditorUI::OptionEntry& entry);
		void OnOpenUIOnHoverPopup(EditorUI::SelectOptionWidget& spec);
		void OnOpenTooltipForUIOnHover(EditorUI::SelectOptionWidget& spec);
		// Modify selection color
		void OnModifyUISelectionColor(EditorUI::EditVec4Widget& spec);
		void OnModifyUIHoveredColor(EditorUI::EditVec4Widget& spec);
		void OnModifyUIEditingColor(EditorUI::EditVec4Widget& spec);

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
		void OnOpenWindowDefaultWidgetPopup(EditorUI::SelectOptionWidget& spec);
		// Modify if window is displayed
		void OnModifyWindowDisplay(EditorUI::CheckboxWidget& spec);
		// Modify window location
		void OnModifyWindowLocation(EditorUI::EditVec3Widget& spec);
		// Modify window size
		void OnModifyWindowSize(EditorUI::EditVec2Widget& spec);
		// Modify window background color
		void OnModifyWindowBackgroundColor(EditorUI::EditVec4Widget& spec);

		//=========================
		// Change Widget Base Data
		//=========================
		// Modify widget tag
		void OnModifyWidgetTag(EditorUI::EditTextSpec& spec);
		// Modify widget size
		void OnModifyWidgetSizePixelOrPercent();
		void OnModifyWidgetPercentSize(EditorUI::EditVec2Widget& spec);
		void OnModifyWidgetPixelSize(EditorUI::EditIVec2Widget& spec);
		// Modify widget location
		void OnModifyWidgetXLocationRelOrAbs();
		void OnModifyWidgetYLocationRelOrAbs();
		void OnModifyWidgetXLocationPixelOrPercent();
		void OnModifyWidgetYLocationPixelOrPercent();
		void OnModifyWidgetXConstraint(const EditorUI::OptionEntry& entry);
		void OnModifyWidgetYConstraint(const EditorUI::OptionEntry& entry);
		void OnOpenWidgetXConstraint(EditorUI::SelectOptionWidget& spec);
		void OnOpenWidgetYConstraint(EditorUI::SelectOptionWidget& spec);
		void OnModifyWidgetXPixelLocation(EditorUI::EditIntegerWidget& spec);
		void OnModifyWidgetYPixelLocation(EditorUI::EditIntegerWidget& spec);
		void OnModifyWidgetXPercentLocation(EditorUI::EditFloatWidget& spec);
		void OnModifyWidgetYPercentLocation(EditorUI::EditFloatWidget& spec);

		//=========================
		// Change Button Widget Data
		//=========================

		//=========================
		// Change Image Button Widget Data
		//=========================

		//=========================
		// Change Checkbox Data
		//=========================
		// Modify if button widget is selectable
		void OnModifyCheckboxWidgetChecked(EditorUI::CheckboxWidget& spec);
		// Modify checkbox widget's checked image
		void OnModifyCheckboxWidgetCheckedImage(const EditorUI::OptionEntry& entry);
		void OnOpenCheckboxWidgetCheckedImagePopup(EditorUI::SelectOptionWidget& spec);
		// Modify checkbox widget's unchecked image
		void OnModifyCheckboxWidgetUnCheckedImage(const EditorUI::OptionEntry& entry);
		void OnOpenCheckboxWidgetUnCheckedImagePopup(EditorUI::SelectOptionWidget& spec);
		// Modify checkbox widget's fixed aspect ratio option
		void OnModifyCheckboxWidgetFixedAspectRatio(EditorUI::CheckboxWidget& spec);

		//=========================
		// Change Input Text Widget Data
		//=========================
		// Modify widget on move cursor
		void OnModifyInputTextOnMoveCursor(const EditorUI::OptionEntry& entry);
		void OnOpenInputTextOnMoveCursorPopup(EditorUI::SelectOptionWidget& spec);
		void OnOpenTooltipForInputTextWidgetOnMoveCursor(EditorUI::SelectOptionWidget& spec);

		//=========================
		// Change Slider Widget Data
		//=========================
		// Modify widget bounds
		void OnModifySliderWidgetBounds(EditorUI::EditVec2Widget& spec);
		// Modify widget colors
		void OnModifySliderWidgetSliderColor(EditorUI::EditVec4Widget& spec);
		void OnModifySliderWidgetLineColor(EditorUI::EditVec4Widget& spec);
		// Modify widget on move slider
		void OnModifySliderWidgetOnMoveSlider(const EditorUI::OptionEntry& entry);
		void OnOpenSliderWidgetOnMoveSliderPopup(EditorUI::SelectOptionWidget& spec);
		void OnOpenTooltipForSliderWidgetOnMoveSlider(EditorUI::SelectOptionWidget& spec);

		//=========================
		// Change Drop-Down Widget Data
		//=========================
		// Modify all of the drop-down widget's text
		void OnModifyDropDownTextSize(EditorUI::EditFloatWidget& spec);
		void OnModifyDropDownTextColor(EditorUI::EditVec4Widget& spec);
		void OnModifyDropDownAlignment(const EditorUI::OptionEntry& entry);

		// Modify widget's drop down background color
		void OnModifyDropDownWidgetOptionBackgroundColor(EditorUI::EditVec4Widget& spec);

		// Modify widget on select option DropDown
		void OnModifyDropDownWidgetOnSelectOption(const EditorUI::OptionEntry& entry);
		void OnOpenDropDownWidgetOnSelectOptionPopup(EditorUI::SelectOptionWidget& spec);
		void OnOpenTooltipForDropDownWidgetOnSelectOption(EditorUI::SelectOptionWidget& spec);

		// Modify widget open
		void OnModifyDropDownWidgetOpen(EditorUI::CheckboxWidget& spec);

		// Modify widget's current option
		void OnModifyDropDownWidgetCurrentOption(const EditorUI::OptionEntry& entry);
		void OnOpenDropDownWidgetCurrentOptionPopup(EditorUI::SelectOptionWidget& spec);

		// Modify widget's options list
		void OnRefreshDropDownWidgetOptionsList();
		void OnDropDownWidgetAddEntryDialog();
		void OnDropDownWidgetEditEntryTooltip(EditorUI::ListEntry& entry, size_t iteration);
		void OnDropDownWidgetEditEntryDialog(EditorUI::TooltipEntry& entry);
		void OnDropDownWidgetDeleteEntryDialog(EditorUI::TooltipEntry& entry);
		void OnDropDownWidgetAddEntry(EditorUI::EditTextSpec& spec);
		void OnDropDownWidgetEditEntry(EditorUI::EditTextSpec& spec);
		void OnDropDownWidgetDeleteEntry();

		//=========================
		// Change Horizontal Container Widget Data
		//=========================
		void OnModifyHorizontalContainerWidgetColumnWidth(EditorUI::EditFloatWidget& spec);
		void OnModifyHorizontalContainerWidgetColumnSpacing(EditorUI::EditFloatWidget& spec);

		//=========================
		// Change Vertical Container Widget Data
		//=========================
		void OnModifyVerticalContainerWidgetRowHeight(EditorUI::EditFloatWidget& spec);
		void OnModifyVerticalContainerWidgetRowSpacing(EditorUI::EditFloatWidget& spec);

		//=========================
		// Modify UI Component Structs (Common data between widgets)
		//=========================
		// Generic text data
		void OnModifyTextDataTextSize(EditorUI::EditFloatWidget& spec);
		void OnModifyTextDataTextColor(EditorUI::EditVec4Widget& spec);
		void OnModifyTextDataAlignment(const EditorUI::OptionEntry& entry);
		void OnOpenTextDataAlignmentPopup(EditorUI::SelectOptionWidget& spec);
		// Single line text data
		void OnModifySingleLineDataText(EditorUI::EditTextSpec& spec);
		// Multi line text data
		void OnModifyMultiLineDataText(EditorUI::EditMultiLineTextWidget& spec);
		void OnModifyTextDataWrapped(EditorUI::CheckboxWidget& spec);
		// Selection data
		void OnModifySelectionDataSelectable(EditorUI::CheckboxWidget& spec);
		void OnModifySelectionDataOnPress(const EditorUI::OptionEntry& entry);
		void OnOpenSelectionDataOnPressPopup(EditorUI::SelectOptionWidget& spec);
		void OnOpenTooltipForSelectionDataOnPress(EditorUI::SelectOptionWidget& spec);
		void OnModifySelectionDataBackgroundColor(EditorUI::EditVec4Widget& spec);
		// Image data
		void OnModifyImageDataImage(const EditorUI::OptionEntry& entry);
		void OnOpenImageDataImagePopup(EditorUI::SelectOptionWidget& spec);
		void OnModifyImageDataFixedAspectRatio(EditorUI::CheckboxWidget& spec);
		// Container data
		void OnModifyContainerDataBackgroundColor(EditorUI::EditVec4Widget& spec);
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
		EditorUI::CollapsingHeaderWidget m_UIHeader{};
		EditorUI::SelectOptionWidget m_UISelectFont{};
		EditorUI::SelectOptionWidget m_UIOnMove{};
		EditorUI::SelectOptionWidget m_UIOnHover{};
		EditorUI::EditVec4Widget m_UISelectionColor{};
		EditorUI::EditVec4Widget m_UIHoveredColor{};
		EditorUI::EditVec4Widget m_UIEditingColor{};

		// Edit Window Options
		EditorUI::CollapsingHeaderWidget m_WindowHeader{};
		EditorUI::EditTextSpec m_WindowTag{};
		EditorUI::SelectOptionWidget m_WindowDefaultWidget{};
		EditorUI::CheckboxWidget m_WindowDisplay{};
		EditorUI::EditVec3Widget m_WindowLocation{};
		EditorUI::EditVec2Widget m_WindowSize{};
		EditorUI::EditVec4Widget m_WindowBackgroundColor{};

		// Edit Widget Options
		EditorUI::CollapsingHeaderWidget m_WidgetGeneralHeader{};
		EditorUI::EditTextSpec m_WidgetTag{};
		EditorUI::RadioSelectWidget m_WidgetPixelOrPercentSize{};
		EditorUI::EditVec2Widget m_WidgetPercentSize{};
		EditorUI::EditIVec2Widget m_WidgetPixelSize{};

		// Edit Widget Location Options
		EditorUI::CollapsingHeaderWidget m_WidgetLocationHeader{};
		EditorUI::RadioSelectWidget m_WidgetXPixelOrPercentLocation{};
		EditorUI::RadioSelectWidget m_WidgetYPixelOrPercentLocation{};
		EditorUI::RadioSelectWidget m_WidgetXRelOrAbsLocation{};
		EditorUI::RadioSelectWidget m_WidgetYRelOrAbsLocation{};
		EditorUI::SelectOptionWidget m_WidgetXConstraintLocation{};
		EditorUI::SelectOptionWidget m_WidgetYConstraintLocation{};
		EditorUI::EditIntegerWidget m_WidgetXPixelLocation{};
		EditorUI::EditIntegerWidget m_WidgetYPixelLocation{};
		EditorUI::EditFloatWidget m_WidgetXPercentLocation{};
		EditorUI::EditFloatWidget m_WidgetYPercentLocation{};

		// Edit Text Widget Options
		EditorUI::CollapsingHeaderWidget m_TextWidgetHeader{};
		EditorUI::EditMultiLineTextWidget m_TextWidgetText{};
		EditorUI::EditFloatWidget m_TextWidgetTextSize{};
		EditorUI::EditVec4Widget m_TextWidgetTextColor{};
		EditorUI::SelectOptionWidget m_TextWidgetTextAlignment{};
		EditorUI::CheckboxWidget m_TextWidgetTextWrapped{};

		// Edit Button Widget Options
		EditorUI::CollapsingHeaderWidget m_ButtonWidgetHeader{};
		EditorUI::EditTextSpec m_ButtonWidgetText{};
		EditorUI::EditFloatWidget m_ButtonWidgetTextSize{};
		EditorUI::EditVec4Widget m_ButtonWidgetTextColor{};
		EditorUI::SelectOptionWidget m_ButtonWidgetTextAlignment{};
		EditorUI::EditVec4Widget m_ButtonWidgetBackgroundColor{};
		EditorUI::SelectOptionWidget m_ButtonWidgetOnPress{};
		EditorUI::CheckboxWidget m_ButtonWidgetSelectable{};

		// Edit Image Widget Options
		EditorUI::CollapsingHeaderWidget m_ImageWidgetHeader{};
		EditorUI::SelectOptionWidget m_ImageWidgetImage{};
		EditorUI::CheckboxWidget m_ImageWidgetFixedAspectRatio{};

		// Edit Image Button Widget Options
		EditorUI::CollapsingHeaderWidget m_ImageButtonWidgetHeader{};
		EditorUI::SelectOptionWidget m_ImageButtonWidgetImage{};
		EditorUI::CheckboxWidget m_ImageButtonWidgetFixedAspectRatio{};
		EditorUI::EditVec4Widget m_ImageButtonWidgetBackgroundColor{};
		EditorUI::SelectOptionWidget m_ImageButtonWidgetOnPress{};
		EditorUI::CheckboxWidget m_ImageButtonWidgetSelectable{};

		// Edit Checkbox Widget Options
		EditorUI::CollapsingHeaderWidget m_CheckboxWidgetHeader{};
		EditorUI::CheckboxWidget m_CheckboxWidgetChecked{};
		EditorUI::SelectOptionWidget m_CheckboxWidgetCheckedImage{};
		EditorUI::SelectOptionWidget m_CheckboxWidgetUnCheckedImage{};
		EditorUI::CheckboxWidget m_CheckboxWidgetFixedAspectRatio{};
		EditorUI::EditVec4Widget m_CheckboxWidgetBackgroundColor{};
		EditorUI::SelectOptionWidget m_CheckboxWidgetOnPress{};
		EditorUI::CheckboxWidget m_CheckboxWidgetSelectable{};

		// Edit InputText Widget Options
		EditorUI::CollapsingHeaderWidget m_InputTextWidgetHeader{};
		EditorUI::EditTextSpec m_InputTextWidgetText{};
		EditorUI::EditFloatWidget m_InputTextWidgetTextSize{};
		EditorUI::EditVec4Widget m_InputTextWidgetTextColor{};
		EditorUI::SelectOptionWidget m_InputTextWidgetTextAlignment{};
		EditorUI::EditVec4Widget m_InputTextWidgetBackgroundColor{};
		EditorUI::SelectOptionWidget m_InputTextWidgetOnPress{};
		EditorUI::CheckboxWidget m_InputTextWidgetSelectable{};
		EditorUI::SelectOptionWidget m_InputTextWidgetOnMoveCursor{};

		// Edit Slider Widget Options
		EditorUI::CollapsingHeaderWidget m_SliderWidgetHeader{};
		EditorUI::EditVec4Widget m_SliderWidgetBackgroundColor{};
		EditorUI::SelectOptionWidget m_SliderWidgetOnPress{};
		EditorUI::CheckboxWidget m_SliderWidgetSelectable{};
		EditorUI::EditVec2Widget m_SliderWidgetBounds{};
		EditorUI::EditVec4Widget m_SliderWidgetSliderColor{};
		EditorUI::EditVec4Widget m_SliderWidgetLineColor{};
		EditorUI::SelectOptionWidget m_SliderWidgetOnMoveSlider{};

		// Edit DropDown Widget Options
		EditorUI::CollapsingHeaderWidget m_DropDownWidgetHeader{};
		EditorUI::EditVec4Widget m_DropDownWidgetBackgroundColor{};
		EditorUI::EditFloatWidget m_DropDownWidgetTextSize{};
		EditorUI::EditVec4Widget m_DropDownWidgetTextColor{};
		EditorUI::SelectOptionWidget m_DropDownWidgetTextAlignment{};
		EditorUI::SelectOptionWidget m_DropDownWidgetOnPress{};
		EditorUI::CheckboxWidget m_DropDownWidgetSelectable{};
		EditorUI::EditVec4Widget m_DropDownWidgetOptionBackgroundColor{};
		EditorUI::SelectOptionWidget m_DropDownWidgetOnSelectOption{};
		EditorUI::ListWidget m_DropDownWidgetOptionsList{};
		EditorUI::EditTextSpec m_DropDownWidgetOptionsListAddEntry{};
		EditorUI::GenericPopupWidget m_DropDownWidgetDeleteEntryWarning{};
		EditorUI::EditTextSpec m_DropDownWidgetEditEntry{};
		size_t m_ActiveDropDownOption{ 0 };
		EditorUI::CheckboxWidget m_DropDownWidgetDropdownOpen{};
		EditorUI::SelectOptionWidget m_DropDownWidgetCurrentOption{};

		// Edit Container Widget Options
		EditorUI::CollapsingHeaderWidget m_ContainerWidgetHeader{};
		EditorUI::EditVec4Widget m_ContainerWidgetBackground{};

		// Edit Vertical Container Widget Options
		EditorUI::CollapsingHeaderWidget m_VerticalContainerWidgetHeader{};
		EditorUI::EditVec4Widget m_VerticalContainerWidgetBackground{};
		EditorUI::EditFloatWidget m_VerticalContainerRowHeight{};
		EditorUI::EditFloatWidget m_VerticalContainerRowSpacing{};

		// Edit Horizontal Container Widget Options
		EditorUI::CollapsingHeaderWidget m_HorizontalContainerWidgetHeader{};
		EditorUI::EditVec4Widget m_HorizontalContainerWidgetBackground{};
		EditorUI::EditFloatWidget m_HorizontalContainerColumnWidth{};
		EditorUI::EditFloatWidget m_HorizontalContainerColumnSpacing{};
	};
}
