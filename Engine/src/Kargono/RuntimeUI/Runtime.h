#pragma once

#include "Kargono/Renderer/EditorCamera.h"
#include "Kargono/Core/Base.h"
#include "Kargono/RuntimeUI/Text.h"
#include "Kargono/Assets/Asset.h"
#include "Kargono/Math/Math.h"

#include <vector>
#include <string>
#include <unordered_map>
#include <functional>


namespace Kargono::RuntimeUI
{
	//============================
	// Function Callbacks Struct
	//============================
	struct WidgetCallbacks
	{
		std::string OnPress {};
	};

	struct UICallbacks
	{
		std::string OnMove {};
	};

	//============================
	// Widget Types Enum
	//============================
	enum class WidgetTypes
	{
		None = 0, TextWidget, ButtonWidget, CheckboxWidget, ComboWidget, PopupWidget
	};

	//============================
	// Direction Pointers Struct
	//============================
	struct DirectionPointers
	{
		int32_t Left { -1 };
		int32_t Right { -1 };
		int32_t Up { -1 };
		int32_t Down { -1 };
	};
	
	//============================================================
	// Widget Class
	//============================================================
	class Widget
	{
	public:
		virtual ~Widget() = default;
	public:
		virtual void PushRenderData(Math::vec3 translation, const Math::vec3& scale, float viewportWidth) = 0;
	public:
		std::string Tag{ "None" };
		Math::vec2 WindowPosition{ 0.4f };
		Math::vec2 Size  {0.3f};
		Math::vec4 DefaultBackgroundColor{1.0f};
		Math::vec4 ActiveBackgroundColor{1.0f};
		WidgetCallbacks FunctionPointers {};
		WidgetTypes WidgetType{ WidgetTypes::None };
		DirectionPointers DirectionPointer{};
		bool Selectable{ true };
	};


	class TextWidget : public Widget
	{
	public:
		TextWidget()
			: Widget()
		{
			WidgetType = WidgetTypes::TextWidget;
			Size = { 0.3f, 0.1f };
			CalculateTextSize();
		}
		virtual ~TextWidget() override = default;
	public:
		virtual void PushRenderData(Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth) override;
	private:
		void CalculateTextSize();

	public:
		void SetText(const std::string& newText);

		std::string Text{ "New Text" };
		float TextSize{ 0.12f };
		Math::vec2 TextAbsoluteDimensions {};
		Math::vec4 TextColor{0.5f};
		bool TextCentered = true;
		friend class Runtime;
	};

	class ButtonWidget : public Widget
	{
	public:
		ButtonWidget()
			: Widget()
		{
			WidgetType = WidgetTypes::ButtonWidget;
		}
		virtual ~ButtonWidget() override = default;
	public:
		virtual void PushRenderData(Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth) override;
	};

	class CheckboxWidget : public Widget
	{
	public:
		CheckboxWidget()
			: Widget()
		{
			WidgetType = WidgetTypes::CheckboxWidget;
		}
		virtual ~CheckboxWidget() override = default;
	public:
		virtual void PushRenderData(Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth) override;
	};

	class ComboWidget : public Widget
	{
	public:
		ComboWidget()
			: Widget()
		{
			WidgetType = WidgetTypes::ComboWidget;
		}
		virtual ~ComboWidget() override = default;
	public:
		virtual void PushRenderData(Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth) override;
	};

	class PopupWidget : public Widget
	{
	public:
		PopupWidget()
			: Widget()
		{
			WidgetType = WidgetTypes::PopupWidget;
		}
		virtual ~PopupWidget() override = default;
	public:
		virtual void PushRenderData(Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth) override;
	};

	struct Window
	{
		struct WidgetCounts
		{
			uint16_t TextWidgetCount{ 0 };
			uint16_t TextWidgetLocation{};
			uint16_t ButtonWidgetCount{ 0 };
			uint16_t ButtonWidgetLocation{};
			uint16_t CheckboxWidgetCount{ 0 };
			uint16_t CheckboxWidgetLocation{};
			uint16_t ComboWidgetCount{ 0 };
			uint16_t ComboWidgetLocation{};
			uint16_t PopupWidgetCount{ 0 };
			uint16_t PopupWidgetLocation{};
		};
	public:
		std::string Tag{ "None" };
		Math::vec3 ScreenPosition{};
		Math::vec2 Size {};
		Math::vec4 BackgroundColor {1.0f};
		WidgetCounts WidgetCounts{};
		int32_t ParentIndex{ -1 };
		int32_t ChildBufferIndex{ -1 };
		uint32_t ChildBufferSize{ 0 };
		int32_t DefaultActiveWidget{ -1 };
		Ref<Widget> DefaultActiveWidgetRef { nullptr };
		std::vector<Ref<Widget>> Widgets {};

		void DisplayWindow();
		void HideWindow();
		bool GetWindowDisplayed();

	private:
		bool WindowDisplayed = false;

	public:
		void AddTextWidget(Ref<TextWidget> newWidget);
		void AddButtonWidget(Ref<ButtonWidget> newWidget);
		void AddCheckboxWidget(Ref<CheckboxWidget> newWidget);
		void AddComboWidget(Ref<ComboWidget> newWidget);
		void AddPopupWidget(Ref<PopupWidget> newWidget);
		void DeleteWidget(int32_t widgetLocation);
	private:
		void IncrementIterators(uint16_t iterator);
		void DecrementIterators(uint16_t iterator);
	};

