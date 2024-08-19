#pragma once

#include "Kargono/Rendering/EditorCamera.h"
#include "Kargono/Scripting/Scripting.h"
#include "Kargono/Core/Base.h"
#include "Kargono/RuntimeUI/Font.h"
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
		Assets::AssetHandle OnPressHandle { Assets::EmptyHandle };
		Ref<Scripting::Script> OnPress { nullptr };
	};

	struct UICallbacks
	{
		Assets::AssetHandle OnMoveHandle { Assets::EmptyHandle };
		Ref<Scripting::Script> OnMove { nullptr };
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

		std::string Text{ "New Text Widget" };
		float TextSize{ 0.12f };
		Math::vec2 TextAbsoluteDimensions {};
		Math::vec4 TextColor{0.5f};
		bool TextCentered = true;
		friend class RuntimeUIService;
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
		
	public:
		std::string Tag{ "None" };
		Math::vec3 ScreenPosition{};
		Math::vec2 Size {};
		Math::vec4 BackgroundColor {1.0f};
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
		void AddWidget(Ref<Widget> newWidget);
		void DeleteWidget(int32_t widgetLocation);
	};

	class UserInterface
	{
	public:
		//==============================
		// Internal Fields
		//==============================
		std::vector<Window> m_Windows {};
		Ref<Font> m_Font = nullptr;
		Math::vec4 m_SelectColor {1.0f};
		Assets::AssetHandle m_FontHandle {0};
		UICallbacks m_FunctionPointers{};

		std::vector<Window*> m_DisplayedWindows{};
		Widget* m_SelectedWidget{ nullptr };
		Widget* m_HoveredWidget{ nullptr };
		Window* m_ActiveWindow{ nullptr };
	};


	class RuntimeUIService
	{
	public:
		//==============================
		// Lifecycle Functions
		//==============================
		static void Init();
		static void Terminate();

		//==============================
		// Modify Active UI
		//==============================
		static void SetSelectedWidgetColor(const Math::vec4& color);
		static void SetActiveWidgetText(const std::string& windowTag, const std::string& widgetTag, const std::string& newText);
		static void SetActiveOnMove(Assets::AssetHandle functionHandle, Ref<Scripting::Script> function);
		static void SetActiveFont(Ref<Font> newFont, Assets::AssetHandle fontHandle);
		static void SetWidgetTextColor(const std::string& windowTag, const std::string& widgetTag, const Math::vec4& color);
		static void SetSelectedWidget(const std::string& windowTag, const std::string& widgetTag);
		static void SetWidgetBackgroundColor(const std::string& windowTag, const std::string& widgetTag, const Math::vec4& color);
		static void SetWidgetSelectable(const std::string& windowTag, const std::string& widgetTag, bool selectable);
		static void SetDisplayWindow(const std::string& windowTag, bool display);
		static Ref<Scripting::Script> GetActiveOnMove();
		static Assets::AssetHandle GetActiveOnMoveHandle();
		static std::vector<Window>& GetActiveWindows();
		static void AddActiveWindow(Window& window);
		static void DeleteActiveWindow(uint32_t windowLocation);

		//==============================
		// Interact With Active UI
		//==============================
		static void MoveRight();
		static void MoveLeft();
		static void MoveUp();
		static void MoveDown();
		static void OnPress();
	private:
		//==============================
		// Internal Functionality
		//==============================
		static void CalculateDirections();

		//==============================
		// Getters/Setters
		//==============================
	public:
		static void SetActiveUI(Ref<UserInterface> userInterface, Assets::AssetHandle uiHandle);
		static void SetActiveUIFromName(const std::string& uiName);
		static Ref<UserInterface> GetActiveUI();
		static Assets::AssetHandle GetActiveUIHandle();
		static void ClearActiveUI();

	public:
		//==============================
		// Other
		//==============================
		static void PushRenderData(const Math::mat4& cameraViewMatrix, uint32_t viewportWidth = 0, uint32_t viewportHeight = 0);
		static bool SaveCurrentUIIntoUIObject();
		static void RefreshDisplayedWindows();
	private:
		//==============================
		// Internal Fields
		//==============================
		static Ref<UserInterface> s_ActiveUI;
		static Assets::AssetHandle s_ActiveUIHandle;
	private:
		friend class TextWidget;
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
				KG_ERROR("Invalid Widget Type at WidgetTypeToString");
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

		KG_ERROR("Invalid Widget Type at StringToWidgetType");
		return RuntimeUI::WidgetTypes::None;
	}
}


