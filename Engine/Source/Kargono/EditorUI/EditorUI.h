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
#include <unordered_set>

namespace Kargono::Rendering { class Texture2D; }

namespace Kargono::EditorUI
{
	struct ChooseDirectorySpec;
	//==============================
	// Widget Forward Declarations
	//==============================
	struct RadioSelectorSpec;
	struct CollapsingHeaderSpec;
	struct EditTextSpec;
	struct PanelHeaderSpec;
	struct CheckboxSpec;
	struct GenericPopupSpec;
	struct SelectOptionSpec;
	struct EditVariableSpec;
	struct TableSpec;
	struct TreeSpec;
	struct InlineButtonSpec;
	struct EditFloatSpec;
	struct EditVec2Spec;
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
		float XPosition{ 0.0f };
		float YPosition{ 0.0f };
		float IconSize{ 0.0f };
		Ref<Rendering::Texture2D> ActiveIcon{ nullptr };
		Ref<Rendering::Texture2D> InactiveIcon{ nullptr };
		std::string ActiveTooltip{};
		std::string InactiveTooltip{};
		PositionType XPositionType{ PositionType::Inline };
		bool Disabled{ false };
	};

	//==============================
	// Widget Count Management
	//==============================
	inline uint32_t widgetCounter{ 1 };
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

		static void EditFloat(EditFloatSpec& spec);
		static void EditVec2(EditVec2Spec& spec);
		static void EditVec3(EditVec3Spec& spec);

		static void RadioSelector(RadioSelectorSpec& spec);
		static void Table(TableSpec& spec);
		static void Tree(TreeSpec& spec);
		static void PanelHeader(PanelHeaderSpec& spec);
		static void CollapsingHeader(CollapsingHeaderSpec& spec);
		static void LabeledText(const std::string& Label, const std::string& Text);
		static void Text(const std::string& Text);
		static void EditText(EditTextSpec& spec);
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
		static void BringWindowToFront(const std::string& windowName);
		static void ClearWindowFocus();
		static bool IsCurrentWindowVisible();
		static void HighlightFocusedWindow();
		static void SetDisableLeftClick(bool option);
		static void BlockMouseEvents(bool block);
		static void SetColorDefaults();
		static void SetButtonDefaults();
		
	public:
		//==============================
		// UI Fonts
		//==============================
		static ImFont* s_FontAntaLarge;
		static ImFont* s_FontAntaRegular;
		static ImFont* s_FontPlexBold;
		static ImFont* s_FontRobotoMono;

	public:
		//==============================
		// UI Images/Textures
		//==============================
		static Ref<Rendering::Texture2D> s_IconCamera,
			s_IconSettings, s_IconDelete, s_IconEdit, s_IconCancel, s_IconCancel2,
			s_IconConfirm, s_IconSearch,
			s_IconCheckbox_Disabled, s_IconCheckbox_Enabled,
			s_IconOptions, s_IconDown, s_IconRight, s_IconDash;

		static Ref<Rendering::Texture2D> s_IconBoxCollider, s_IconCircleCollider, s_IconEntity,
			s_IconClassInstance, s_IconRigidBody, s_IconTag, s_IconTransform;

		static Ref<Rendering::Texture2D> s_IconDisplay, s_IconSimulate, s_IconStep, s_IconPlay,
			s_IconPause, s_IconStop, s_IconGrid;

		static Ref<Rendering::Texture2D> s_IconDirectory, s_IconGenericFile,
			s_IconBack, s_IconForward,
			s_IconAudio, s_IconImage, s_IconBinary,
			s_IconScene, s_IconRegistry, s_IconScriptProject,
			s_IconUserInterface, s_IconFont, s_IconInput;

	public:
		//==============================
		// UI Colors
		//==============================
		inline static ImVec4 s_PureWhite{ 1.0f, 1.0f, 1.0f, 1.0f };
		inline static ImVec4 s_PureEmpty{ 0.0f, 0.0f, 0.0f, 0.0f };
		inline static ImVec4 s_PureBlack{ 0.0f, 0.0f, 0.0f, 1.0f };
		inline static ImVec4 s_LightGray_Thin{ 0.5f, 0.5f, 0.5f, 1.0f };
		
		// Editor Colors
		inline static ImVec4 s_PrimaryTextColor{ 242.0f / 255.0f, 236.5f / 255.0f, 221.1f / 255.0f, 1.0f };
		inline static ImVec4 s_SecondaryTextColor{ 220.0f / 255.0f, 215.0f / 255.0f, 201.0f / 255.0f, 1.0f };
		inline static ImVec4 s_HoveredColor{ 91.4f / 255.0f, 113.3f / 255.0f, 114.7f / 255.0f, 1.0f };
		inline static ImVec4 s_ActiveColor{ 76.2f / 255.0f, 94.4f / 255.0f, 95.6f / 255.0f, 1.0f };
		inline static ImVec4 s_SelectedColor{ 162.0f / 255.0f, 123.0f / 255.0f, 92.0f / 255.0f, 1.0f };
		inline static ImVec4 s_DisabledColor{ s_LightGray_Thin };

		inline static ImVec4 s_BackgroundColor { 44.0f / 255.0f, 54.0f / 255.0f, 57.0f / 255.0f, 1.0f  };
		inline static ImVec4 s_DarkBackgroundColor { 35.2f / 255.0f, 43.2f / 255.0f, 45.6f / 255.0f, 1.0f  };
		inline static ImVec4 s_AccentColor { 63.0f / 255.0f, 78.0f / 255.0f, 79.0f / 255.0f, 1.0f  };
		inline static ImVec4 s_DarkAccentColor { 50.4f / 255.0f, 62.4f / 255.0f, 63.2f / 255.0f, 1.0f  };

		inline static ImVec4 s_HighlightColor1{ 247.6f / 255.0f, 188.2f / 255.0f, 140.7f / 255.0f, 1.0f };
		inline static ImVec4 s_HighlightColor1_Thin { s_HighlightColor1.x, s_HighlightColor1.y, s_HighlightColor1.z, s_HighlightColor1.w * 0.75f };
		inline static ImVec4 s_HighlightColor2{ 147.0f / 255.0f, 247.0f / 255.0f, 141.4f / 255.0f, 1.0f };
		inline static ImVec4 s_HighlightColor3{ 241.0f / 255.0f, 141.0f / 255.0f, 247.4f / 255.0f, 1.0f };

		inline static ImVec4 s_GridMajor{ 0.735f, 0.720f, 0.690f, 1.0f };
		inline static ImVec4 s_GridMinor{ 0.347f, 0.347f, 0.347f, 1.0f };
	public:
		//==============================
		// UI Button Presets
		//==============================
		inline static InlineButtonSpec s_SmallEditButton;
		inline static InlineButtonSpec s_SmallExpandButton;
		inline static InlineButtonSpec s_MediumOptionsButton;
		inline static InlineButtonSpec s_SmallCheckboxButton;
		inline static InlineButtonSpec s_SmallCheckboxDisabledButton;
		inline static InlineButtonSpec s_SmallLinkButton;
		inline static InlineButtonSpec s_LargeDeleteButton;
		inline static InlineButtonSpec s_LargeCancelButton;
		inline static InlineButtonSpec s_LargeConfirmButton;
		inline static InlineButtonSpec s_LargeSearchButton;

	public:
		//==============================
		// Spacing
		//==============================
		inline static float s_SmallButtonRightOffset = 24.0f;
		inline static float s_SmallButtonSpacing = 22.0f;
		inline static float s_MediumButtonRightOffset = 28.0f;

		inline static float s_TextLeftIndentOffset = 30.5f;
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
		std::function<void()> PopupContents{ nullptr };
		std::function<void()> ConfirmAction{ nullptr };
		std::function<void()> DeleteAction{ nullptr };
		std::function<void()> PopupAction{ nullptr };
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
		Checkbox_Indented = BIT(1)
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
		bool Editing{ false };
		WidgetID WidgetID;
	private:
		friend void EditorUIService::Checkbox(CheckboxSpec& spec);
	};

	enum EditFloatFlags
	{
		EditFloat_None = 0,
		EditFloat_Indented = BIT(0)
	};

	struct EditFloatSpec
	{
	public:
		EditFloatSpec()
		{
			WidgetID = IncrementWidgetCounter();
		}
	public:
		std::string Label{};
		WidgetFlags Flags{ EditFloat_None };
		float CurrentFloat{};
		std::function<void()> ConfirmAction{ nullptr };
	private:
		bool Editing{ false };
		WidgetID WidgetID;
	private:
		friend void EditorUIService::EditFloat(EditFloatSpec& spec);
	};

	enum EditVec2Flags
	{
		EditVec2_None = 0,
		EditVec2_Indented = BIT(0)
	};

	struct EditVec2Spec
	{
	public:
		EditVec2Spec()
		{
			WidgetID = IncrementWidgetCounter();
		}
	public:
		std::string Label{};
		WidgetFlags Flags{ EditVec2_None };
		Math::vec2 CurrentVec2{};
		std::function<void()> ConfirmAction{ nullptr };
	private:
		bool Editing{ false };
		WidgetID WidgetID;
	private:
		friend void EditorUIService::EditVec2(EditVec2Spec& spec);
	};

	enum EditVec3Flags
	{
		EditVec3_None = 0,
		EditVec3_Indented = BIT(0)
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
		Math::vec3 CurrentVec3{};
		std::function<void()> ConfirmAction{ nullptr };
	private:
		bool Editing{ false };
		WidgetID WidgetID;
	private:
		friend void EditorUIService::EditVec3(EditVec3Spec& spec);
	};

	enum RadioSelectorFlags
	{
		RadioSelector_None = 0,
		RadioSelector_Indented = BIT(0)
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
		WidgetFlags Flags{ RadioSelector_None };
		uint16_t SelectedOption{ 0 };
		std::string FirstOptionLabel{ "None" };
		std::string SecondOptionLabel{ "None" };
		bool Editing{ false };
		std::function<void()> SelectAction{ nullptr };
	private:
		WidgetID WidgetID;
	private:
		friend void EditorUIService::RadioSelector(RadioSelectorSpec& spec);
	};

	enum EditTextFlags
	{
		EditText_None = 0,
		EditText_PopupOnly = BIT(0), // Only use a popup and remove inline text
		EditText_Indented = BIT(1) // Display indented
	};

	struct EditTextSpec
	{
	public:
		EditTextSpec()
		{
			WidgetID = IncrementWidgetCounter();
		}
	public:
		std::string Label;
		WidgetFlags Flags{ EditText_None };
		std::string CurrentOption{};
		std::function<void()> ConfirmAction;
		bool StartPopup{ false };
	private:
		WidgetID WidgetID;
	private:
		friend void EditorUIService::EditText(EditTextSpec& spec);
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
		WidgetFlags Flags{ EditText_None };
		std::filesystem::path CurrentOption{};
		std::function<void(const std::string&)> ConfirmAction{ nullptr };
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
		std::function<void()> OnExpand{ nullptr };
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

	class TreePath
	{
	public:
		void AddNode(uint16_t newNode)
		{
			m_Path.push_back(newNode);
		}

		void PopNode()
		{
			m_Path.pop_back();
		}

		std::size_t GetDepth()
		{
			return m_Path.size();
		}

		const std::vector<uint16_t>& GetPath() const
		{
			return m_Path;
		}

		bool operator==(const TreePath& other) const
		{
			return m_Path == other.m_Path;
		}

		operator bool() const
		{
			return (bool)m_Path.size();
		}
	private:
		std::vector<uint16_t> m_Path{};
	};
}

