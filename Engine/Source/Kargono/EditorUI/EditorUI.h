#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Events/ApplicationEvent.h"
#include "Kargono/Assets/Asset.h"
#include "Kargono/Core/Buffer.h"
#include "Kargono/Core/WrappedData.h"

#include "API/EditorUI/ImGuiAPI.h"
#include "API/EditorUI/ImGuizmoAPI.h"

#include <functional>
#include <vector>
#include <unordered_map>

namespace Kargono::Rendering { class Texture2D; }

namespace Kargono::EditorUI
{
	struct ChooseDirectorySpec;
	//==============================
	// Widget Forward Declarations
	//==============================
	struct RadioSelectorSpec;
	struct CollapsingHeaderSpec;
	struct TextInputSpec;
	struct PanelHeaderSpec;
	struct CheckboxSpec;
	struct GenericPopupSpec;
	struct SelectOptionSpec;
	struct EditVariableSpec;
	struct TableSpec;
	struct InlineButtonSpec;
	struct EditVec3Spec;

	//==============================
	// Type Defines
	//==============================
	using WidgetID = uint32_t;
	using SelectionList = std::unordered_map<std::string, std::function<void()>>;
	using WidgetFlags = uint8_t;
	enum class SpacingAmount
	{
		None = 0,
		Small,
		Medium,
		Large
	};

	enum class PositionType
	{
		None = 0,
		Absolute,
		Relative,
		Inline
	};

	struct InlineButtonSpec
	{
	public:
		float XPosition {0.0f};
		float YPosition{ 0.0f };
		float IconSize{ 0.0f };
		Ref<Rendering::Texture2D> ActiveIcon { nullptr };
		Ref<Rendering::Texture2D> InactiveIcon { nullptr };
		std::string ActiveTooltip {};
		std::string InactiveTooltip{};
		PositionType XPositionType{ PositionType::Inline };
		bool Disabled{ false };
	};

	//==============================
	// Widget Count Management
	//==============================
	inline uint32_t widgetCounter {1};
	// Maintain unique id for each widget
	static WidgetID IncrementWidgetCounter()
	{
		widgetCounter++;
		return widgetCounter * 0x400'000; // 2 to the power of 22
	}

	//==============================
	// EditorUI Service Class
	//==============================
	class EditorUIService
	{
	public:
		//==============================
		// LifeCycle Functions
		//==============================
		static void Init();
		static void Terminate();

		static void StartRendering();
		static void EndRendering();

		static void StartWindow(const std::string& label, bool* closeWindow = nullptr, int32_t flags = 0);
		static void EndWindow();
	public:
		//==============================
		// Event Functions
		//==============================
		static void OnEvent(Events::Event& e);

	public:
		//==============================
		// Modify Visuals Functions
		//==============================
		static void Spacing(float space);
		static void Spacing(SpacingAmount space);

		//==============================
		// Create/Display Widget Functions
		//==============================
		static void TitleText(const std::string& text);
		static void GenericPopup(GenericPopupSpec& spec);
		static void SelectOption(SelectOptionSpec& spec);
		static void EditVariable(EditVariableSpec& spec);
		static void NewItemScreen(const std::string& label1, std::function<void()> func1, const std::string& label2, std::function<void()> func2);
		static void Checkbox(CheckboxSpec& spec);

		static void EditVec3(EditVec3Spec& spec);

		static void RadioSelector(RadioSelectorSpec& spec);
		static void Table(TableSpec& spec);
		static void PanelHeader(PanelHeaderSpec& spec);
		static void CollapsingHeader(CollapsingHeaderSpec& spec);
		static void LabeledText(const std::string& Label, const std::string& Text);
		static void Text(const std::string& Text);
		static void TextInputPopup(TextInputSpec& spec);
		static void ChooseDirectory(ChooseDirectorySpec& spec);
		static void BeginTabBar(const std::string& title);
		static void EndTabBar();
		static bool BeginTabItem(const std::string& title);
		static void EndTabItem();

	public:
		//==============================
		// Interact With UI State Functions
		//==============================
		static uint32_t GetActiveWidgetID();
		static std::string GetFocusedWindowName();
		static void SetFocusedWindow(const std::string& windowName);
		static void ClearWindowFocus();
		static void HighlightFocusedWindow();
		static void SetDisableLeftClick(bool option);
		static void BlockMouseEvents(bool block);
	public:
		//==============================
		// UI Fonts
		//==============================
		static ImFont* s_AntaLarge;
		static ImFont* s_AntaRegular;
		static ImFont* s_AntaSmall;
		static ImFont* s_PlexBold;
		static ImFont* s_PlexRegular;
		static ImFont* s_OpenSansRegular;
		static ImFont* s_OpenSansBold;
		static ImFont* s_RobotoRegular;
		static ImFont* s_RobotoMono;
		static ImFont* s_AnonymousRegular;

