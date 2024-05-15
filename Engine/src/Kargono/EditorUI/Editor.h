#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Events/ApplicationEvent.h"

#include "imgui.h"
#include "ImGuizmo.h"

#include <functional>
#include <vector>
#include <unordered_map>

#include "Kargono/Assets/Asset.h"
#include "Kargono/Core/Buffer.h"
#include "Kargono/Core/WrappedData.h"

namespace Kargono { class Texture2D; }

namespace Kargono::EditorUI
{
	struct RadioSelectorSpec;
	struct CollapsingHeaderSpec;
	struct TextInputSpec;
	struct SelectorHeaderSpec;
	struct CheckboxSpec;
	struct GenericPopupSpec;
	struct SelectOptionSpec;
	struct EditVariableSpec;
	struct TableSpec;

	using WidgetID = uint32_t;
	using SelectionList = std::unordered_map<std::string, std::function<void()>>;

	// Maintain unique id for each widget
	static uint32_t widgetCounter { 1 };
	static WidgetID IncrementWidgetCounter()
	{
		widgetCounter = widgetCounter + 1;
		return widgetCounter * 0x400'000; // 2 to the power of 22
	}

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
		float YPosition {0.0f};
		float IconSize{ 0.0f };
		Ref<Texture2D> ActiveIcon { nullptr };
		Ref<Texture2D> InactiveIcon { nullptr };
		std::string ActiveTooltip {};
		std::string InactiveTooltip{};
		PositionType XPositionType{ PositionType::Inline };
	};

	class Editor
	{
	public:
		// Lifetime Functions
		static void Init();
		static void Terminate();

		static void StartRendering();
		static void EndRendering();

		static void StartWindow(const std::string& label, bool* closeWindow = nullptr, int32_t flags = 0);
		static void EndWindow();

		static void OnEvent(Events::Event& e);

	public:
		// Modify Visuals
		static void Spacing(float space);
		static void Spacing(SpacingAmount space);
		static void Separator();

		// Use Widget Functions
		static void TitleText(const std::string& text);
		static void GenericPopup(GenericPopupSpec& spec);
		static void SelectOption(SelectOptionSpec& spec);
		static void EditVariable(EditVariableSpec& spec);
		static void NewItemScreen(const std::string& label1, std::function<void()> func1, const std::string& label2, std::function<void()> func2);
		static void Checkbox(CheckboxSpec& spec);
		static void RadioSelector(RadioSelectorSpec& spec);
		static void Table(TableSpec& spec);
		static void SelectorHeader(SelectorHeaderSpec& spec);
		static void CollapsingHeader(CollapsingHeaderSpec& spec);
		static void LabeledText(const std::string& Label, const std::string& Text);
		static void Text(const std::string& Text);
		static void TextInputPopup(TextInputSpec& spec);

	public:
		// Getter/Setter Functions
		static uint32_t GetActiveWidgetID();

		static void BlockEvents(bool block)
		{
			s_BlockEvents = block;
		}
	public:
		// Additional Resources
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

		static Ref<Texture2D> s_IconPlay, s_IconPause, s_IconStop,
			s_IconStep, s_IconSimulate, s_IconAddItem,
			s_IconSettings, s_IconDelete, s_IconDeleteActive, s_IconEdit, s_IconEdit_Active, s_IconCancel,
			s_IconConfirm, s_IconSearch, s_IconCheckbox_Empty_Disabled,
			s_IconCheckbox_Check_Disabled, s_IconCheckbox_Empty_Enabled,
			s_IconCheckbox_Check_Enabled, s_IconOptions, s_IconDown, s_IconRight, s_IconDash;

		static Ref<Texture2D> s_DirectoryIcon, s_GenericFileIcon,
			s_BackIcon, s_BackInactiveIcon, s_ForwardIcon, s_ForwardInactiveIcon,
			s_AudioIcon, s_ImageIcon, s_BinaryIcon,
			s_SceneIcon, s_RegistryIcon, s_ScriptProjectIcon,
			s_UserInterfaceIcon, s_FontIcon, s_InputIcon;

		inline static ImVec4 s_PureWhite {1.0f, 1.0f, 1.0f, 1.0f};
		inline static ImVec4 s_PearlBlue {38.0f / 255.0f, 212.0f / 255.0f, 212.0f / 255.0f, 1.0f};
		inline static ImVec4 s_DarkPurple {0.27843f, 0.011764f, 0.4f, 1.0f};
		inline static ImVec4 s_LightPurple_Thin { 182.0f / 255.0f, 103.0f / 255.0f, 219.0f / 255.0f, 0.35f };

		inline static InlineButtonSpec s_SmallEditButton;
		inline static InlineButtonSpec s_SmallExpandButton;
		inline static InlineButtonSpec s_SmallOptionsButton;
		inline static InlineButtonSpec s_LargeDeleteButton;
		inline static InlineButtonSpec s_LargeCancelButton;
		inline static InlineButtonSpec s_LargeConfirmButton;
		inline static InlineButtonSpec s_LargeSearchButton;
	private:
		// Internal Data
		inline static bool s_BlockEvents = true;
		inline static bool s_Running = false;
	};

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
		std::function<void(GenericPopupSpec&)> PopupAction {nullptr};
		bool PopupActive{ false };
	private:
		WidgetID WidgetID;
	private:
		friend void Editor::GenericPopup(GenericPopupSpec& spec);
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
		bool LeftLean{ true };
		bool ToggleBoolean{ false };
		std::function<void(bool)> ConfirmAction;
	private:
		bool Editing {false};
		WidgetID WidgetID;
	private:
		friend void Editor::Checkbox(CheckboxSpec& spec);
	};

	struct RadioSelectorSpec
	{
	public:
		RadioSelectorSpec()
		{
			WidgetID = IncrementWidgetCounter();
		}
	public:
		std::string Label;
		uint16_t SelectedOption{ 0 };
		std::string FirstOptionLabel {"None"};
		std::string SecondOptionLabel {"None"};
		bool Editing{ false };
		std::function<void(uint16_t selectedOption)> SelectAction {nullptr};
	private:
		WidgetID WidgetID;
	private:
		friend void Editor::RadioSelector(RadioSelectorSpec& spec);
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
		std::string CurrentOption;
		std::function<void(const std::string&)> ConfirmAction;
		bool PopupOnly{ false };
		bool StartPopup{ false };
	private:
		WidgetID WidgetID;
	private:
		friend void Editor::TextInputPopup(TextInputSpec& spec);
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
		bool Expanded{ false };
		std::function<void()> OnExpand { nullptr };
	private:
		WidgetID WidgetID;
	private:
		friend void Editor::CollapsingHeader(CollapsingHeaderSpec& spec);
	};

	struct SelectorHeaderSpec
	{
	public:
		SelectorHeaderSpec()
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
		friend void Editor::SelectorHeader(SelectorHeaderSpec& spec);
	};

	struct TableEntry
	{
		std::string Label;
		std::string Value;
		UUID Handle;
		std::function<void(TableEntry& entry)> OnEdit { nullptr };
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
		std::string Column1Title {};
		std::string Column2Title {};
		bool Expanded{ false };
		std::function<void()> OnRefresh { nullptr };
	public:
		void InsertTableEntry(const std::string& label, const std::string& value, 
			std::function<void(TableEntry& entry)> func, Assets::AssetHandle handle = 0)
		{
			TableEntry newEntry{label, value, handle, func };
			TableValues.push_back(newEntry);
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
		friend void Editor::Table(TableSpec& spec);
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
		std::function<void(SelectOptionSpec&)> PopupAction {nullptr};
		// Only used if PopupOnly is true
		bool StartPopup{ false };
		// Determines if line of text is generated for options
		bool PopupOnly{ false };
		bool Indented{ false };
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
		friend void Editor::SelectOption(SelectOptionSpec&);
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
		void AllocateBuffer()
		{
			FieldBuffer.Allocate(400);
			FieldBuffer.SetDataToByte(0);
		}
		Buffer FieldBuffer {};
	private:
		WidgetID WidgetID;
	private:
		friend void Editor::EditVariable(EditVariableSpec&);
	};
}
