#pragma once

#include "Kargono/Rendering/EditorPerspectiveCamera.h"
#include "Kargono/Scripting/ScriptService.h"
#include "Kargono/Core/Base.h"
#include "Kargono/RuntimeUI/Font.h"
#include "Kargono/Assets/Asset.h"
#include "Kargono/Math/Math.h"
#include "Kargono/Core/Directions.h"
#include "Kargono/RuntimeUI/RuntimeUICommon.h"
#include "Kargono/Core/Window.h"

#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include <tuple>

namespace Kargono::Assets
{
	class UserInterfaceManager;
}

namespace Kargono::Rendering
{
	class Texture2D;
}

namespace Kargono::RuntimeUI
{
	class RuntimeUIService;
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

	struct UserInterfaceCallbacks
	{
		Assets::AssetHandle m_OnMoveHandle { Assets::EmptyHandle };
		Ref<Scripting::Script> m_OnMove { nullptr };
		Assets::AssetHandle m_OnHoverHandle{ Assets::EmptyHandle };
		Ref<Scripting::Script> m_OnHover{ nullptr };
	};

	//============================
	// Widget Types Enum
	//============================
	enum class WidgetTypes
	{
		None = 0, TextWidget, ButtonWidget, CheckboxWidget, 
		ComboWidget, PopupWidget, ImageWidget, ImageButtonWidget,
		InputTextWidget, SliderWidget
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

	struct SelectionData
	{
		Math::vec4 m_DefaultBackgroundColor{ 0.5f };
		WidgetCallbacks m_FunctionPointers{};
		bool m_Selectable{ true };

		// Runtime calculated data
		NavigationLinks m_NavigationLinks;
	};

	struct ImageData
	{
		Ref<Rendering::Texture2D> m_ImageRef{ nullptr };
		Assets::AssetHandle m_ImageHandle{ Assets::EmptyHandle };
		bool m_FixedAspectRatio{ false };
	};

	struct SingleLineTextData
	{
		std::string m_Text{ "New Widget" };
		float m_TextSize{ 0.3f };
		Math::vec4 m_TextColor{ 1.0f };
		Constraint m_TextAlignment{ Constraint::Center };

		// Runtime calculated data
		Math::vec2 m_CachedTextDimensions{};
		size_t m_CursorIndex{ 0 };
	};

	struct MultiLineTextData
	{
		std::string m_Text{ "New Text Widget" };
		float m_TextSize{ 0.3f };
		Math::vec4 m_TextColor{ 1.0f };
		Constraint m_TextAlignment{ Constraint::Center };
		bool m_TextWrapped{ false };

		// Runtime calculated data
		MultiLineTextDimensions m_CachedTextDimensions{};
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
		virtual bool Selectable() = 0;
	public:
		//============================
		// Supporting Methods
		//============================
		Math::vec3 CalculateWidgetSize(const Math::vec3& windowSize);
		Math::vec3 CalculateWorldPosition(const Math::vec3& windowTranslation, const Math::vec3& windowSize);
		Math::vec3 CalculateWindowPosition(Math::vec2 worldPosition, const Math::vec3& windowTranslation, const Math::vec3& windowSize);
	public:
		//============================
		// Widget Data
		//============================
		std::string m_Tag{ "None" };
		PixelOrPercent m_SizeType{ PixelOrPercent::Percent };
		PixelOrPercent m_XPositionType{ PixelOrPercent::Percent };
		PixelOrPercent m_YPositionType{ PixelOrPercent::Percent };
		RelativeOrAbsolute m_XRelativeOrAbsolute{ RelativeOrAbsolute::Absolute };
		RelativeOrAbsolute m_YRelativeOrAbsolute{ RelativeOrAbsolute::Absolute };
		Constraint m_XConstraint{ Constraint::None };
		Constraint m_YConstraint{ Constraint::None };
		Math::vec2 m_PercentPosition{ 0.4f };
		Math::ivec2 m_PixelPosition{ 0 };
		Math::vec2 m_PercentSize  { 0.3f, 0.1f };
		Math::ivec2 m_PixelSize  { 50 };
		WidgetTypes m_WidgetType{ WidgetTypes::None };
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

		//============================
		// Query State
		//============================
		virtual bool Selectable() override
		{
			return false;
		}

	public:
		//============================
		// Re-validation Methods
		//============================
		void CalculateTextSize(Window* parentWindow);
	public:
		//============================
		// Public Fields
		//============================
		MultiLineTextData m_TextData;
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

	public:
		//============================
		// Modify State
		//============================
		void SetText(const std::string& newText);

		//============================
		// Query State
		//============================
		virtual bool Selectable() override
		{
			return m_SelectionData.m_Selectable;
		}