	public:
		//==============================
		// UI Images/Textures
		//==============================
		static Ref<Rendering::Texture2D> s_IconPlay, s_IconPause, s_IconStop,
			s_IconStep, s_IconSimulate, s_IconAddItem, s_IconDisplay, s_IconDisplayActive,
			s_IconCamera, s_IconCameraActive,
			s_IconPlayActive, s_IconStopActive, s_IconPauseActive, s_IconStepActive, s_IconSimulateActive,
			s_IconSettings, s_IconDelete, s_IconDeleteActive, s_IconEdit, s_IconEdit_Active, s_IconCancel,
			s_IconConfirm, s_IconSearch, s_IconCheckbox_Empty_Disabled,
			s_IconCheckbox_Check_Disabled, s_IconCheckbox_Empty_Enabled,
			s_IconCheckbox_Check_Enabled, s_IconOptions, s_IconDown, s_IconRight, s_IconDash;

		static Ref<Rendering::Texture2D> s_DirectoryIcon, s_GenericFileIcon,
			s_BackIcon, s_BackInactiveIcon, s_ForwardIcon, s_ForwardInactiveIcon,
			s_AudioIcon, s_ImageIcon, s_BinaryIcon,
			s_SceneIcon, s_RegistryIcon, s_ScriptProjectIcon,
			s_UserInterfaceIcon, s_FontIcon, s_InputIcon;

	public:
		//==============================
		// UI Colors
		//==============================
		inline static ImVec4 s_PureWhite {1.0f, 1.0f, 1.0f, 1.0f};
		inline static ImVec4 s_PureBlack {0.0f, 0.0f, 0.0f, 1.0f};
		inline static ImVec4 s_LightGray {0.7f, 0.7f, 0.7f, 1.0f};
		inline static ImVec4 s_PureEmpty {0.0f, 0.0f, 0.0f, 0.0f};
		inline static ImVec4 s_PearlBlue {38.0f / 255.0f, 212.0f / 255.0f, 212.0f / 255.0f, 1.0f};
		inline static ImVec4 s_PearlBlue_Thin {38.0f / 255.0f, 212.0f / 255.0f, 212.0f / 255.0f, 0.75f};
		inline static ImVec4 s_DarkPurple {0.27843f, 0.011764f, 0.4f, 1.0f};
		inline static ImVec4 s_LightPurple {0.9226f, 0.4630f, 1.0f, 1.0f};
		inline static ImVec4 s_LightGreen {0.2879f, 1.0f, 0.39322f, 1.0f};
		inline static ImVec4 s_LightPurple_Thin { 182.0f / 255.0f, 103.0f / 255.0f, 219.0f / 255.0f, 0.35f };
	public:
		//==============================
		// UI Button Presets
		//==============================
		inline static InlineButtonSpec s_SmallEditButton;
		inline static InlineButtonSpec s_SmallExpandButton;
		inline static InlineButtonSpec s_SmallOptionsButton;
		inline static InlineButtonSpec s_SmallCheckboxButton;
		inline static InlineButtonSpec s_SmallLinkButton;
		inline static InlineButtonSpec s_LargeDeleteButton;
		inline static InlineButtonSpec s_LargeCancelButton;
		inline static InlineButtonSpec s_LargeConfirmButton;
		inline static InlineButtonSpec s_LargeSearchButton;
	private:
		//==============================
		// Internal Fields
		//==============================
		inline static bool s_BlockMouseEvents = true;
		inline static bool s_Running = false;
		inline static bool s_DisableLeftClick = false;
	};

	//==============================
	// Widget Specifications
	//==============================
	struct GenericPopupSpec
	{
	public:
		GenericPopupSpec()
		{
			WidgetID = IncrementWidgetCounter();
		}
	public:
		std::string Label;
		float PopupWidth{ 700.0f };
		std::function<void()> PopupContents {nullptr};
		std::function<void()> ConfirmAction {nullptr};
		std::function<void()> DeleteAction {nullptr};
		std::function<void()> PopupAction {nullptr};
		bool PopupActive{ false };
	private:
		WidgetID WidgetID;
	private:
		friend void EditorUIService::GenericPopup(GenericPopupSpec& spec);
	};

