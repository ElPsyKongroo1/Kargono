#pragma once

#include "Kargono/Rendering/EditorPerspectiveCamera.h"
#include "Kargono/Scripting/ScriptService.h"
#include "Kargono/Core/Base.h"
#include "Kargono/RuntimeUI/Font.h"
#include "Kargono/Assets/Asset.h"
#include "Kargono/Math/Math.h"
#include "Kargono/Core/Directions.h"

#include <vector>
#include <string>
#include <unordered_map>
#include <functional>


namespace Kargono::Assets
{
	class UserInterfaceManager;
}

namespace Kargono::RuntimeUI
{
	class UserInterfaceService;
	struct RuntimeUIContext;
	class Window;


	//============================
	// Function Callbacks Struct
	//============================
	struct WidgetCallbacks
	{
		Assets::AssetHandle m_OnPressHandle { Assets::EmptyHandle };
		Ref<Scripting::Script> m_OnPress { nullptr };
	};

	struct UICallbacks
	{
		Assets::AssetHandle m_OnMoveHandle { Assets::EmptyHandle };
		Ref<Scripting::Script> m_OnMove { nullptr };
	};

	//============================
	// Widget Types Enum
	//============================
	enum class WidgetTypes
	{
		None = 0, TextWidget, ButtonWidget, CheckboxWidget, ComboWidget, PopupWidget
	};

	constexpr std::size_t k_InvalidWidgetIndex{ std::numeric_limits<std::size_t>().max() };

	//============================
	// Navigation Links Struct
	//============================
	struct NavigationLinks
	{
		std::size_t m_LeftWidgetIndex { k_InvalidWidgetIndex };
		std::size_t m_RightWidgetIndex { k_InvalidWidgetIndex };
		std::size_t m_UpWidgetIndex{ k_InvalidWidgetIndex };
		std::size_t m_DownWidgetIndex { k_InvalidWidgetIndex };
	};
	
	enum class RelativeOrAbsolute
	{
		Relative = 0,
		Absolute
	};

	enum class Constraint
	{
		None = 0,
		Top,
		Bottom,
		Left,
		Right,
		Center
	};

	enum class PixelOrPercent
	{
		Pixel = 0, 
		Percent
	};

	//============================
	// Widget Class (Base)
	//============================
	class Widget
	{
	public:
		//============================
		// Constructors/Destructors
		//============================
		virtual ~Widget() = default;
	public:
		//============================
		// Rendering Methods
		//============================
		virtual void OnRender(Math::vec3 translation, const Math::vec3& scale, float viewportWidth) = 0;

	public:
		//============================
		// Supporting Methods
		//============================
		Math::vec3 CalculateSize(const Math::vec3& windowSize);
		Math::vec3 CalculateWorldPosition(const Math::vec3& windowTranslation, const Math::vec3& windowSize);
		Math::vec3 CalculateWindowPosition(Math::vec2 worldPosition, const Math::vec3& windowTranslation, const Math::vec3& windowSize);
	public:
		//============================
		// Widget Data
		//============================
		std::string m_Tag{ "None" };
		PixelOrPercent m_XPositionType{ PixelOrPercent::Percent };
		PixelOrPercent m_YPositionType{ PixelOrPercent::Percent };
		RelativeOrAbsolute m_XRelativeOrAbsolute{ RelativeOrAbsolute::Absolute };
		RelativeOrAbsolute m_YRelativeOrAbsolute{ RelativeOrAbsolute::Absolute };
		Constraint m_XConstraint{ Constraint::None };
		Constraint m_YConstraint{ Constraint::None };
		Math::vec2 m_PercentPosition{ 0.4f };
		Math::ivec2 m_PixelPosition{ 0 };
		Math::vec2 m_Size  {0.3f};
		Math::vec4 m_DefaultBackgroundColor{0.5f};
		Math::vec4 m_ActiveBackgroundColor{0.5f};
		WidgetCallbacks m_FunctionPointers {};
		WidgetTypes m_WidgetType{ WidgetTypes::None };
		NavigationLinks m_NavigationLinks{};
		bool m_Selectable{ true };
	};

	//============================
	// Text Widget Class (Derived)
	//============================
	class TextWidget : public Widget
	{
	public:
		//============================
		// Constructors/Destructors
		//============================
		TextWidget()
			: Widget()
		{
			m_WidgetType = WidgetTypes::TextWidget;
			m_Size = { 0.3f, 0.1f };
			CalculateTextSize();
		}
		virtual ~TextWidget() override = default;
	public:
		//============================
		// Rendering Methods
		//============================
		virtual void OnRender(Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth) override;
	public:
		//============================
		// Modify State
		//============================
		void SetText(const std::string& newText, Window* parentWindow);

