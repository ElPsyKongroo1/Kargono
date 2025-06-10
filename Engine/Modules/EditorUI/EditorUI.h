#pragma once

#include "Kargono/Core/Base.h"
#include "Modules/Events/ApplicationEvent.h"
#include "Modules/Assets/Asset.h"
#include "Kargono/Core/Buffer.h"
#include "Kargono/Core/WrappedData.h"
#include "Kargono/Core/Window.h"

#include "Modules/EditorUI/EditorUICommon.h"
#include "Modules/EditorUI/Widgets/EditorUIWidget.h"
#include "Modules/EditorUI/Widgets/EditorUISelectOptionWidget.h"

#include "Modules/EditorUI/ExternalAPI/ImGuiAPI.h"
#include "Modules/EditorUI/ExternalAPI/ImGuizmoAPI.h"

#include <functional>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <array>
#include <stack>
#include <optional>


namespace Kargono::Rendering { class Texture2D; }

namespace Kargono::EditorUI
{
	struct TreeWidget;
	struct InlineButtonWidget;
	struct EditVariableWidget;
	struct EditIntegerWidget;
	struct EditIVec2Widget;
	struct EditIVec3Widget;
	struct EditIVec4Widget;
	struct EditFloatWidget;
	struct EditVec2Widget;
	struct EditVec3Widget;
	struct EditVec4Widget;

	enum LabeledTextFlags : WidgetFlags
	{
		LabeledText_None = 0,
		LabeledText_Indented = BIT(0)
	};

	struct EditorMemento
	{
	public:
		EditorMemento(Math::vec4 value, EditVec4Widget* widget) : m_Value(value), m_Widget(widget) {}
	private:
		EditorMemento() = default;
	public:
		Math::vec4 m_Value;
		EditVec4Widget* m_Widget;
	};

	class MementoStack
	{
	public:
		void AddMemento(const EditorMemento& memento);
		Optional<EditorMemento> PopMemento();
	private:
		std::stack<EditorMemento> m_Stack;
	};

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

		static void StartDockspaceWindow();
		static void EndDockspaceWindow();
	public:
		//==============================
		// Event Functions
		//==============================
		static bool OnInputEvent(Events::Event* e);

	public:
		//==============================
		// Interact with EditorUIService context
		//==============================
		static bool Undo();
		static void StoreUndoMemento(const EditorMemento& memento);
	public:
		//==============================
		// Modify Visuals Functions
		//==============================
		static void Spacing(float space);
		static void Spacing(SpacingAmount space);

		static void CreateImage(Ref<Rendering::Texture2D> image, float size, ImVec4 tint = { 1.0f ,1.0f, 1.0f, 1.0f });
		static float SmallButtonRelativeLocation(uint32_t slot);
		static void RecalculateWindowDimensions();
		static OptionMap GenerateSearchCache(OptionMap& originalList, const std::string& searchQuery);
		static void TruncateText(const std::string& text, uint32_t maxTextSize);
		static void WriteMultilineText(const std::string& text, float lineWidth, float xOffset = 0, float yOffset = 0);
		static void CreateButton(ImGuiID widgetID, std::function<void()> onPress,
			const InlineButtonWidget& spec, bool active = false, ImVec4 tintColor = { 1.0f, 1.0f, 1.0f, 1.0f });
		static uint32_t WidgetIterator(uint32_t& count);

	public:
		//==============================
		// Create/Display Widget Functions
		//==============================
		static void TitleText(const std::string& text);
		static void EditVariable(EditVariableWidget& spec);
		static void NewItemScreen(const std::string& label1, std::function<void()> func1, const std::string& label2, std::function<void()> func2);

		static void EditInteger(EditIntegerWidget& spec);
		static void EditIVec2(EditIVec2Widget& spec);
		static void EditIVec3(EditIVec3Widget& spec);
		static void EditIVec4(EditIVec4Widget& spec);
		static void EditFloat(EditFloatWidget& spec);
		static void EditVec2(EditVec2Widget& spec);
		static void EditVec3(EditVec3Widget& spec);
		static void EditVec4(EditVec4Widget& spec);