	public:
		//============================
		// Re-validation Methods
		//============================
		void CalculateTextSize();
	public:
		//============================
		// Public Fields
		//============================
		SingleLineTextData m_TextData;
		SelectionData m_SelectionData;
	private:
		friend class RuntimeUIService;
		friend class Assets::UserInterfaceManager;
	};

	//============================
	// ImageButton Widget Class (Derived)
	//============================
	class ImageButtonWidget : public Widget
	{
	public:
		//============================
		// Constructors/Destructors
		//============================
		ImageButtonWidget()
			: Widget()
		{
			m_WidgetType = WidgetTypes::ImageButtonWidget;
		}
		virtual ~ImageButtonWidget() override = default;
	public:
		//============================
		// Rendering Methods
		//============================
		virtual void OnRender(Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth) override;

	public:
		//============================
		// Query State
		//============================
		virtual bool Selectable() override
		{
			return m_SelectionData.m_Selectable;
		}

	public:
		//============================
		// Public Fields
		//============================
		ImageData m_ImageData;
		SelectionData m_SelectionData;
	private:
		friend class RuntimeUIService;
		friend class Assets::UserInterfaceManager;
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
		virtual bool Selectable()
		{
			return m_SelectionData.m_Selectable;
		}
		//============================
		// Public Fields
		//============================
		ImageData m_ImageChecked;
		ImageData m_ImageUnChecked;
		SelectionData m_SelectionData;
		bool m_Checked{ false };
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
	// Image Widget Class (Derived)
	//============================
	class ImageWidget : public Widget
	{
	public:
		//============================
		// Constructors/Destructors
		//============================
		ImageWidget()
			: Widget()
		{
			m_WidgetType = WidgetTypes::ImageWidget;
		}
		virtual ~ImageWidget() override = default;

	public:
		//============================
		// Query State
		//============================
		virtual bool Selectable() override
		{
			return false;
		}

	public:
		//============================
		// Rendering Methods
		//============================
		virtual void OnRender(Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth) override;

	public:
		//============================
		// Public Fields
		//============================
		ImageData m_ImageData;
	};

	//============================
	// InputText Widget Class (Derived)
	//============================
	class InputTextWidget : public Widget
	{
	public:
		//============================
		// Constructors/Destructors
		//============================
		InputTextWidget()
			: Widget()
		{
			m_WidgetType = WidgetTypes::InputTextWidget;
		}
		virtual ~InputTextWidget() override = default;

	public:
		//============================
		// Query State
		//============================
		virtual bool Selectable() override
		{
			return m_SelectionData.m_Selectable;
		}
		//============================
		// Modify State
		//============================
		void SetText(const std::string& newText);

	public:
		//============================
		// Re-validation Methods
		//============================
		void CalculateTextSize();
	public:
		//============================
		// Rendering Methods
		//============================
		virtual void OnRender(Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth) override;

	public:
		//============================
		// Public Fields
		//============================
		SingleLineTextData m_TextData;
		SelectionData m_SelectionData;
		Assets::AssetHandle m_OnMoveCursorHandle{ Assets::EmptyHandle };
		Ref<Scripting::Script> m_OnMoveCursor{ nullptr };
		
	};

	//============================
	// InputText Widget Class (Derived)
	//============================
	class SliderWidget : public Widget
	{
	public:
		//============================
		// Constructors/Destructors
		//============================
		SliderWidget()
			: Widget()
		{
			m_WidgetType = WidgetTypes::SliderWidget;
		}
		virtual ~SliderWidget() override = default;

	public:
		//============================
		// Query State
		//============================
		virtual bool Selectable() override
		{
			return m_SelectionData.m_Selectable;
		}

	public:
		//============================
		// Rendering Methods
		//============================
		virtual void OnRender(Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth) override;

	public:
		//============================
		// Public Fields
		//============================
		SelectionData m_SelectionData;
		Math::vec2 m_Bounds{ 0.0f, 1.0f };
		Assets::AssetHandle m_OnMoveSliderHandle{ Assets::EmptyHandle };
		Ref<Scripting::Script> m_OnMoveSlider{ nullptr };
		Math::vec4 m_SliderColor{ 1.0f };
		Math::vec4 m_LineColor{ 1.0f };

		// Runtime Value
		float m_CurrentValue{ 0.0f };

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
		Math::vec4 m_HoveredColor{ 0.5f };
		Math::vec4 m_EditingColor{ 0.15f, 0.15f, 0.15f, 1.0f };
		Assets::AssetHandle m_FontHandle {0};
		UserInterfaceCallbacks m_FunctionPointers{};