	public:
		//============================
		// Re-validation Methods
		//============================
		void CalculateTextSize();
		void CalculateTextMetadata(Window* parentWindow);
	public:
		//============================
		// Public Fields
		//============================
		std::string m_Text{ "New Text Widget" };
		float m_TextSize{ 0.3f };
		Math::vec4 m_TextColor{1.0f};
		Constraint m_TextAlignment{ Constraint::Center };
		bool m_TextWrapped{ true };
	private:
		TextMetadata m_TextMetadata{};
		Math::vec2 m_TextAbsoluteDimensions{};
	private:
		friend class RuntimeUIService;
		friend class Assets::UserInterfaceManager;
	};

	//============================
	// Button Widget Class (Derived)
	//============================
	class ButtonWidget : public Widget
	{
	public:
		//============================
		// Constructors/Destructors
		//============================
		ButtonWidget()
			: Widget()
		{
			m_WidgetType = WidgetTypes::ButtonWidget;
		}
		virtual ~ButtonWidget() override = default;
	public:
		//============================
		// Rendering Methods
		//============================
		virtual void OnRender(Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth) override;
	};

	//============================
	// Checkbox Widget Class (Derived)
	//============================
	class CheckboxWidget : public Widget
	{
	public:
		//============================
		// Constructors/Destructors
		//============================
		CheckboxWidget()
			: Widget()
		{
			m_WidgetType = WidgetTypes::CheckboxWidget;
		}
		virtual ~CheckboxWidget() override = default;
	public:
		//============================
		// Rendering Methods
		//============================
		virtual void OnRender(Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth) override;
	};

	//============================
	// Combo Widget Class (Derived)
	//============================
	class ComboWidget : public Widget
	{
	public:
		//============================
		// Constructors/Destructors
		//============================
		ComboWidget()
			: Widget()
		{
			m_WidgetType = WidgetTypes::ComboWidget;
		}
		virtual ~ComboWidget() override = default;
	public:
		//============================
		// Rendering Methods
		//============================
		virtual void OnRender(Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth) override;
	};

	//============================
	// Popup Widget Class (Derived)
	//============================
	class PopupWidget : public Widget
	{
	public:
		//============================
		// Constructors/Destructors
		//============================
		PopupWidget()
			: Widget()
		{
			m_WidgetType = WidgetTypes::PopupWidget;
		}
		virtual ~PopupWidget() override = default;
	public:
		//============================
		// Rendering Methods
		//============================
		virtual void OnRender(Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth) override;
	};

	//============================
	// Window Class
	//============================
	class Window
	{
	public:
		//============================
		// Interact w/ Window
		//============================
		void DisplayWindow();
		void HideWindow();
		
	public:
		//============================
		// Modify Window
		//============================
		void AddWidget(Ref<Widget> newWidget);
		void DeleteWidget(std::size_t widgetLocation);

	public:
		//============================
		// Query Window State
		//============================
		bool GetWindowDisplayed();

	public:
		//============================
		// Supporting Methods
		//============================
		Math::vec3 CalculateSize(uint32_t viewportWidth, uint32_t viewportHeight);
		Math::vec3 CalculateWorldPosition(uint32_t viewportWidth, uint32_t viewportHeight);
		Math::vec3 CalculateScreenPosition(Math::vec2 worldPosition, uint32_t viewportWidth, uint32_t viewportHeight);
	public:
		//============================
		// Public Fields
		//============================
		std::string m_Tag{ "None" };
		Math::vec3 m_ScreenPosition{};
		Math::vec2 m_Size{1.0f, 1.0f};
		Math::vec4 m_BackgroundColor{ 0.3f };
		std::size_t m_ParentIndex{ k_InvalidWidgetIndex };
		std::size_t m_ChildBufferIndex{ k_InvalidWidgetIndex };
		std::size_t m_ChildBufferSize{ 0 };
		std::size_t m_DefaultActiveWidget{ k_InvalidWidgetIndex };
		Ref<Widget> m_DefaultActiveWidgetRef{ nullptr };
		std::vector<Ref<Widget>> m_Widgets{};

	private:
		//============================
		// Internal Fields
		//============================
		bool m_WindowDisplayed{ false };
	};

	//============================
	// User Interface Class
	//============================
	class UserInterface
	{
	public:
		// Config data
		std::vector<Window> m_Windows {};
		Ref<Font> m_Font{ nullptr };
		Math::vec4 m_SelectColor {1.0f};
		Assets::AssetHandle m_FontHandle {0};
		UICallbacks m_FunctionPointers{};

		// Runtime Data
		std::vector<Window*> m_DisplayedWindows{};
		std::vector<size_t> m_DisplayedWindowIndices{};
		Widget* m_SelectedWidget{ nullptr };
		Widget* m_HoveredWidget{ nullptr };
		Window* m_ActiveWindow{ nullptr };
	};

	//============================
	// Runtime UI Service Class
	//============================
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
		static void AddActiveWindow(Window& window);
		static bool DeleteActiveUIWindow(std::size_t windowLocation);
		static bool DeleteActiveUIWidget(std::size_t windowIndex, std::size_t widgetIndex);

		//==============================
		// Modify Provided UI
		//==============================
		static bool DeleteUIWindow(Ref<UserInterface> userInterface, std::size_t windowLocation);
		static bool DeleteUIWidget(Ref<UserInterface> userInterface, std::size_t windowIndex, std::size_t widgetIndex);