		static void Tree(TreeWidget& spec);
		static void LabeledText(const std::string& m_Label, const std::string& Text, LabeledTextFlags flags = LabeledText_None);
		static void Text(const char* text);
		static void BeginTabBar(const std::string& title);
		static void EndTabBar();
		static bool BeginTabItem(const std::string& title);
		static void EndTabItem();

	public:
		//==============================
		// Draw Functions Used By Widgets
		//==============================
		static bool DrawColorPickerButton(const char* name, ImVec4& mainColor);

	private:
		// Internal button rendering
		static bool DrawColorPickerButtonInternal(const char* desc_id, const ImVec4& col, const ImVec2& size_arg);
		static bool DrawColorPickerPopupContents(const char* label, float col[4], const float* ref_col);

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
		static void CreateInfoNotification(const char* text, int delayMS);
		static void CreateWarningNotification(const char* text, int delayMS);
		static void CreateCriticalNotification(const char* text, int delayMS);
		static void AutoCalcViewportSize(Math::vec2 screenViewportBounds[2], ViewportData& viewportData, bool& viewportFocused, bool& viewportHovered, const Math::uvec2& aspectRatio);
		static void SkipMouseIconChange();

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

		// Buttons
		inline static InlineButtonWidget s_TableEditButton{};
		inline static InlineButtonWidget s_TableLinkButton{};
		inline static InlineButtonWidget s_ListExpandButton{};

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
		inline static Ref<Rendering::Texture2D> s_IconBoxCollider, s_IconCircleCollider, s_IconEntity, s_IconParticles,
			s_IconClassInstance, s_IconRigidBody, s_IconTag, s_IconTransform;

		// Viewport icons
		inline static Ref<Rendering::Texture2D> s_IconDisplay, s_IconSimulate, s_IconStep, s_IconPlay,
			s_IconPause, s_IconStop, s_IconGrid;

		// Runtime UI icons
		inline static Ref<Rendering::Texture2D> s_IconWindow, s_IconTextWidget, s_IconImageButtonWidget,
			s_IconButtonWidget, s_IconInputTextWidget, s_IconUserInterface2, s_IconSliderWidget,
			s_IconDropDownWidget, s_IconHorizontalContainer, s_IconVerticalContainer;

		// Scripting icons
		inline static Ref<Rendering::Texture2D> s_IconNumber, s_IconVariable, s_IconFunction, s_IconBoolean, s_IconDecimal;

		// Content browser icons
		inline static Ref<Rendering::Texture2D> s_IconDirectory, s_IconGenericFile,
			s_IconBack, s_IconForward,
			s_IconAudio, s_IconTexture, s_IconBinary,
			s_IconScene, s_IconScene_KG, s_IconRegistry,
			s_IconUserInterface, s_IconFont, s_IconInput, s_IconAI_KG,
			s_IconAudio_KG, s_IconFont_KG, s_IconGlobalState, s_IconProjectComponent,
			s_IconScript, s_IconTexture_KG, s_IconEmitterConfig, s_IconEnum, s_IconColorPalette;

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
		inline static ImVec4 s_HighlightColor1_UltraThin { s_HighlightColor1.x, s_HighlightColor1.y, s_HighlightColor1.z, s_HighlightColor1.w * 0.3f };
		inline static ImVec4 s_HighlightColor2{ 147.0f / 255.0f, 247.0f / 255.0f, 141.4f / 255.0f, 1.0f };
		inline static ImVec4 s_HighlightColor2_Thin{ s_HighlightColor2.x, s_HighlightColor2.y, s_HighlightColor2.z, s_HighlightColor2.w * 0.75f };
		inline static ImVec4 s_HighlightColor2_UltraThin{ s_HighlightColor2.x, s_HighlightColor2.y, s_HighlightColor2.z, s_HighlightColor2.w * 0.3f };
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

