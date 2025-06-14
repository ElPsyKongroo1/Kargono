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

	struct UndoMemento
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		UndoMemento(Math::vec4 value, EditVec4Widget* widget) : m_Value(value), m_Widget(widget) {}
	private:
		UndoMemento() = default;
	public:
		//==============================
		// Public Fields
		//==============================
		Math::vec4 m_Value;
		EditVec4Widget* m_Widget;
	};

	class UndoStack
	{
	private:
		//==============================
		// Constructors/Destructors
		//==============================
		UndoStack() = default;
		~UndoStack() = default;
	public:
		//==============================
		// Modify Stack
		//==============================
		void StoreUndo(const UndoMemento& memento);
		bool Undo();
	private:
		//==============================
		// Internal Fields
		//==============================
		std::stack<UndoMemento> m_Stack;
	private:
		//==============================
		// Owning Class
		//==============================
		friend class EditorUIContext;
	};

	class ActiveWindowData
	{
	private:
		//==============================
		// Constructors/Destructors
		//==============================
		ActiveWindowData(ImVec4 defaultBackgroundColor, ConfigSpacing* spacing) : 
			m_ActiveBackgroundColor(defaultBackgroundColor), i_ConfigSpacing(spacing)
		{
			KG_ASSERT(i_ConfigSpacing);
		}
		~ActiveWindowData() = default;

	public:
		//==============================
		// Revalidate Window
		//==============================
		void RecalculateDimensions();
	public:
		//==============================
		// Active Window Fields
		//==============================
		// Window Info
		ImVec2 m_WindowPosition{ 0.0f, 0.0f };
		// Primary Text
		float m_PrimaryTextWidth{ 0.0f };
		float m_PrimaryTextIndentedWidth{ 0.0f };
		// Secondary Text
		float m_SecondaryTextSmallWidth{ 0.0f };
		float m_SecondaryTextMediumWidth{ 0.0f };
		float m_SecondaryTextLargeWidth{ 0.0f };
		float m_SecondaryTextPosOne{ 0.0f };
		float m_SecondaryTextPosTwo{ 0.0f };
		float m_SecondaryTextPosThree{ 0.0f };
		float m_SecondaryTextPosFour{ 0.0f };
		float m_SecondaryTextPosMiddle{ 0.0f };
		// Color
		ImVec4 m_ActiveBackgroundColor{ 0.0f, 0.0f, 0.0f, 0.0f };

	private:
		//==============================
		// Injected Dependency(s)
		//==============================
		ConfigSpacing* i_ConfigSpacing{ nullptr };
	private:
		//==============================
		// Owning Class
		//==============================
		friend class EditorUIContext;
	};

	class UIPresets
	{
	private:
		//==============================
		// Constructors/Destructors
		//==============================
		UIPresets(GeneralIcons* genIcons, ContentBrowserIcons* contentIcons, ConfigSpacing* configSpace) :
			i_GenIcons(genIcons), i_BrowserIcons(contentIcons), i_ConfigSpacing(configSpace)
		{
			KG_ASSERT(i_GenIcons);
			KG_ASSERT(i_BrowserIcons);
			KG_ASSERT(i_ConfigSpacing);
		}
		~UIPresets() = default;
	public:
		//==============================
		// Create Presets
		//==============================
		void CreatePresets();
	public:
		//==============================
		// Public Fields
		//==============================
		// Defaults
		InlineButton m_SmallEditButton;
		InlineButton m_SmallExpandButton;
		InlineButton m_MediumOptionsButton;
		InlineButton m_SmallCheckboxButton;
		InlineButton m_SmallCheckboxDisabledButton;
		InlineButton m_SmallLinkButton;
		InlineButton m_LargeDeleteButton;
		InlineButton m_LargeCancelButton;
		InlineButton m_LargeConfirmButton;
		InlineButton m_LargeSearchButton;
		// Custom
		InlineButton m_TableEditButton{};
		InlineButton m_TableLinkButton{};
		InlineButton m_ListExpandButton{};
	private:
		//==============================
		// Injected Dependencies
		//==============================
		GeneralIcons* i_GenIcons{ nullptr };
		ContentBrowserIcons* i_BrowserIcons{ nullptr };
		ConfigSpacing* i_ConfigSpacing{ nullptr };
	private:
		//==============================
		// Owning Class
		//==============================
		friend class EditorUIContext;
	};

	class EditorUIContext
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		EditorUIContext() = default;
		~EditorUIContext() = default;
	public:
		//==============================
		// LifeCycle Functions
		//==============================
		static void Init();
		static void Terminate();
	public:
		//==============================
		// On Events
		//==============================
		static bool OnInputEvent(Events::Event* e);
	public:
		//==============================
		// Public Rendering
		//==============================
		static void StartRendering();
		static void EndRendering();

		static void StartDockspaceWindow();
		static void EndDockspaceWindow();

		static void StartRenderWindow(const char* label, bool* closeWindow = nullptr, int32_t flags = 0);
		static void EndRenderWindow();

		static void Spacing(float space);
		static void Spacing(SpacingAmount space);

		static void TitleText(const std::string& text);
		static void NewItemScreen(const std::string& label1, std::function<void()> func1, const std::string& label2, std::function<void()> func2);

		static void LabeledText(const char* m_Label, const char* Text, LabeledTextFlags flags = LabeledText_None);
		static void Text(const char* text);
		static void BeginTabBar(const char* title);
		static void EndTabBar();
		static bool BeginTabItem(const char* title);
		static void EndTabItem();

		static void RenderInfoNotification(const char* text, int delayMS);
		static void RenderWarningNotification(const char* text, int delayMS);
		static void RenderCriticalNotification(const char* text, int delayMS);
	public:
		//==============================
		// Internal Rendering
		//==============================
		static void RenderImage(Ref<Rendering::Texture2D> image, float size, ImVec4 tint = { 1.0f ,1.0f, 1.0f, 1.0f });
		static void RenderTruncatedText(const std::string& text, uint32_t maxTextSize);
		static void RenderMultiLineText(const std::string& text, float lineWidth, float xOffset = 0, float yOffset = 0);
		static void RenderInlineButton(ImGuiID widgetID, std::function<void()> onPress,
			const InlineButton& spec, bool active = false, ImVec4 tintColor = { 1.0f, 1.0f, 1.0f, 1.0f });
		static bool RenderColorPicker(const char* name, ImVec4& mainColor);
		static void RenderNotifications();
	private:
		// Rendering helper(s)
		static bool DrawColorPickerButtonInternal(const char* desc_id, const ImVec4& col, const ImVec2& size_arg);
		static bool DrawColorPickerPopupContents(const char* label, float col[4], const float* ref_col);
	public:
		//==============================
		// Query UI State
		//==============================
		static WidgetID GetNextChildID(WidgetID& id);
		static uint32_t GetActiveWidgetID();
		static bool IsAnyItemHovered();
		static const char* GetFocusedWindowName();
		static void SetFocusedWindow(const char* windowName);
		static void BringWindowToFront(const char* windowName);
		static void BringCurrentWindowToFront();
		static void ClearWindowFocus();
		static bool IsCurrentWindowVisible();
		static void HighlightFocusedWindow();
		static void SetDisableLeftClick(bool option);
		static void BlockMouseEvents(bool block);
		static const char* GetHoveredWindowName();
		static void CalculateViewportDimensions(Math::vec2 screenViewportBounds[2], ViewportData& viewportData, bool& viewportFocused, bool& viewportHovered, const Math::uvec2& aspectRatio);
		static void SkipMouseIconChange();
		static float SmallButtonRelativeLocation(uint32_t slot);

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
		inline static ConfigSpacing m_ConfigSpacing{};
		// Fonts
		inline static ConfigFonts m_ConfigFonts{};
		// Undo
		inline static UndoStack s_UndoStack{};
		// Presets
		inline static UIPresets m_UIPresets{&m_GenIcons, &m_ContentBrowserIcons, &m_ConfigSpacing};
		// Active Data
		inline static ActiveWindowData m_ActiveWindowData{ m_ConfigColors.m_DarkBackgroundColor, &m_ConfigSpacing };
	private:
		//==============================
		// Internal Fields
		//==============================
		inline static bool s_BlockMouseEvents{ true };
		inline static bool s_Running{ false };
		inline static bool s_DisableLeftClick{ false };
		inline static bool s_BlockMouseIconChange{ false };
	};
}

namespace Kargono::Utility
{
	inline Math::vec4 ImVec4ToMathVec4(const ImVec4& color )
	{
		return { color.x, color.y, color.z, color.w };
	}
	inline ImVec4 MathVec4ToImVec4(const Math::vec4& color)
	{
		return { color.x, color.y, color.z, color.w };
	}
}