		// Runtime Data
		bool m_IBeamVisible{ true };
		float m_IBeamAccumulator{ 0.0f };
		float m_IBeamVisiblilityInterval{ 0.75f };
		std::vector<Window*> m_DisplayedWindows{};
		std::vector<size_t> m_DisplayedWindowIndices{};
		Widget* m_SelectedWidget{ nullptr };
		Widget* m_HoveredWidget{ nullptr };
		Widget* m_EditingWidget{ nullptr };
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
		// On Event Functions
		//==============================
		static void OnUpdate(Timestep ts);
		static bool OnKeyTypedEvent(Events::KeyTypedEvent event);
		static bool OnKeyPressedEvent(Events::KeyPressedEvent event);
		static void OnLeftMouseButtonPressed(Math::vec2 mousePosition, ViewportData* viewportData);

		//==============================
		// Modify Active UI
		//==============================
		static void SetSelectedWidgetColor(const Math::vec4& color);
		static void SetActiveWidgetTextByTag(const std::string& windowTag, const std::string& widgetTag, const std::string& newText);
		static void SetActiveWidgetTextByIndex(WidgetID widgetID, const std::string& newText);
		static void SetWidgetImageByIndex(WidgetID widgetID, Assets::AssetHandle textureHandle);
		static void SetActiveOnMove(Assets::AssetHandle functionHandle, Ref<Scripting::Script> function);
		static void SetActiveOnHover(Assets::AssetHandle functionHandle, Ref<Scripting::Script> function);
		static void SetActiveFont(Ref<Font> newFont, Assets::AssetHandle fontHandle);
		static void SetWidgetTextColorByTag(const std::string& windowTag, const std::string& widgetTag, const Math::vec4& color);
		static void SetWidgetTextColorByIndex(WidgetID widgetID, const Math::vec4& color);
		static void SetSelectedWidgetByTag(const std::string& windowTag, const std::string& widgetTag);
		static void SetSelectedWidgetByIndex(WidgetID widgetID);
		static void SetEditingWidgetByIndex(WidgetID widgetID);
		static void SetHoveredWidgetByIndex(WidgetID widgetID);
		static void ClearHoveredWidget();
		static void ClearEditingWidget();
		static void SetWidgetBackgroundColorByTag(const std::string& windowTag, const std::string& widgetTag, const Math::vec4& color);
		static void SetWidgetBackgroundColorByIndex(WidgetID widgetID, const Math::vec4& color);
		static void SetWidgetSelectableByTag(const std::string& windowTag, const std::string& widgetTag, bool selectable);
		static void SetWidgetSelectableByIndex(WidgetID widgetID, bool selectable);
		static void SetDisplayWindowByTag(const std::string& windowTag, bool display);
		static void SetDisplayWindowByIndex(WindowID widgetID, bool display);
		static void AddActiveWindow(Window& window);
		static bool DeleteActiveUIWindow(std::size_t windowLocation);
		static bool DeleteActiveUIWidget(std::size_t windowIndex, std::size_t widgetIndex);

		//==============================
		// Modify Indicated UI
		//==============================
		static bool DeleteUIWindow(Ref<UserInterface> userInterface, std::size_t windowLocation);
		static bool DeleteUIWidget(Ref<UserInterface> userInterface, std::size_t windowIndex, std::size_t widgetIndex);

		//==============================
		// Query Active UI
		//==============================
		static const std::string& GetWidgetTextByIndex(WidgetID widgetID);
		static bool IsWidgetSelectedByTag(const std::string& windowTag, const std::string& widgetTag);
		static bool IsWidgetSelectedByIndex(WidgetID widgetID);
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
		static void OnPressByIndex(WidgetID widgetID);

		//==============================
		// Revalidate UI Context
		//==============================
		static void CalculateWindowNavigationLinks();
		static void RecalculateTextData(Window* parentWindow, Widget* widget);
		static void CalculateFixedAspectRatioSize(Window* parentWindow, Widget* widget, uint32_t viewportWidth, uint32_t viewportHeight,
			bool useXValueAsBase);
		static SelectionData* GetSelectionDataFromWidget(Widget* currentWidget);
		static ImageData* GetImageDataFromWidget(Widget* currentWidget);
		static SingleLineTextData* GetSingleLineTextDataFromWidget(Widget* currentWidget);
		static MultiLineTextData* GetMultiLineTextDataFromWidget(Widget* currentWidget);

