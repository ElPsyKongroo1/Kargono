#pragma once

#include "Kargono/Core/Base.h"
#include "Modules/Events/ApplicationEvent.h"
#include "Modules/Assets/Asset.h"
#include "Kargono/Core/Buffer.h"
#include "Kargono/Core/WrappedData.h"
#include "Kargono/Core/Window.h"

#include "Modules/EditorUI/EditorUICommon.h"
#include "Modules/EditorUI/EditorUIResources.h"
#include "Modules/EditorUI/Widgets/EditorUIWidget.h"
#include "Modules/EditorUI/Widgets/EditorUISelectOptionWidget.h"
#include "Modules/EditorUI/Widgets/EditorUIFloatWidget.h"

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

namespace Kargono::EditorUI
{
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
			const InlineButton& spec, bool active = false, ImVec4 tintColor = { 1.0f, 1.0f, 1.0f, 1.0f });
		static uint32_t WidgetIterator(uint32_t& count);

	public:
		//==============================
		// Create/Display Widget Functions
		//==============================
		static void TitleText(const std::string& text);
		static void NewItemScreen(const std::string& label1, std::function<void()> func1, const std::string& label2, std::function<void()> func2);

		static void LabeledText(const char* m_Label, const char* Text, LabeledTextFlags flags = LabeledText_None);
		static void Text(const char* text);
		static void BeginTabBar(const char* title);
		static void EndTabBar();
		static bool BeginTabItem(const char* title);
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
		static void RenderNotifications();

	public:
		// Icons
		inline static GeneralIcons m_GenIcons{};
		inline static SceneGraphIcons m_SceneIcons{};
		inline static ViewportIcons m_ViewportIcons{};
		inline static RuntimeUIIcons m_RuntimeUIIcons{};
		inline static ScriptingIcons m_ScriptingIcons{};
		inline static ContentBrowserIcons m_ContentBrowserIcons{};

		// Colors
		inline static ConfigColors m_ConfigColors{};

		// Spacing

		// Fonts
		inline static ConfigFonts m_ConfigFonts{};

	public:
		//==============================
		// UI Button Presets
		//==============================
		inline static InlineButton s_SmallEditButton;
		inline static InlineButton s_SmallExpandButton;
		inline static InlineButton s_MediumOptionsButton;
		inline static InlineButton s_SmallCheckboxButton;
		inline static InlineButton s_SmallCheckboxDisabledButton;
		inline static InlineButton s_SmallLinkButton;
		inline static InlineButton s_LargeDeleteButton;
		inline static InlineButton s_LargeCancelButton;
		inline static InlineButton s_LargeConfirmButton;
		inline static InlineButton s_LargeSearchButton;

		// Buttons
		inline static InlineButton s_TableEditButton{};
		inline static InlineButton s_TableLinkButton{};
		inline static InlineButton s_ListExpandButton{};

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
		// Window Info
		inline static ImVec2 s_WindowPosition{ 0.0f, 0.0f };
		// Primary Text
		inline static float s_PrimaryTextWidth{ 0.0f };
		inline static float s_PrimaryTextIndentedWidth{ 0.0f };
		// Secondary Text
		inline static float s_SecondaryTextSmallWidth{ 0.0f };
		inline static float s_SecondaryTextMediumWidth{ 0.0f };
		inline static float s_SecondaryTextLargeWidth{ 0.0f };
		inline static float s_SecondaryTextPosOne{ 0.0f };
		inline static float s_SecondaryTextPosTwo{ 0.0f };
		inline static float s_SecondaryTextPosThree{ 0.0f };
		inline static float s_SecondaryTextPosFour{ 0.0f };
		inline static float s_SecondaryTextPosMiddle{ 0.0f };
		// Color
		inline static ImVec4 s_ActiveBackgroundColor{ m_ConfigColors.s_DarkBackgroundColor };
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
