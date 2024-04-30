#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Events/ApplicationEvent.h"

#include "imgui.h"
#include "ImGuizmo.h"

#include <functional>
#include <vector>
#include <unordered_map>

#include "Kargono/Core/Buffer.h"
#include "Kargono/Core/WrappedData.h"


namespace Kargono { class Texture2D; }

namespace Kargono::UI
{
	struct SelectOptionSpec;
	struct EditVariableSpec;
	struct TableSpec;

	struct ScopedStyleColor
	{
		ScopedStyleColor() = default;

		ScopedStyleColor(ImGuiCol idx, ImVec4 color, bool predicate = true)
			:m_Set(predicate)
		{
			if (predicate)
			{
				ImGui::PushStyleColor(idx, color);
			}
		}

		ScopedStyleColor(ImGuiCol idx, ImU32 color, bool predicate = true)
			:m_Set(predicate)
		{
			if (predicate)
			{
				ImGui::PushStyleColor(idx, color);
			}
		}

		~ScopedStyleColor()
		{
			if (m_Set)
			{
				ImGui::PopStyleColor();
			}

		}
	private:
		bool m_Set = false;
	};

	using OptionList = std::unordered_map<std::string, std::vector<std::string>>;
	using SelectionList = std::unordered_map<std::string, std::function<void()>>;

	struct GenericPopupSpec
	{
	public:
		UUID WidgetID;
		std::string Label;
		float PopupWidth { 700.0f };
		std::function<void()> PopupContents {nullptr};
		std::function<void()> ConfirmAction {nullptr};
		std::function<void()> DeleteAction {nullptr};
		std::function<void(GenericPopupSpec&)> PopupAction {nullptr};
		bool PopupActive {false};
		bool DeleteActive{ false };
	};

	struct CheckboxSpec
	{
	public:
		UUID WidgetID;
		std::string Label;
		bool LeftLean{ true };
		bool ToggleBoolean;
		std::function<void(bool)> ConfirmAction;
	};

	struct TextInputSpec
	{
	public:
		UUID WidgetID;
		std::string Label;
		std::string CurrentOption;
		std::function<void(const std::string&)> ConfirmAction;
	};

	struct SelectorHeaderSpec
	{
		UUID WidgetID;
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
	};

	enum class SpacingAmount
	{
		None = 0,
		Small,
		Medium,
		Large
	};

	class Editor
	{
	public:
		static void Init();
		static void Terminate();

		static void StartRendering();
		static void EndRendering();

		static void StartWindow(const std::string& label, bool* closeWindow = nullptr, int32_t flags = 0);
		static void EndWindow();

		static void OnEvent(Events::Event& e);

		static void Spacing(float space);
		static void TitleText(const std::string& text);
		static void Spacing(SpacingAmount space);
		static void Separator();

		static void GenericPopup(GenericPopupSpec& spec);
		static void SelectOption(SelectOptionSpec& spec);
		static void EditVariable(EditVariableSpec& spec);
		static void NewItemScreen(const std::string& label1, std::function<void()> func1, const std::string& label2, std::function<void()> func2);
		static void Checkbox(CheckboxSpec& spec);
		static void Table(TableSpec& spec);
		static void SelectorHeader(SelectorHeaderSpec& spec);
		static void LabeledText(const std::string& Label, const std::string& Text);
		static void Text(const std::string& Text);
		static void TextInputModal(TextInputSpec& spec);

		static uint32_t GetActiveWidgetID();

		static void BlockEvents(bool block)
		{
			s_BlockEvents = block;
		}

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
	private:
		inline static bool s_BlockEvents = true;
		inline static bool s_Running = false;
	};

	struct TableEntry
	{
		std::string Label;
		std::string Value;
		std::function<void(TableEntry& entry)> OnEdit { nullptr };
	};

	struct TableSpec
	{
	public:
		UUID WidgetID;
		std::string Label;
		std::string Column1Title {};
		std::string Column2Title {};
		std::function<void()> OnRefresh { nullptr };
	public:
		void InsertTableEntry(const std::string& label, const std::string& value, std::function<void(TableEntry& entry)> func)
		{
			TableEntry newEntry{label, value, func};
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
		bool ShowTable { false };
		std::vector<TableEntry> TableValues{};
		SelectionList EditTableSelectionList{};
	private:
		friend void Editor::Table(TableSpec& spec);
	};

	struct SelectOptionSpec
	{
	public:
		UUID WidgetID;
		std::string Label;
		std::string CurrentOption;
		uint32_t LineCount{ 3 };
		std::function<void(const std::string&)> ConfirmAction {nullptr};
		std::function<void(SelectOptionSpec&)> PopupAction {nullptr};
		// Only used if PopupOnly is true
		bool StartPopup{ false };
		// Determines if line of text is generated for options
		bool PopupOnly{ false };
		void ClearOptionsList()
		{
			OptionsList.clear();
		}
		void AddToOptionsList(const std::string& title, const std::string& option)
		{
			if (!OptionsList.contains(title))
			{
				std::vector<std::string> newVector {};
				newVector.push_back(option);
				OptionsList.insert_or_assign(title, newVector);
				return;
			}

			OptionsList.at(title).push_back(option);
		}
		OptionList& GetOptionsList()
		{
			return OptionsList;
		}
	private:
		OptionList OptionsList{};
	private:
		friend void Editor::SelectOption(SelectOptionSpec&);
	};

	struct EditVariableSpec
	{
	public:
		UUID WidgetID;
		std::string Label;
		void AllocateBuffer()
		{
			FieldBuffer.Allocate(400);
			FieldBuffer.SetDataToByte(0);
		}
		Buffer FieldBuffer {};
	private:
		friend void Editor::EditVariable(EditVariableSpec&);
	};
}