		//==============================
		// Query Active UI
		//==============================
		static bool IsWidgetSelected(const std::string& windowTag, const std::string& widgetTag);
		static Ref<Scripting::Script> GetActiveOnMove();
		static Assets::AssetHandle GetActiveOnMoveHandle();
		static std::vector<Window>& GetAllActiveWindows();

		//==============================
		// Interact With Active UI
		//==============================
		static void MoveRight();
		static void MoveLeft();
		static void MoveUp();
		static void MoveDown();
		static void OnPress();

	public:
		//==============================
		// Getters/Setters
		//==============================
		static void SetActiveUI(Ref<UserInterface> userInterface, Assets::AssetHandle uiHandle);
		static void SetActiveUIFromHandle(Assets::AssetHandle uiHandle);
		static Ref<UserInterface> GetActiveUI();
		static Assets::AssetHandle GetActiveUIHandle();
		static void ClearActiveUI();

	public:
		//==============================
		// Rendering API
		//==============================
		static void OnRender(const Math::mat4& cameraViewMatrix, uint32_t viewportWidth, uint32_t viewportHeight);
		static void OnRender(uint32_t viewportWidth, uint32_t viewportHeight);
	private:
		//==============================
		// Internal Functionality
		//==============================
		static void CalculateWindowNavigationLinks();
		static std::size_t CalculateNavigationLink(Window& window, Ref<Widget> currentWidget, Direction direction, const Math::vec3& windowPosition, const Math::vec3& windowSize);
		static Ref<Widget> GetWidget(const std::string& windowTag, const std::string& widgetTag);
		static void RevalidateDisplayedWindows();

	private:
		//==============================
		// Internal Fields
		//==============================
		static inline Ref<RuntimeUIContext> s_RuntimeUIContext{ nullptr };
	private:
		friend class TextWidget;
		friend class Window;
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

	static std::string PixelOrPercentToString(RuntimeUI::PixelOrPercent type)
	{
		switch (type)
		{
			case RuntimeUI::PixelOrPercent::Pixel: return "Pixel";
			case RuntimeUI::PixelOrPercent::Percent: return "Percent";
			default:
			{
				KG_ERROR("Invalid PixelOrPercent at PixelOrPercentToString");
				return "Pixel";
			}
		}
	}

	static RuntimeUI::PixelOrPercent StringToPixelOrPercent(const std::string& type)
	{
		if (type == "Pixel") { return RuntimeUI::PixelOrPercent::Pixel; }
		if (type == "Percent") { return RuntimeUI::PixelOrPercent::Percent; }

		KG_ERROR("Invalid PixelOrPercent at StringToPixelOrPercent");
		return RuntimeUI::PixelOrPercent::Pixel;
	}

	static std::string RelativeOrAbsoluteToString(RuntimeUI::RelativeOrAbsolute type)
	{
		switch (type)
		{
		case RuntimeUI::RelativeOrAbsolute::Relative: return "Relative";
		case RuntimeUI::RelativeOrAbsolute::Absolute: return "Absolute";
		default:
		{
			KG_ERROR("Invalid RelativeOrAbsolute at RelativeOrAbsoluteToString");
			return "Absolute";
		}
		}
	}

	static RuntimeUI::RelativeOrAbsolute StringToRelativeOrAbsolute(const std::string& type)
	{
		if (type == "Relative") { return RuntimeUI::RelativeOrAbsolute::Relative; }
		if (type == "Absolute") { return RuntimeUI::RelativeOrAbsolute::Absolute; }

		KG_ERROR("Invalid RelativeOrAbsolute at StringToRelativeOrAbsolute");
		return RuntimeUI::RelativeOrAbsolute::Absolute;
	}

	static std::string ConstraintToString(RuntimeUI::Constraint type)
	{
		switch (type)
		{
		case RuntimeUI::Constraint::None: return "None";
		case RuntimeUI::Constraint::Top: return "Top";
		case RuntimeUI::Constraint::Bottom: return "Bottom";
		case RuntimeUI::Constraint::Left: return "Left";
		case RuntimeUI::Constraint::Right: return "Right";
		case RuntimeUI::Constraint::Center: return "Center";
		default:
		{
			KG_ERROR("Invalid Constraint at ConstraintToString");
			return "None";
		}
		}
	}

	static RuntimeUI::Constraint StringToConstraint(const std::string& type)
	{
		if (type == "None") { return RuntimeUI::Constraint::None; }
		if (type == "Top") { return RuntimeUI::Constraint::Top; }
		if (type == "Bottom") { return RuntimeUI::Constraint::Bottom; }
		if (type == "Left") { return RuntimeUI::Constraint::Left; }
		if (type == "Right") { return RuntimeUI::Constraint::Right; }
		if (type == "Center") { return RuntimeUI::Constraint::Center; }

		KG_ERROR("Invalid Constraint at StringToConstraint");
		return RuntimeUI::Constraint::None;
	}
}