namespace std
{
	template<>
	struct hash<Kargono::EditorUI::TreePath>
	{
		std::size_t operator()(const Kargono::EditorUI::TreePath& path) const
		{
			std::size_t seed = 0;
			for (uint16_t node : path.GetPath()) 
			{
				seed ^= std::hash<uint16_t>()(node) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			}
			return seed;
		}
	};
}

namespace Kargono::EditorUI 
{
	struct TreeEntry;

	struct SelectionEntry
	{
		std::string Label{};
		std::function<void(TreeEntry&)> OnClick { nullptr };
	};

	struct TreeEntry
	{
		std::string Label {};
		UUID Handle {};
		Ref<Rendering::Texture2D> IconHandle{ nullptr };
		std::function<void(TreeEntry& entry)> OnLeftClick { nullptr };
		std::function<void(TreeEntry& entry)> OnDoubleLeftClick { nullptr };
		void* ProvidedData { nullptr };
		std::vector<TreeEntry> SubEntries{};
		std::vector<SelectionEntry> OnRightClickSelection {};
	};

	struct TreeSpec
	{
	public:
		TreeSpec()
		{
			WidgetID = IncrementWidgetCounter();
		}
	public:
		std::string Label;
		TreePath SelectedEntry{};
		std::function<void()> OnRefresh { nullptr };
	public:
		void InsertEntry(const TreeEntry& entry)
		{
			TreeEntries.push_back(entry);
		}
		void ClearTree()
		{
			TreeEntries.clear();
		}
		std::vector<TreeEntry>& GetTreeEntries()
		{
			return TreeEntries;
		}

		TreePath GetPathFromEntryReference(TreeEntry* entryQuery);
	private:
		WidgetID WidgetID;
		std::vector<TreeEntry> TreeEntries{};
		std::unordered_set<TreePath> ExpandedNodes{};
		TreeEntry* CurrentRightClick{ nullptr };
	private:
		friend void EditorUIService::Tree(TreeSpec& spec);
		friend void DrawEntries(TreeSpec& spec, std::vector<TreeEntry>& entries, uint32_t& widgetCount, TreePath& currentPath , ImVec2 rootPosition);
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
