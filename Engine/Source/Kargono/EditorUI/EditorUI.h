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
#include <variant>


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
	struct NavigationHeaderSpec;
	struct CheckboxSpec;
	struct GenericPopupSpec;
	struct WarningPopupSpec;
	struct GridSpec;
	struct SelectOptionSpec;
	struct EditVariableSpec;
	struct TooltipSpec;
	struct ListSpec;
	struct TreeSpec;
	struct InlineButtonSpec;
	struct EditIntegerSpec;
	struct EditFloatSpec;
	struct EditVec2Spec;
	struct EditVec3Spec;
	struct EditVec4Spec;

	//==============================
	// Type Defines
	//==============================

	struct DragDropPayload
	{
		FixedString32 m_Label;
		void* m_DataPointer;
		std::size_t m_DataSize;
	};

	using WidgetID = uint32_t;
	constexpr inline WidgetID k_InvalidWidgetID{ std::numeric_limits<WidgetID>::max() };
	using WidgetFlags = uint8_t;
	using SelectionList = std::unordered_map<std::string, std::function<void()>>;
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

		static void StartWindow(const char* label, bool* closeWindow = nullptr, int32_t flags = 0);
		static void EndWindow();
	public:
		//==============================
		// Event Functions
		//==============================
		static bool OnInputEvent(Events::Event* e);

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
		static void WarningPopup(WarningPopupSpec& spec);
		static void SelectOption(SelectOptionSpec& spec);
		static void EditVariable(EditVariableSpec& spec);
		static void NewItemScreen(const std::string& label1, std::function<void()> func1, const std::string& label2, std::function<void()> func2);
		static void Checkbox(CheckboxSpec& spec);

		static void EditInteger(EditIntegerSpec& spec);
		static void EditFloat(EditFloatSpec& spec);
		static void EditVec2(EditVec2Spec& spec);
		static void EditVec3(EditVec3Spec& spec);
		static void EditVec4(EditVec4Spec& spec);

		static void RadioSelector(RadioSelectorSpec& spec);
		static void List(ListSpec& spec);
		static void Tree(TreeSpec& spec);
		static void PanelHeader(PanelHeaderSpec& spec);
		static void NavigationHeader(NavigationHeaderSpec& spec);
		static void Grid(GridSpec& spec);
		static void CollapsingHeader(CollapsingHeaderSpec& spec);
		static void LabeledText(const std::string& Label, const std::string& Text);
		static void Text(const char* text);
		static void EditText(EditTextSpec& spec);
		static void ChooseDirectory(ChooseDirectorySpec& spec);
		static void Tooltip(TooltipSpec& spec);
		static void BeginTabBar(const std::string& title);
		static void EndTabBar();
		static bool BeginTabItem(const std::string& title);
		static void EndTabItem();
		

	public:
		//==============================
		// Interact With UI State Functions
		//==============================
		static uint32_t GetActiveWidgetID();
		static const char* GetFocusedWindowName();
		static void SetFocusedWindow(const char* windowName);
		static void BringWindowToFront(const char* windowName);
		static void BringCurrentWindowToFront();
		static void ClearWindowFocus();
		static bool IsCurrentWindowVisible();
		static void HighlightFocusedWindow();
		static void SetDisableLeftClick(bool option);
		static bool IsAnyItemHovered();
		static void BlockMouseEvents(bool block);
		static void SetColorDefaults();
		static void SetButtonDefaults();
		static const char* GetHoveredWindowName();
		static void CreateWarningNotification(const char* text, int delayMS);
		static void CreateCriticalNotification(const char* text, int delayMS);

	private:
		static void RenderImGuiNotify();

	public:
		//==============================
		// UI Fonts
		//==============================
		inline static ImFont* s_FontAntaLarge{nullptr};
		inline static ImFont* s_FontAntaRegular {nullptr};
		inline static ImFont* s_FontPlexBold {nullptr};
		inline static ImFont* s_FontRobotoMono {nullptr};

	public:
		//==============================
		// UI Images/Textures
		//==============================

		// General Icons
		inline static Ref<Rendering::Texture2D> s_IconCamera,
			s_IconSettings, s_IconDelete, s_IconEdit, s_IconCancel, s_IconCancel2,
			s_IconConfirm, s_IconSearch,
			s_IconCheckbox_Disabled, s_IconCheckbox_Enabled, s_IconNotification,
			s_IconOptions, s_IconDown, s_IconRight, s_IconDash, s_IconAI;

		// Scene graph icons
		inline static Ref<Rendering::Texture2D> s_IconBoxCollider, s_IconCircleCollider, s_IconEntity,
			s_IconClassInstance, s_IconRigidBody, s_IconTag, s_IconTransform;

		// Viewport icons
		inline static Ref<Rendering::Texture2D> s_IconDisplay, s_IconSimulate, s_IconStep, s_IconPlay,
			s_IconPause, s_IconStop, s_IconGrid;

		// Runtime UI icons
		inline static Ref<Rendering::Texture2D> s_IconWindow, s_IconTextWidget;

		// Scripting icons
		inline static Ref<Rendering::Texture2D> s_IconNumber, s_IconVariable, s_IconFunction, s_IconBoolean, s_IconDecimal;

		// Content browser icons
		inline static Ref<Rendering::Texture2D> s_IconDirectory, s_IconGenericFile,
			s_IconBack, s_IconForward,
			s_IconAudio, s_IconTexture, s_IconBinary,
			s_IconScene, s_IconRegistry,
			s_IconUserInterface, s_IconFont, s_IconInput, s_IconAI_KG,
			s_IconAudio_KG, s_IconFont_KG, s_IconGameState, s_IconProjectComponent,
			s_IconScript, s_IconTexture_KG;

	public:
		//==============================
		// UI Colors
		//==============================
		inline static ImVec4 s_PureWhite{ 1.0f, 1.0f, 1.0f, 1.0f };
		inline static ImVec4 s_PureEmpty{ 0.0f, 0.0f, 0.0f, 0.0f };
		inline static ImVec4 s_PureBlack{ 0.0f, 0.0f, 0.0f, 1.0f };

		
		// Editor Colors
		inline static ImVec4 s_PrimaryTextColor{ 242.0f / 255.0f, 236.5f / 255.0f, 221.1f / 255.0f, 1.0f };
		inline static ImVec4 s_SecondaryTextColor{ 220.0f / 255.0f, 215.0f / 255.0f, 201.0f / 255.0f, 1.0f };
		inline static ImVec4 s_HoveredColor{ 91.4f / 255.0f, 113.3f / 255.0f, 114.7f / 255.0f, 1.0f };
		inline static ImVec4 s_ActiveColor{ 76.2f / 255.0f, 94.4f / 255.0f, 95.6f / 255.0f, 1.0f };
		inline static ImVec4 s_SelectedColor{ 162.0f / 255.0f, 123.0f / 255.0f, 92.0f / 255.0f, 1.0f };
		inline static ImVec4 s_DisabledColor{ 0.5f, 0.5f, 0.5f, 1.0f };

		inline static ImVec4 s_BackgroundColor { 44.0f / 255.0f, 54.0f / 255.0f, 57.0f / 255.0f, 1.0f  };
		inline static ImVec4 s_DarkBackgroundColor { 35.2f / 255.0f, 43.2f / 255.0f, 45.6f / 255.0f, 1.0f  };
		inline static ImVec4 s_AccentColor { 63.0f / 255.0f, 78.0f / 255.0f, 79.0f / 255.0f, 1.0f  };
		inline static ImVec4 s_DarkAccentColor { 50.4f / 255.0f, 62.4f / 255.0f, 63.2f / 255.0f, 1.0f  };

		inline static ImVec4 s_HighlightColor1{ 247.6f / 255.0f, 188.2f / 255.0f, 140.7f / 255.0f, 1.0f };
		inline static ImVec4 s_HighlightColor1_Thin { s_HighlightColor1.x, s_HighlightColor1.y, s_HighlightColor1.z, s_HighlightColor1.w * 0.75f };
		inline static ImVec4 s_HighlightColor2{ 147.0f / 255.0f, 247.0f / 255.0f, 141.4f / 255.0f, 1.0f };
		inline static ImVec4 s_HighlightColor2_Thin{ s_HighlightColor2.x, s_HighlightColor2.y, s_HighlightColor2.z, s_HighlightColor2.w * 0.75f };
		inline static ImVec4 s_HighlightColor3{ 241.0f / 255.0f, 141.0f / 255.0f, 247.4f / 255.0f, 1.0f };
		inline static ImVec4 s_HighlightColor3_Thin { s_HighlightColor3.x, s_HighlightColor3.y, s_HighlightColor3.z, s_HighlightColor3.w * 0.75f };
		inline static ImVec4 s_HighlightColor3_UltraThin { s_HighlightColor3.x, s_HighlightColor3.y, s_HighlightColor3.z, s_HighlightColor3.w * 0.3f };
		inline static ImVec4 s_HighlightColor4{ 141.0f / 255.0f, 200.0f / 255.0f, 247.0f / 255.0f, 1.0f };
		inline static ImVec4 s_HighlightColor4_Thin { s_HighlightColor4.x, s_HighlightColor4.y, s_HighlightColor4.z, s_HighlightColor4.w * 0.75f };
		inline static ImVec4 s_HighlightColor4_UltraThin { s_HighlightColor4.x, s_HighlightColor4.y, s_HighlightColor4.z, s_HighlightColor4.w * 0.3f };

		inline static ImVec4 s_Red { 255.0f / 255.0f, 40.0f / 255.0f, 73.0f / 255.0f, 1.0f };
		inline static ImVec4 s_Green { 147.0f / 255.0f, 247.0f / 255.0f, 141.4f / 255.0f, 1.0f };
		inline static ImVec4 s_Blue { 141.0f / 255.0f, 200.0f / 255.0f, 247.0f / 255.0f, 1.0f };
		inline static ImVec4 s_Alpha { 247.6f / 255.0f, 188.2f / 255.0f, 140.7f / 255.0f, 1.0f };

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
		inline static float s_TextBackgroundHeight = 21.0f;

		// These values represent the percentage location
		//		of text relative to the size of the current window
		//		Ex: 0.3f means 30% of the width of the window
		inline static float s_SecondaryTextFirstPercentage = 0.43f;
		inline static float s_SecondaryTextSecondPercentage = 0.56f;
		inline static float s_SecondaryTextThirdPercentage = 0.69f;
		inline static float s_SecondaryTextFourthPercentage = 0.82f;
		inline static float s_SecondaryTextMiddlePercentage = 0.655f;


	public:
		//==============================
		// Active Window Fields
		//==============================
		inline static ImVec2 s_WindowPosition{ 0.0f, 0.0f };
		inline static float s_PrimaryTextWidth{ 0.0f };
		inline static float s_PrimaryTextIndentedWidth{ 0.0f };
		inline static float s_SecondaryTextSmallWidth{ 0.0f };
		inline static float s_SecondaryTextMediumWidth{ 0.0f };
		inline static float s_SecondaryTextLargeWidth{ 0.0f };
		inline static float s_SecondaryTextPosOne{ 0.0f };
		inline static float s_SecondaryTextPosTwo{ 0.0f };
		inline static float s_SecondaryTextPosThree{ 0.0f };
		inline static float s_SecondaryTextPosFour{ 0.0f };
		inline static float s_SecondaryTextPosMiddle{ 0.0f };


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

	struct WarningPopupSpec
	{
	public:
		WarningPopupSpec()
		{
			WidgetID = IncrementWidgetCounter();
		}
	public:
		std::string Label;
		float PopupWidth{ 700.0f };
		std::function<void()> PopupContents{ nullptr };
		bool OpenPopup{ false };
	private:
		WidgetID WidgetID;
	private:
		friend void EditorUIService::WarningPopup(WarningPopupSpec& spec);
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
		std::function<void()> PopupContents{ nullptr };
		std::function<void()> ConfirmAction{ nullptr };
		std::function<void()> DeleteAction{ nullptr };
		std::function<void()> PopupAction{ nullptr };
		std::function<void()> CancelAction{ nullptr };
		bool OpenPopup{ false };
	public:
		void CloseActivePopup()
		{
			m_CloseActivePopup = true;
		}
	private:
		WidgetID WidgetID;
		bool m_CloseActivePopup{ false };
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
		bool CurrentBoolean{ false };
		std::function<void(CheckboxSpec&)> ConfirmAction;
		Ref<void> ProvidedData { nullptr };
	private:
		bool Editing{ false };
		WidgetID WidgetID;
	private:
		friend void EditorUIService::Checkbox(CheckboxSpec& spec);
	};

	enum EditIntegerFlags
	{
		EditInteger_None = 0,
		EditInteger_Indented = BIT(0)
	};

	struct EditIntegerSpec
	{
	public:
		EditIntegerSpec()
		{
			WidgetID = IncrementWidgetCounter();
		}
	public:
		std::string Label{};
		WidgetFlags Flags{ EditInteger_None };
		int32_t CurrentInteger{};
		std::function<void(EditIntegerSpec&)> ConfirmAction{ nullptr };
		Ref<void> ProvidedData { nullptr };
	private:
		bool Editing{ false };
		WidgetID WidgetID;
	private:
		friend void EditorUIService::EditInteger(EditIntegerSpec& spec);
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
		std::function<void(EditFloatSpec&)> ConfirmAction{ nullptr };
		Ref<void> ProvidedData { nullptr };
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
		std::function<void(EditVec2Spec&)> ConfirmAction{ nullptr };
		Ref<void> ProvidedData { nullptr };
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
		std::function<void(EditVec3Spec&)> ConfirmAction{ nullptr };
		Ref<void> ProvidedData { nullptr };
	private:
		bool Editing{ false };
		WidgetID WidgetID;
	private:
		friend void EditorUIService::EditVec3(EditVec3Spec& spec);
	};

	enum EditVec4Flags
	{
		EditVec4_None =		0,
		EditVec4_Indented = BIT(0),
		EditVec4_RGBA =		BIT(1)
	};

	struct EditVec4Spec
	{
	public:
		EditVec4Spec()
		{
			WidgetID = IncrementWidgetCounter();
		}
	public:
		std::string Label{};
		WidgetFlags Flags{ EditVec4_None };
		Math::vec4 CurrentVec4{};
		std::function<void(EditVec4Spec&)> ConfirmAction{ nullptr };
		Ref<void> ProvidedData { nullptr };
	private:
		bool Editing{ false };
		WidgetID WidgetID;
	private:
		friend void EditorUIService::EditVec4(EditVec4Spec& spec);
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
		std::function<void(EditTextSpec&)> ConfirmAction;
		bool StartPopup{ false };
		Ref<void> ProvidedData { nullptr };
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
	using CollapsingHeaderSelectionList = std::unordered_map<std::string, std::function<void(EditorUI::CollapsingHeaderSpec& spec)>>;
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
		Ref<void> ProvidedData { nullptr };
	public:
		void ClearSelectionList()
		{
			SelectionList.clear();
		}
		void AddToSelectionList(const std::string& label, std::function<void(CollapsingHeaderSpec&)> function)
		{
			if (!SelectionList.contains(label))
			{
				SelectionList.insert_or_assign(label, function);
				return;
			}
		}
		CollapsingHeaderSelectionList& GetSelectionList()
		{
			return SelectionList;
		}
	private:
		WidgetID WidgetID;
		CollapsingHeaderSelectionList SelectionList{};
	private:
		friend void EditorUIService::CollapsingHeader(CollapsingHeaderSpec& spec);
	};


	struct PanelHeaderSpec
	{
	using PanelHeaderSelectionList = std::unordered_map<std::string, std::function<void(EditorUI::PanelHeaderSpec& spec)>>;
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

	enum NavigationHeaderFlags
	{
		NavigationHeader_None = 0,
		NavigationHeader_AllowDragDrop = BIT(0) 
	};

	struct NavigationHeaderSpec
	{
	public:
		NavigationHeaderSpec()
		{
			WidgetID = IncrementWidgetCounter();
		}
	public:
		FixedString64 m_Label;
		std::function<void()> m_OnNavigateBack{};
		std::function<void()> m_OnNavigateForward{};
		std::function<void(const char*, void*, std::size_t)> m_OnReceivePayloadBack{};
		std::function<void(const char*, void*, std::size_t)> m_OnReceivePayloadForward{};
		std::vector<FixedString32> m_AcceptableOnReceivePayloads;
		WidgetFlags m_Flags{ 0 };
		bool m_IsBackActive{ false };
		bool m_IsForwardActive{ false };
	private:
		WidgetID WidgetID;
	private:
		friend void EditorUIService::NavigationHeader(NavigationHeaderSpec& spec);
	};

	enum GridFlags
	{
		Grid_None = 0,
		Grid_AllowDragDrop = BIT(0)
	};

	static inline uint32_t k_InvalidArchetypeID{ 0 };

	struct GridEntry
	{
		FixedString64 m_Label;
		uint32_t m_ArchetypeID { k_InvalidArchetypeID };
		UUID m_EntryID;
	};

	struct GridEntryArchetype
	{
		// Display metadata
		Ref<Rendering::Texture2D> m_Icon;
		ImVec4 m_IconColor{ EditorUI::EditorUIService::s_DisabledColor };

		// Handle key input
		std::function<void(GridEntry& currentEntry)> m_OnDoubleLeftClick;
		std::function<void(GridEntry& currentEntry)> m_OnLeftClick;
		std::function<void(GridEntry& currentEntry)> m_OnRightClick;

		// Handle create/receive payload
		std::function<void(GridEntry& currentEntry, DragDropPayload& newPayload)> m_OnCreatePayload;
		std::function<void(GridEntry& currentEntry, const char*, void*, std::size_t)> m_OnReceivePayload;
		std::vector<FixedString32> m_AcceptableOnReceivePayloads;
	};


	struct GridSpec
	{
	public:
		GridSpec()
		{
			WidgetID = IncrementWidgetCounter();
		}
	public:
		FixedString16 m_Label;
		float m_CellPadding{ 25.0f };
		float m_CellIconSize { 140.0f };
		WidgetFlags m_Flags{ 0 };

	public:
		bool AddEntry(GridEntry& newEntry) 
		{
			// Ensure a valid archetype id is provided
			if (newEntry.m_ArchetypeID == k_InvalidArchetypeID)
			{
				return false;
			}

			// Ensure UUID is unique inside entry
			while (!ValidateEntryID(newEntry.m_EntryID))
			{
				newEntry.m_EntryID = {};
			}

			// Insert new entry
			Entries.push_back(newEntry);
			return true;
		}
		bool AddEntry(GridEntry&& newEntry)
		{
			// Ensure a valid archetype id is provided
			if (newEntry.m_ArchetypeID == k_InvalidArchetypeID)
			{
				return false;
			}

			// Ensure UUID is unique inside entry
			while (!ValidateEntryID(newEntry.m_EntryID))
			{
				newEntry.m_EntryID = {};
			}

			// Insert new entry
			Entries.push_back(std::move(newEntry));
			return true;
		}

		void ClearEntries()
		{
			Entries.clear();
			ClearSelectedEntry();
		}

		void ClearSelectedEntry()
		{
			SelectedEntry = k_EmptyUUID;
		}

		bool AddEntryArchetype(uint32_t key, const GridEntryArchetype& newArchetype)
		{
			auto [iterator, success] = EntryArchetypes.insert({key, newArchetype});
			return success;
		}

		bool AddEntryArchetype(uint32_t key, GridEntryArchetype&& newArchetype)
		{
			auto [iterator, success] = EntryArchetypes.insert({ key, std::move(newArchetype) });
			return success;
		}

	private:
		bool ValidateEntryID(UUID queryID)
		{
			// Ensure empty id is not provided
			if (queryID == k_EmptyUUID)
			{
				return false;
			}

			// Ensure that no match id is found in internal entries list
			for (GridEntry& entry : Entries)
			{
				if (entry.m_EntryID == queryID)
				{
					return false;
				}
			}

			// Return true if no duplicate is found
			return true;
		}

	private:
		WidgetID WidgetID;
		UUID SelectedEntry { k_EmptyUUID };
		std::vector<GridEntry> Entries{};
		std::unordered_map<uint32_t, GridEntryArchetype> EntryArchetypes;
	private:
		friend void EditorUIService::Grid(GridSpec& spec);
	};

	class TreePath
	{
	public:
		void AddNode(uint16_t newNode)
		{
			m_Path.push_back(newNode);
		}

		void SetNode(uint16_t newValue, size_t location)
		{
			if (location >= m_Path.size())
			{
				KG_WARN("Invalid location provided. Cannot update TreePath node");
				return;
			}

			m_Path.at(location) = newValue;
		}

		void PopBack()
		{
			m_Path.pop_back();
		}

		uint16_t GetBack()
		{
			return m_Path.back();
		}

		void SetBack(uint16_t newNode)
		{
			PopBack();
			m_Path.push_back(newNode);
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

		bool SameParentPath(const TreePath& other) const
		{
			// Parent path cannot be equal if sizes do not match
			if (m_Path.size() != other.m_Path.size())
			{
				return false;
			}

			for (size_t iteration{ 0 }; iteration < m_Path.size(); iteration++)
			{
				// Check if nodes differ
				if (m_Path.at(iteration) != other.m_Path.at(iteration))
				{
					if (iteration == m_Path.size() - 1)
					{
						return true;
					}
					else
					{
						return false;
					}
				}
			}

			// If paths are equivalent, return true
			return true;
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
		Ref<void> ProvidedData { nullptr };
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

		void MoveUp();
		void MoveDown();
		void MoveLeft();
		void MoveRight();

		void SelectFirstEntry();
		TreeEntry* SearchFirstLayer(UUID handle);
		std::vector<TreePath> SearchDepth(std::function<bool(TreeEntry& entry)> searchFunction, size_t depth = 0);
		void EditDepth(std::function<void(TreeEntry& entry)> editFunction, size_t depth = 0);

		void InsertEntry(const TreeEntry& entry)
		{
			TreeEntries.push_back(entry);
		}

		void RemoveEntry(TreePath& path);
		void ClearTree()
		{
			TreeEntries.clear();
			ExpandedNodes.clear();
			SelectedEntry = {};
		}
		std::vector<TreeEntry>& GetTreeEntries()
		{
			return TreeEntries;
		}

		void ClearExpandedNodes()
		{
			ExpandedNodes.clear();
		}

		void ExpandFirstLayer();

		void ExpandNodePath(TreePath& path);

		TreeEntry* GetEntryFromPath(TreePath& path);
		TreePath GetPathFromEntryReference(TreeEntry* entryQuery);
	private:
		void SearchDepthRecursive(TreeEntry& currentEntry, size_t currentDepth, size_t terminalDepth, std::function<bool(TreeEntry& entry)> searchFunction, std::vector<TreePath>& allPaths);
		void EditDepthRecursive(TreeEntry& currentEntry, size_t currentDepth, size_t terminalDepth, std::function<void(TreeEntry& entry)> editFunction);
	private:
		WidgetID WidgetID;
		std::vector<TreeEntry> TreeEntries{};
		std::unordered_set<TreePath> ExpandedNodes{};
		TreeEntry* CurrentRightClick{ nullptr };
		bool SelectionChanged{ false };
	private:
		friend void EditorUIService::Tree(TreeSpec& spec);
		friend void DrawEntries(TreeSpec& spec, std::vector<TreeEntry>& entries, uint32_t& widgetCount, TreePath& currentPath , ImVec2 rootPosition);
	};

	struct TooltipSeperatorData
	{
		ImVec4 m_SeperatorColor{ EditorUI::EditorUIService::s_DisabledColor };
	};

	struct TooltipEntry
	{
	public:
		TooltipEntry(const char* itemLabel, std::function<void(TooltipEntry&)> onClick) : m_Label(itemLabel), m_EntryData(onClick)
		{
		}
		TooltipEntry(const char* itemLabel, const std::vector<TooltipEntry>& subEntryList) : m_Label(itemLabel), m_EntryData(subEntryList)
		{
		}
		TooltipEntry(const char* itemLabel, std::vector<TooltipEntry>&& subEntryList) : m_Label(itemLabel), m_EntryData(std::move(subEntryList))
		{
		}
	private:
		TooltipEntry() {};

	public:
		FixedString32 m_Label;
		UUID m_EntryID;
		bool m_IsVisible{ true };
	private:
		std::variant<std::vector<TooltipEntry>, std::function<void(TooltipEntry&)>, TooltipSeperatorData> m_EntryData;
		friend void ProcessTooltipEntries(TooltipSpec& spec, std::vector<TooltipEntry>& entryList);
		friend struct TooltipSpec;
	};

	struct TooltipSpec
	{
	public:
		TooltipSpec()
		{
			m_WidgetID = IncrementWidgetCounter();
		}
	public:
		std::string m_Label;
		bool TooltipActive{ false };

	public:
		void AddSeperator(ImVec4 seperatorColor)
		{
			TooltipSeperatorData seperatorData{ seperatorColor };
			TooltipEntry newSeperatorEntry;
			newSeperatorEntry.m_EntryData = seperatorData;

			// Ensure UUID is unique inside entry
			while (!ValidateEntryID(newSeperatorEntry.m_EntryID))
			{
				newSeperatorEntry.m_EntryID = {};
			}

			// Add new entry if valid
			m_Entries.push_back(newSeperatorEntry);
		}

		UUID AddTooltipEntry(TooltipEntry& newEntry)
		{
			// Ensure valid label is provided
			if (newEntry.m_Label.IsEmpty())
			{
				return k_EmptyUUID;
			}

			// Ensure UUID is unique inside entry
			while (!ValidateEntryID(newEntry.m_EntryID))
			{
				newEntry.m_EntryID = {};
			}

			// Add new entry if valid
			m_Entries.push_back(newEntry);
			return newEntry.m_EntryID;
		}
		UUID AddTooltipEntry(TooltipEntry&& newEntry)
		{
			// Ensure valid label is provided
			if (newEntry.m_Label.IsEmpty())
			{
				return k_EmptyUUID;
			}

			// Ensure UUID is unique inside entry
			while (!ValidateEntryID(newEntry.m_EntryID))
			{
				newEntry.m_EntryID = {};
			}

			// Add new entry if valid
			UUID cacheID = newEntry.m_EntryID;
			m_Entries.push_back(std::move(newEntry));
			return cacheID;
		}

		bool SetIsVisible(UUID entry, bool isVisible);
		bool SetAllChildrenIsVisible(UUID entry, bool isVisible);
		void ClearEntries()
		{
			m_Entries.clear();
		}

	private:
		bool ValidateEntryID(UUID queryID);
		bool ValidateEntryIDRecursive(std::vector<TooltipEntry>& entries, UUID queryID);
		bool SetIsVisibleRecursive(std::vector<TooltipEntry>& entries, UUID queryID, bool isVisible);
		bool SetAllChildrenIsVisibleRecursive(std::vector<TooltipEntry>& entries, UUID queryID, bool isVisible);
	private:
		WidgetID m_WidgetID;
		std::vector<TooltipEntry> m_Entries{};
	private:
		friend void EditorUIService::Tooltip(TooltipSpec& spec);
		friend void ProcessTooltipEntries(TooltipSpec& spec, std::vector<TooltipEntry>& entryList);
	};


	enum ListFlags
	{
		List_None = 0,
		List_UnderlineTitle = BIT(0), // Adds an underline to the title
		List_RegularSizeTitle = BIT(1), // Ensures title uses normally sized font
		List_Indented = BIT(2) // Indents the list over once
	};

	struct ListEntry
	{
		std::string Label;
		std::string Value;
		UUID Handle;
		std::function<void(ListEntry& entry, std::size_t iteration)> OnEdit { nullptr };
	};

	static inline std::size_t k_ListSearchIndex{ std::numeric_limits<std::size_t>::max() };
	static inline std::size_t k_ListIndex{ std::numeric_limits<std::size_t>::max() };

	struct ListSpec
	{
	public:
		ListSpec()
		{
			WidgetID = IncrementWidgetCounter();
		}
	public:
		std::string Label;
		WidgetFlags Flags{ List_None };
		std::string Column1Title {};
		std::string Column2Title {};
		bool Expanded{ false };
		std::function<void()> OnRefresh { nullptr };
	public:
		void InsertListEntry(const std::string& label, const std::string& value, 
			std::function<void(ListEntry& entry, std::size_t iteration)> onEdit, Assets::AssetHandle handle = 0)
		{
			ListEntry newEntry{label, value, handle, onEdit};
			ListEntries.push_back(newEntry);
		}

		void InsertListEntry(const ListEntry& entry)
		{
			ListEntries.push_back(entry);
		}

		bool RemoveEntry(std::size_t entryIndex)
		{
			if (entryIndex >= ListEntries.size()) 
			{
				return false;
			}
			ListEntries.erase(ListEntries.begin() + entryIndex);
			return true;
		}

		std::size_t SearchEntries(std::function<bool(const ListEntry& currentEntry)> searchFunction)
		{
			// Run search function on entry
			std::size_t iteration{ 0 };
			for (const ListEntry& currentEntry : ListEntries)
			{
				if (searchFunction(currentEntry))
				{
					return iteration;
				}
				iteration++;
			}
			
			// Return invalid index if none is found
			return k_ListSearchIndex;
		}

		void EditEntries(std::function<void(ListEntry& currentEntry)> editFunction)
		{
			// Run edit function on every entry
			for (ListEntry& currentEntry : ListEntries)
			{
				if (editFunction)
				{
					editFunction(currentEntry);
				}
			}
		}

		void ClearList()
		{
			ListEntries.clear();
		}
		void ClearEditListSelectionList()
		{
			EditListSelectionList.clear();
		}
		void AddToSelectionList(const std::string& label, std::function<void()> function)
		{
			if (!EditListSelectionList.contains(label))
			{
				EditListSelectionList.insert_or_assign(label, function);
				return;
			}
		}
		SelectionList& GetEditSelectionList()
		{
			return EditListSelectionList;
		}

		std::size_t GetEntriesListSize()
		{
			return ListEntries.size();
		}

		ListEntry& GetEntry(std::size_t index)
		{
			KG_ASSERT(index < ListEntries.size());
			return ListEntries.at(index);
		}

	private:
		WidgetID WidgetID;
		std::vector<ListEntry> ListEntries{};
		SelectionList EditListSelectionList{};
	private:
		friend void EditorUIService::List(ListSpec& spec);
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
		SelectOption_PopupOnly = BIT(1), // Determines if line of text is generated for options
		SelectOption_HandleEditButtonExternally = BIT(2) // Allows calling a custom function for edit button
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
		std::function<void()> OnEdit{ nullptr };
		std::function<void(const OptionEntry&)> ConfirmAction {nullptr};
		std::function<void()> PopupAction {nullptr};
		// Only used if PopupOnly is true
		bool OpenPopup{ false };
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