	enum CheckboxFlags
	{
		Checkbox_None = 0,
		Checkbox_LeftLean = BIT(0), // Check box aligns to the left
	};

	
	struct CheckboxSpec
	{
	public:
		CheckboxSpec()
		{
			WidgetID = IncrementWidgetCounter();
		}
	public:
		std::string Label;
		WidgetFlags Flags{ Checkbox_LeftLean };
		bool ToggleBoolean{ false };
		std::function<void(bool)> ConfirmAction;
	private:
		bool Editing {false};
		WidgetID WidgetID;
	private:
		friend void EditorUIService::Checkbox(CheckboxSpec& spec);
	};

	enum EditVec3Flags
	{
		EditVec3_None = 0,
		EditVec3_Indented = BIT(0), // Check box aligns to the left
	};

	struct EditVec3Spec
	{
	public:
		EditVec3Spec()
		{
			WidgetID = IncrementWidgetCounter();
		}
	public:
		std::string Label{};
		WidgetFlags Flags{ EditVec3_None };
		Math::vec3 CurrentVec3 {};
		std::function<void()> ConfirmAction { nullptr };
	private:
		bool Editing{ false };
		WidgetID WidgetID;
	private:
		friend void EditorUIService::EditVec3(EditVec3Spec& spec);
	};

	struct RadioSelectorSpec
	{
	public:
		RadioSelectorSpec()
		{
			WidgetID = IncrementWidgetCounter();
		}
	public:
		std::string Label ;
		uint16_t SelectedOption{ 0 };
		std::string FirstOptionLabel {"None"};
		std::string SecondOptionLabel {"None"};
		bool Editing{ false };
		std::function<void(uint16_t selectedOption)> SelectAction {nullptr};
	private:
		WidgetID WidgetID;
	private:
		friend void EditorUIService::RadioSelector(RadioSelectorSpec& spec);
	};

	enum TextInputFlags
	{
		TextInput_None = 0,
		TextInput_PopupOnly = BIT(0), // Only use a popup and remove inline text
	};

	struct TextInputSpec
	{
	public:
		TextInputSpec()
		{
			WidgetID = IncrementWidgetCounter();
		}
	public:
		std::string Label;
		WidgetFlags Flags{ TextInput_None };
		std::string CurrentOption {};
		std::function<void()> ConfirmAction;
		bool StartPopup{ false };
	private:
		WidgetID WidgetID;
	private:
		friend void EditorUIService::TextInputPopup(TextInputSpec& spec);
	};

	struct ChooseDirectorySpec
	{
	public:
		ChooseDirectorySpec()
		{
			WidgetID = IncrementWidgetCounter();
		}
	public:
		std::string Label;
		WidgetFlags Flags{ TextInput_None };
		std::filesystem::path CurrentOption {};
		std::function<void(const std::string&)> ConfirmAction { nullptr };
	private:
		WidgetID WidgetID;
	private:
		friend void EditorUIService::ChooseDirectory(ChooseDirectorySpec& spec);
	};

	enum CollapsingHeaderFlags
	{
		CollapsingHeader_None = 0,
		CollapsingHeader_UnderlineTitle = BIT(0), // Underlines the title text
	};

	struct CollapsingHeaderSpec
	{
	public:
		CollapsingHeaderSpec()
		{
			WidgetID = IncrementWidgetCounter();
		}
	public:
		std::string Label;
		WidgetFlags Flags{ CollapsingHeader_None };
		bool Expanded{ false };
		std::function<void()> OnExpand { nullptr };
	public:
		void ClearSelectionList()
		{
			SelectionList.clear();
		}
		void AddToSelectionList(const std::string& label, std::function<void()> function)
		{
			if (!SelectionList.contains(label))
			{
				SelectionList.insert_or_assign(label, function);
				return;
			}
		}
		SelectionList& GetSelectionList()
		{
			return SelectionList;
		}
	private:
		WidgetID WidgetID;
		SelectionList SelectionList{};
	private:
		friend void EditorUIService::CollapsingHeader(CollapsingHeaderSpec& spec);
	};

	struct PanelHeaderSpec
	{
	public:
		PanelHeaderSpec()
		{
			WidgetID = IncrementWidgetCounter();
		}
	public:
		std::string Label;
		bool EditColorActive{ false };
		void ClearSelectionList()
		{
			SelectionsList.clear();
		}
		void AddToSelectionList(const std::string& label, std::function<void()> function)
		{
			if (!SelectionsList.contains(label))
			{
				SelectionsList.insert_or_assign(label, function);
				return;
			}
		}
		SelectionList& GetSelectionList()
		{
			return SelectionsList;
		}

	private:
		SelectionList SelectionsList{};
		WidgetID WidgetID;
	private:
		friend void EditorUIService::PanelHeader(PanelHeaderSpec& spec);
	};

	enum TableFlags
	{
		Table_None = 0,
		Table_Indented = BIT(0), // Indents the table over once
		Table_UnderlineTitle = BIT(1) // Adds an underline to the title
	};