	public:
		//==============================
		// Getters/Setters
		//==============================
		static void SetActiveUI(Ref<UserInterface> userInterface, Assets::AssetHandle uiHandle);
		static void SetActiveUIFromHandle(Assets::AssetHandle uiHandle);
		static Ref<UserInterface> GetActiveUI();
		static Assets::AssetHandle GetActiveUIHandle();
		static void ClearActiveUI();
		static Ref<Widget> GetWidget(const std::string& windowTag, const std::string& widgetTag);
		static Ref<Widget> GetWidget(uint16_t windowIndex, uint16_t widgetIndex);
		static std::tuple<Ref<Widget>, Window*> GetWidgetAndWindow(const std::string& windowTag, const std::string& widgetTag);
		static std::tuple<Ref<Widget>, Window*> GetWidgetAndWindow(uint16_t windowIndex, uint16_t widgetIndex);

	public:
		//==============================
		// Rendering API
		//==============================
		static void OnRender(const Math::mat4& cameraViewMatrix, uint32_t viewportWidth, uint32_t viewportHeight);
		static void OnRender(uint32_t viewportWidth, uint32_t viewportHeight);
	private:
		//==============================
		// Revalidate UI Context (Internal)
		//==============================
		static std::size_t CalculateNavigationLink(Window& window, Ref<Widget> currentWidget, Direction direction, const Math::vec3& windowPosition, const Math::vec3& windowSize);
		static void RevalidateDisplayedWindows();
		static void CalculateSingleLineText(SingleLineTextData& textData);
		static Math::vec2 CalculateSingleLineText(std::string_view textData);
		static size_t CalculateCursorIndexFromMousePosition(SingleLineTextData& textData, float textStartingPosition, float mouseXPosition, float textScalingFactor);
		static void CalculateMultiLineText(MultiLineTextData& textData, const Math::vec3& widgetSize, float textSize);

		//==============================
		// Manage Active UI (Internal)
		//==============================
		static void SetWidgetTextInternal(Window* currentWindow, Ref<Widget> currentWidget, const std::string& newText);
		static void SetSelectedWidgetInternal(Ref<Widget> newSelectedWidget);
		static void SetHoveredWidgetInternal(Ref<Widget> newSelectedWidget);
		static void SetWidgetTextColorInternal(Ref<Widget> currentWidget, const Math::vec4& newColor);
		static void SetWidgetSelectableInternal(Ref<Widget> currentWidget, bool selectable);
		static bool IsWidgetSelectedInternal(Ref<Widget> currentWidget);
		static void SetWidgetBackgroundColorInternal(Ref<Widget> currentWidget, const Math::vec4& newColor);

		//==============================
		// Interact With Active UI (Internal)
		//==============================
		static void OnPressInternal(Widget* currentWidget);
		static void OnMoveCursorInternal(Widget* currentWidget);

		//==============================
		// Rendering API (Internal)
		//==============================
		static void RenderBackground(const Math::vec4& color, const Math::vec3& translation, const Math::vec3 size);
		static void RenderImage(const ImageData& imageData, const Math::vec3& translation, const Math::vec3 size);
		static void RenderSingleLineText(const SingleLineTextData& textData, const Math::vec3& textStartingPoint, float textScalingFactor);
		static void RenderTextCursor(const SingleLineTextData& textData, const Math::vec3& renderLocation, float textScalingFactor);
		static void RenderSliderLine(const Math::vec4& color, const Math::vec3& widgetTranslation, const Math::vec3& widgetSize);
		static Math::vec3 GetSingleLineTextStartingPosition(const SingleLineTextData& textData, const Math::vec3& translation, const Math::vec3 size, float textScalingFactor);


	private:
		//==============================
		// Internal Fields
		//==============================
		static inline Ref<RuntimeUIContext> s_RuntimeUIContext{ nullptr };
	private:
		friend class TextWidget;
		friend class ButtonWidget;
		friend class ImageWidget;
		friend class ImageButtonWidget;
		friend class CheckboxWidget;
		friend class InputTextWidget;
		friend class SliderWidget;
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
			case RuntimeUI::WidgetTypes::ImageWidget: return "ImageWidget";
			case RuntimeUI::WidgetTypes::ImageButtonWidget: return "ImageButtonWidget";
			case RuntimeUI::WidgetTypes::InputTextWidget: return "InputTextWidget";
			case RuntimeUI::WidgetTypes::SliderWidget: return "SliderWidget";
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
		if (widgetName == "ImageWidget") { return RuntimeUI::WidgetTypes::ImageWidget; }
		if (widgetName == "ImageButtonWidget") { return RuntimeUI::WidgetTypes::ImageButtonWidget; }
		if (widgetName == "InputTextWidget") { return RuntimeUI::WidgetTypes::InputTextWidget; }
		if (widgetName == "SliderWidget") { return RuntimeUI::WidgetTypes::SliderWidget; }
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