		// Cached colors (Meant to be modifed at runtime)
		inline static ImVec4 s_ActiveBackgroundColor{ s_DarkBackgroundColor };
	public:
		//==============================
		// UI Button Presets
		//==============================
		inline static InlineButtonWidget s_SmallEditButton;
		inline static InlineButtonWidget s_SmallExpandButton;
		inline static InlineButtonWidget s_MediumOptionsButton;
		inline static InlineButtonWidget s_SmallCheckboxButton;
		inline static InlineButtonWidget s_SmallCheckboxDisabledButton;
		inline static InlineButtonWidget s_SmallLinkButton;
		inline static InlineButtonWidget s_LargeDeleteButton;
		inline static InlineButtonWidget s_LargeCancelButton;
		inline static InlineButtonWidget s_LargeConfirmButton;
		inline static InlineButtonWidget s_LargeSearchButton;

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
		inline static bool s_BlockMouseEvents{ true };
		inline static bool s_Running{ false };
		inline static bool s_DisableLeftClick{ false };
		inline static bool s_BlockMouseIconChange{ false };

		inline static MementoStack s_UndoStack;
	};

	enum EditIntegerFlags : WidgetFlags
	{
		EditInteger_None = 0,
		EditInteger_Indented = BIT(0)
	};

	struct EditIntegerWidget : public Widget
	{
	public:
		EditIntegerWidget() : Widget() {}
	public:
		FixedString32 m_Label{};
		WidgetFlags m_Flags{ EditInteger_None };
		int32_t m_CurrentInteger{};
		std::array<int32_t, 2> m_Bounds{ 0, 0 };
		int32_t m_ScrollSpeed{ 1 };
		std::function<void(EditIntegerWidget&)> m_ConfirmAction{ nullptr };
		Ref<void> m_ProvidedData { nullptr };
	private:
		bool m_Editing{ false };
	private:
		friend void EditorUIService::EditInteger(EditIntegerWidget& spec);
	};

	enum EditIVec2Flags : WidgetFlags
	{
		EditIVec2_None = 0,
		EditIVec2_Indented = BIT(0)
	};

	struct EditIVec2Widget : public Widget
	{
	public:
		EditIVec2Widget() : Widget() {}
	public:
		FixedString32 m_Label{};
		WidgetFlags m_Flags{ EditIVec2_None };
		Math::ivec2 m_CurrentIVec2{};
		std::array<int32_t, 2> m_Bounds{ 0, 10'000 };
		int32_t m_ScrollSpeed{ 1 };
		std::function<void(EditIVec2Widget&)> m_ConfirmAction{ nullptr };
		Ref<void> m_ProvidedData{ nullptr };
	private:
		bool m_Editing{ false };
	private:
		friend void EditorUIService::EditIVec2(EditIVec2Widget& spec);
	};

	enum EditIVec3Flags : WidgetFlags
	{
		EditIVec3_None = 0,
		EditIVec3_Indented = BIT(0)
	};

	struct EditIVec3Widget : public Widget
	{
	public:
		EditIVec3Widget() : Widget() {}
	public:
		FixedString32 m_Label{};
		WidgetFlags m_Flags{ EditIVec3_None };
		Math::ivec3 m_CurrentIVec3{};
		std::array<int32_t, 2> m_Bounds{ 0, 10'000 };
		int32_t m_ScrollSpeed{ 1 };
		std::function<void(EditIVec3Widget&)> m_ConfirmAction{ nullptr };
		Ref<void> m_ProvidedData{ nullptr };
	private:
		bool m_Editing{ false };
	private:
		friend void EditorUIService::EditIVec3(EditIVec3Widget& spec);
	};

	enum EditIVec4Flags : WidgetFlags
	{
		EditIVec4_None = 0,
		EditIVec4_Indented = BIT(0)
	};

	struct EditIVec4Widget : public Widget
	{
	public:
		EditIVec4Widget() : Widget() {}
	public:
		FixedString32 m_Label{};
		WidgetFlags m_Flags{ EditIVec4_None };
		Math::ivec4 m_CurrentIVec4{};
		std::array<int32_t, 2> m_Bounds{ 0, 10'000 };
		int32_t m_ScrollSpeed{ 1 };
		std::function<void(EditIVec4Widget&)> m_ConfirmAction{ nullptr };
		Ref<void> m_ProvidedData{ nullptr };
	private:
		bool m_Editing{ false };
	private:
		friend void EditorUIService::EditIVec4(EditIVec4Widget& spec);
	};

	enum EditFloatFlags : WidgetFlags
	{
		EditFloat_None = 0,
		EditFloat_Indented = BIT(0)
	};

	struct EditFloatWidget : public Widget
	{
	public:
		EditFloatWidget() : Widget() {}
	public:
		FixedString32 m_Label{};
		WidgetFlags m_Flags{ EditFloat_None };
		float m_CurrentFloat{};
		std::function<void(EditFloatWidget&)> m_ConfirmAction{ nullptr };
		Ref<void> m_ProvidedData { nullptr };
		std::array<float, 2> m_Bounds{ 0.0f, 0.0f };
		float m_ScrollSpeed{ 0.01f };
	private:
		bool m_Editing{ false };
	private:
		friend void EditorUIService::EditFloat(EditFloatWidget& spec);
	};

	enum EditVec2Flags : WidgetFlags
	{
		EditVec2_None = 0,
		EditVec2_Indented = BIT(0)
	};

	struct EditVec2Widget : public Widget
	{
	public:
		EditVec2Widget() : Widget() {}
	public:
		FixedString32 m_Label{};
		WidgetFlags m_Flags{ EditVec2_None };
		Math::vec2 m_CurrentVec2{};
		std::function<void(EditVec2Widget&)> m_ConfirmAction{ nullptr };
		Ref<void> m_ProvidedData { nullptr };
		std::array<float, 2> m_Bounds{ 0.0f,0.0f };
		float m_ScrollSpeed{ 0.01f };
	private:
		bool m_Editing{ false };
	private:
		friend void EditorUIService::EditVec2(EditVec2Widget& spec);
	};

	enum EditVec3Flags : WidgetFlags
	{
		EditVec3_None = 0,
		EditVec3_Indented = BIT(0)
	};

	struct EditVec3Widget : public Widget
	{
	public:
		EditVec3Widget() : Widget() {}
	public:
		FixedString32 m_Label{};
		WidgetFlags m_Flags{ EditVec3_None };
		Math::vec3 m_CurrentVec3{};
		std::function<void(EditVec3Widget&)> m_ConfirmAction{ nullptr };
		Ref<void> m_ProvidedData { nullptr };
		std::array<float, 2> m_Bounds{ 0.0f, 0.0f };
		float m_ScrollSpeed{ 0.01f };
	private:
		bool m_Editing{ false };
	private:
		friend void EditorUIService::EditVec3(EditVec3Widget& spec);
	};

	enum EditVec4Flags : WidgetFlags
	{
		EditVec4_None =		0,
		EditVec4_Indented = BIT(0),
		EditVec4_RGBA =		BIT(1),
		EditVec4_HandleEditButtonExternally = BIT(2)
	};

	struct EditVec4Widget : public Widget
	{
	public:
		EditVec4Widget() : Widget() {}
	public:
		FixedString32 m_Label{};
		WidgetFlags m_Flags{ EditVec4_None };
		Math::vec4 m_CurrentVec4{};
		std::function<void(EditVec4Widget&)> m_ConfirmAction{ nullptr };
		std::function<void(EditVec4Widget&)> m_OnEdit{ nullptr };
		Ref<void> m_ProvidedData { nullptr };
		std::array<float, 2> m_Bounds{ 0.0f, 0.0f };
		float m_ScrollSpeed{ 0.01f };
		bool m_Editing{ false };
	private:
		friend void EditorUIService::EditVec4(EditVec4Widget& spec);
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
		std::string m_Label{};
		std::function<void(TreeEntry&)> m_OnClick { nullptr };
	};

	struct TreeEntry
	{
		std::string m_Label {};
		UUID m_Handle {};
		Ref<Rendering::Texture2D> m_IconHandle{ nullptr };
		std::function<void(TreeEntry& entry)> m_OnLeftClick { nullptr };
		std::function<void(TreeEntry& entry)> m_OnDoubleLeftClick { nullptr };
		std::function<void(TreeEntry& entry)> m_OnRightClick { nullptr };
		Ref<void> m_ProvidedData { nullptr };
		std::vector<TreeEntry> m_SubEntries{};
	};

	struct TreeWidget : public Widget
	{
	public:
		TreeWidget() : Widget() {}
	public:
		FixedString32 m_Label;
		TreePath m_SelectedEntry{};
		std::function<void()> m_OnRefresh { nullptr };
	public:

		void MoveUp();
		void MoveDown();
		void MoveLeft();
		void MoveRight();

		void SelectFirstEntry();
		bool SelectEntry(TreePath& path);
		TreeEntry* SearchFirstLayer(UUID queryHandle);
		TreeEntry* SearchDepth(UUID queryHandle, size_t terminalDepth = 0);
		std::vector<TreePath> SearchDepth(std::function<bool(TreeEntry& entry)> searchFunction, size_t depth = 0);
		void EditDepth(std::function<void(TreeEntry& entry)> editFunction, size_t depth = 0);

		void InsertEntry(const TreeEntry& entry)
		{
			m_TreeEntries.push_back(entry);
		}

		void RemoveEntry(TreePath& path);
		void ClearTree()
		{
			m_TreeEntries.clear();
			m_ExpandedNodes.clear();
			m_SelectedEntry = {};
		}
		std::vector<TreeEntry>& GetTreeEntries()
		{
			return m_TreeEntries;
		}

		void ClearExpandedNodes()
		{
			m_ExpandedNodes.clear();
		}

		void ExpandFirstLayer();

		void ExpandNodePath(TreePath& path);

		TreeEntry* GetEntryFromPath(TreePath& path);
		TreePath GetPathFromEntryReference(TreeEntry* entryQuery);
	private:
		void SearchDepthRecursive(TreeEntry& currentEntry, size_t currentDepth, size_t terminalDepth, std::function<bool(TreeEntry& entry)> searchFunction, std::vector<TreePath>& allPaths);
		TreeEntry* SearchDepthRecursive(TreeEntry& currentEntry, size_t currentDepth, size_t terminalDepth, UUID queryID);
		void EditDepthRecursive(TreeEntry& currentEntry, size_t currentDepth, size_t terminalDepth, std::function<void(TreeEntry& entry)> editFunction);
	private:
		std::vector<TreeEntry> m_TreeEntries{};
		std::unordered_set<TreePath> m_ExpandedNodes{};
		bool m_SelectionChanged{ false };
	private:
		friend void EditorUIService::Tree(TreeWidget& spec);
		friend void DrawEntries(TreeWidget& spec, std::vector<TreeEntry>& entries, uint32_t& widgetCount, TreePath& currentPath , ImVec2 rootPosition);
	};

	struct EditVariableWidget : public Widget
	{
	public:
		EditVariableWidget() : Widget() {}
	public:
		FixedString32 m_Label;
		Buffer FieldBuffer {};
		WrappedVarType VariableType{ WrappedVarType::Integer32 };
	public:
		void AllocateBuffer()
		{
			FieldBuffer.Allocate(400);
			FieldBuffer.SetDataToByte(0);
		}
	private:
		friend void EditorUIService::EditVariable(EditVariableWidget&);
	};


}

namespace Kargono::Utility
{
	//==============================
	// ImVec4 -> Math::vec4
	//==============================
	inline Math::vec4 ImVec4ToMathVec4(const ImVec4& color )
	{
		return { color.x, color.y, color.z, color.w };
	}

	//==============================
	// Math::vec4 -> ImVec4
	//==============================
	inline ImVec4 MathVec4ToImVec4(const Math::vec4& color)
	{
		return { color.x, color.y, color.z, color.w };
	}
}