	struct TableEntry
	{
		std::string Label;
		std::string Value;
		UUID Handle;
		std::function<void(TableEntry& entry)> OnEdit { nullptr };
		std::function<void(TableEntry& entry)> OnLink { nullptr };
	};

	struct TableSpec
	{
	public:
		TableSpec()
		{
			WidgetID = IncrementWidgetCounter();
		}
	public:
		std::string Label;
		WidgetFlags Flags{ Table_None };
		std::string Column1Title {};
		std::string Column2Title {};
		bool Expanded{ false };
		std::function<void()> OnRefresh { nullptr };
	public:
		void InsertTableEntry(const std::string& label, const std::string& value, 
			std::function<void(TableEntry& entry)> onEdit, Assets::AssetHandle handle = 0)
		{
			TableEntry newEntry{label, value, handle, onEdit, nullptr };
			TableValues.push_back(newEntry);
		}

		void InsertTableEntry(const TableEntry& entry)
		{
			TableValues.push_back(entry);
		}
		void ClearTable()
		{
			TableValues.clear();
		}
		void ClearEditTableSelectionList()
		{
			EditTableSelectionList.clear();
		}
		void AddToSelectionList(const std::string& label, std::function<void()> function)
		{
			if (!EditTableSelectionList.contains(label))
			{
				EditTableSelectionList.insert_or_assign(label, function);
				return;
			}
		}
		SelectionList& GetEditTableSelectionList()
		{
			return EditTableSelectionList;
		}
	private:
		WidgetID WidgetID;
		std::vector<TableEntry> TableValues{};
		SelectionList EditTableSelectionList{};
	private:
		friend void EditorUIService::Table(TableSpec& spec);
	};

	struct OptionEntry
	{
	public:
		std::string Label{};
		Assets::AssetHandle Handle { Assets::EmptyHandle };
	public:
		bool operator==(const OptionEntry& other) const
		{
			if (this->Label == other.Label && this->Handle == other.Handle)
			{
				return true;
			}
			return false;
		}
	};

	enum SelectOptionFlags
	{
		SelectOption_None = 0,
		SelectOption_Indented = BIT(0), // Indents the text (used in collapsing headers usually)
		SelectOption_PopupOnly = BIT(1) // Determines if line of text is generated for options
	};

	using OptionList = std::unordered_map<std::string, std::vector<OptionEntry>>;

	struct SelectOptionSpec
	{
	public:
		SelectOptionSpec()
		{
			WidgetID = IncrementWidgetCounter();
		}
	public:
		std::string Label{};
		OptionEntry CurrentOption{};
		uint32_t LineCount{ 3 };
		std::function<void(const OptionEntry&)> ConfirmAction {nullptr};
		std::function<void()> PopupAction {nullptr};
		// Only used if PopupOnly is true
		bool PopupActive{ false };
		WidgetFlags Flags{ SelectOption_None };
		void ClearOptions()
		{
			ActiveOptions.clear();
		}
		void AddToOptions(const std::string& group, const std::string& optionLabel, UUID optionIdentifier)
		{
			const OptionEntry newEntry{ optionLabel, optionIdentifier };
			if (!ActiveOptions.contains(group))
			{
				std::vector<OptionEntry> newVector {};
				newVector.push_back(newEntry);
				ActiveOptions.insert_or_assign(group, newVector);
				return;
			}

			ActiveOptions.at(group).push_back(newEntry);
		}
		OptionList& GetAllOptions()
		{
			return ActiveOptions;
		}
	private:
		WidgetID WidgetID;
		OptionList ActiveOptions{};
		bool Searching { false };
		OptionEntry CachedSelection {};
		OptionList CachedSearchResults{};
	private:
		friend void EditorUIService::SelectOption(SelectOptionSpec&);
	};

	struct EditVariableSpec
	{
	public:
		EditVariableSpec()
		{
			WidgetID = IncrementWidgetCounter();
		}
	public:
		std::string Label;
		Buffer FieldBuffer {};
		WrappedVarType VariableType{ WrappedVarType::Integer32 };
	public:
		void AllocateBuffer()
		{
			FieldBuffer.Allocate(400);
			FieldBuffer.SetDataToByte(0);
		}
	private:
		WidgetID WidgetID;
	private:
		friend void EditorUIService::EditVariable(EditVariableSpec&);
	};
}

namespace Kargono::Utility
{
	//==============================
	// ImVec4 -> Math::vec4
	//==============================
	static Math::vec4 ImVec4ToMathVec4(const ImVec4& color )
	{
		return { color.x, color.y, color.z, color.w };
	}
}