	struct UIObject
	{
		std::vector<Window> Windows {};
		Ref<Font> m_Font = nullptr;
		Math::vec4 m_SelectColor {1.0f};
		Assets::AssetHandle m_FontHandle {0};
		UICallbacks m_FunctionPointers{};
	};


	class Runtime
	{
	public:
		static void Init();
		static void LoadUIObject(Ref<UIObject> uiObject, Assets::AssetHandle uiHandle);
		static void LoadUserInterfaceFromName(const std::string& uiName);
		static void Terminate();
		static bool SaveCurrentUIIntoUIObject();
		static void DeleteWindow(uint32_t windowLocation);
		static void PushRenderData(const Math::mat4& cameraViewMatrix, uint32_t viewportWidth = 0, uint32_t viewportHeight = 0);
		static void AddWindow(Window& window); // TODO: Remove this api, it is for testing!
		static void SetFont(Ref<Font> newFont, Assets::AssetHandle fontHandle);
		static std::vector<Window>& GetAllWindows();
		static void RefreshDisplayedWindows();
		static void ClearUIEngine();
		static Ref<UIObject> GetCurrentUIObject();
		static Assets::AssetHandle GetCurrentUIHandle();
		static void SetCurrentUIObject(Ref<UIObject> newUI);
		static void SetCurrentUIHandle(Assets::AssetHandle newHandle);
		static void SetSelectedWidgetColor(const Math::vec4& color);
		static void SetWidgetText(const std::string& windowTag, const std::string& widgetTag, const std::string& newText);

		static void SetSelectedWidget(const std::string& windowTag, const std::string& widgetTag);

		static void SetWidgetTextColor(const std::string& windowTag, const std::string& widgetTag, const Math::vec4& color);

		static void SetWidgetBackgroundColor(const std::string& windowTag, const std::string& widgetTag, const Math::vec4& color);

		static void SetWidgetSelectable(const std::string& windowTag, const std::string& widgetTag, bool selectable);

		static void SetFunctionOnMove(const std::string& function);
		static std::string GetFunctionOnMove();

		static void SetDisplayWindow(const std::string& windowTag, bool display);

		static void MoveRight();
		static void MoveLeft();
		static void MoveUp();
		static void MoveDown();
		static void OnPress();
	private:
		static void CalculateDirections();
	private:
		std::vector<Window*> m_DisplayedWindows{};
		Ref<UIObject> m_CurrentUI{ nullptr };
		Assets::AssetHandle m_CurrentUIHandle{0};
		std::vector<Window> m_Windows {};
		Widget* m_SelectedWidget { nullptr };
		Widget* m_HoveredWidget { nullptr };
		Window* m_ActiveWindow { nullptr };
		UICallbacks m_FunctionPointers{};

		Ref<Font> m_CurrentFont;
		Assets::AssetHandle m_FontHandle {0};
		Math::vec4 m_SelectColor {1.0f};

		static Runtime s_Engine;

		friend class TextWidget;

	public:
		// Editor API
		static int32_t& GetWindowToDelete();
		static int32_t& GetWidgetToDelete();
		static int32_t& GetWindowsToAddWidget();
		static RuntimeUI::WidgetTypes& GetWidgetTypeToAdd();
		static uint32_t& GetWindowToAdd();
		static int32_t& GetSelectedWindow();
		static int32_t& GetSelectedWidget();
		static Math::vec4& GetSelectColor();
	};
}

namespace Kargono::Utility
{
	static std::string WidgetTypeToString(RuntimeUI::WidgetTypes widgetType)
	{
		switch (widgetType)
		{
		case RuntimeUI::WidgetTypes::TextWidget: return "TextWidget";
		case RuntimeUI::WidgetTypes::ButtonWidget: return "ButtonWidget";
		case RuntimeUI::WidgetTypes::CheckboxWidget: return "CheckboxWidget";
		case RuntimeUI::WidgetTypes::ComboWidget: return "ComboWidget";
		case RuntimeUI::WidgetTypes::PopupWidget: return "PopupWidget";
		case RuntimeUI::WidgetTypes::None: return "None";
		default:
		{
			KG_ASSERT(false, "Invalid Widget Type at WidgetTypeToString");
			return "None";
		}
		}
	}

	static RuntimeUI::WidgetTypes StringToWidgetType(const std::string& widgetName)
	{
		if (widgetName == "TextWidget") { return RuntimeUI::WidgetTypes::TextWidget; }
		if (widgetName == "ButtonWidget") { return RuntimeUI::WidgetTypes::ButtonWidget; }
		if (widgetName == "CheckboxWidget") { return RuntimeUI::WidgetTypes::CheckboxWidget; }
		if (widgetName == "ComboWidget") { return RuntimeUI::WidgetTypes::ComboWidget; }
		if (widgetName == "PopupWidget") { return RuntimeUI::WidgetTypes::PopupWidget; }
		if (widgetName == "None") { return RuntimeUI::WidgetTypes::None; }

		KG_ASSERT(false, "Invalid Widget Type at StringToWidgetType");
		return RuntimeUI::WidgetTypes::None;
	}
}


