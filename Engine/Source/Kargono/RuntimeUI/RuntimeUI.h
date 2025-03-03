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
#include "Kargono/Utility/Random.h"

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
	class Widget;

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
		None = 0, TextWidget, ButtonWidget, CheckboxWidget, ContainerWidget,
		DropDownWidget, ImageWidget, ImageButtonWidget,
		InputTextWidget, SliderWidget, VerticalContainerWidget, HorizontalContainerWidget
	};

	//============================
	// Navigation Links Struct
	//============================
	struct NavigationLinks
	{
		int32_t m_LeftWidgetID { k_InvalidWidgetID };
		int32_t m_RightWidgetID { k_InvalidWidgetID };
		int32_t m_UpWidgetID{ k_InvalidWidgetID };
		int32_t m_DownWidgetID { k_InvalidWidgetID };
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

	struct ContainerData
	{
		std::vector<Ref<Widget>> m_ContainedWidgets;
		Math::vec4 m_BackgroundColor{ 0.5f };
	};

	struct ImageData
	{
		Ref<Rendering::Texture2D> m_ImageRef{ nullptr };
		Assets::AssetHandle m_ImageHandle{ Assets::EmptyHandle };
		bool m_FixedAspectRatio{ false };
	};

	struct SingleLineTextData
	{
		std::string m_Text{ "..." };
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
		Widget()
		{
			m_ID = Utility::RandomService::GenerateRandomInteger((int32_t)0, k_InvalidWidgetID - 1);
		}
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
		int32_t m_ID{ k_InvalidWidgetID };
		PixelOrPercent m_SizeType{ PixelOrPercent::Percent };
		PixelOrPercent m_XPositionType{ PixelOrPercent::Percent };
		PixelOrPercent m_YPositionType{ PixelOrPercent::Percent };
		RelativeOrAbsolute m_XRelativeOrAbsolute{ RelativeOrAbsolute::Absolute };
		RelativeOrAbsolute m_YRelativeOrAbsolute{ RelativeOrAbsolute::Absolute };
		Constraint m_XConstraint{ Constraint::None };
		Constraint m_YConstraint{ Constraint::None };
		Math::vec2 m_PercentPosition{ 0.0f };
		Math::ivec2 m_PixelPosition{ 0 };
		Math::vec2 m_PercentSize  { 0.2f, 0.2f };
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
			m_Tag = "TextWidget";
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
		void SetText(const std::string& newText);

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
		void CalculateTextSize();
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
			m_Tag = "ButtonWidget";
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
			m_Tag = "ImageButtonWidget";
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
			m_Tag = "CheckboxWidget";
		}
		virtual ~CheckboxWidget() override = default;
	public:
		//============================
		// Rendering Methods
		//============================
		virtual void OnRender(Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth) override;

		//============================
		// Query State
		//============================
		virtual bool Selectable() override
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
	// Container Widget Class (Derived)
	//============================
	class ContainerWidget : public Widget
	{
	public:
		//============================
		// Constructors/Destructors
		//============================
		ContainerWidget()
			: Widget()
		{
			m_WidgetType = WidgetTypes::ContainerWidget;
			m_Tag = "FrameContainerWidget";
		}
		virtual ~ContainerWidget() override = default;
	public:
		//============================
		// Rendering Methods
		//============================
		virtual void OnRender(Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth) override;

		//============================
		// Query State
		//============================
		virtual bool Selectable() override
		{
			return false;
		}

		//============================
		// Public Fields
		//============================
		ContainerData m_ContainerData;
	};

	//============================
	// HorizontalContainer Widget Class (Derived)
	//============================
	class HorizontalContainerWidget : public Widget
	{
	public:
		//============================
		// Constructors/Destructors
		//============================
		HorizontalContainerWidget()
			: Widget()
		{
			m_WidgetType = WidgetTypes::HorizontalContainerWidget;
			m_Tag = "HorizontalContainerWidget";
		}
		virtual ~HorizontalContainerWidget() override = default;
	public:
		//============================
		// Rendering Methods
		//============================
		virtual void OnRender(Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth) override;

		//============================
		// Query State
		//============================
		virtual bool Selectable() override
		{
			return false;
		}

		//============================
		// Public Fields
		//============================
		ContainerData m_ContainerData;
		float m_ColumnWidth{ 0.25f };
		float m_ColumnSpacing{ 0.0f };
	};

	//============================
	// VerticalContainer Widget Class (Derived)
	//============================
	class VerticalContainerWidget : public Widget
	{
	public:
		//============================
		// Constructors/Destructors
		//============================
		VerticalContainerWidget()
			: Widget()
		{
			m_WidgetType = WidgetTypes::VerticalContainerWidget;
			m_Tag = "VerticalContainerWidget";
		}
		virtual ~VerticalContainerWidget() override = default;
	public:
		//============================
		// Rendering Methods
		//============================
		virtual void OnRender(Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth) override;

		//============================
		// Query State
		//============================
		virtual bool Selectable() override
		{
			return false;
		}

		//============================
		// Public Fields
		//============================
		ContainerData m_ContainerData;
		float m_RowHeight{ 0.25f };
		float m_RowSpacing{ 0.0f };
	};

	//============================
	// Drop Down Widget Class (Derived)
	//============================
	class DropDownWidget : public Widget
	{
	public:
		//============================
		// Constructors/Destructors
		//============================
		DropDownWidget()
			: Widget()
		{
			m_WidgetType = WidgetTypes::DropDownWidget;
			m_Tag = "DropDownWidget";
		}
		virtual ~DropDownWidget() override = default;
	public:
		//============================
		// Rendering Methods
		//============================
		virtual void OnRender(Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth) override;

		//============================
		// Query State
		//============================
		virtual bool Selectable() override
		{
			return m_SelectionData.m_Selectable;
		}

		//============================
		// Re-validation Methods
		//============================
		void CalculateTextSize();
	public:
		//============================
		// Public Fields
		//============================
		SelectionData m_SelectionData{};
		std::vector<SingleLineTextData> m_DropDownOptions{};
		Math::vec4 m_DropDownBackground{ 1.0f };
		Assets::AssetHandle m_OnSelectOptionHandle{ Assets::EmptyHandle };
		Ref<Scripting::Script> m_OnSelectOption{ nullptr };
		
		// Runtime Values
		size_t m_CurrentOption{ 0 };
		bool m_DropDownOpen{ false };
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
			m_Tag = "ImageWidget";
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
			m_Tag = "InputTextWidget";
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
			m_Tag = "SliderWidget";
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
		// Constructors/Destructors
		//============================
		Window()
		{
			m_ID = Utility::RandomService::GenerateRandomInteger((int32_t)0, k_InvalidWindowID - 1);
		}

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
		std::vector<Ref<Widget>> GetAllChildWidgets();

	private:
		void GetChildWidget(std::vector<Ref<Widget>>& returnVector, Ref<Widget> currentWidget);

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
		int32_t m_ID{ k_InvalidWindowID };
		Math::vec3 m_ScreenPosition{};
		Math::vec2 m_Size{1.0f, 1.0f};
		Math::vec4 m_BackgroundColor{ 0.3f };
		int32_t m_DefaultActiveWidget{ k_InvalidWidgetID };
		Ref<Widget> m_DefaultActiveWidgetRef{ nullptr };
		std::vector<Ref<Widget>> m_Widgets{};

	private:
		//============================
		// Internal Fields
		//============================
		bool m_WindowDisplayed{ false };
	};

	using IDToLocationMap = std::unordered_map<int32_t, std::vector<uint16_t>>;

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
		Widget* m_PressedWidget{ nullptr };
		Window* m_ActiveWindow{ nullptr };
		IDToLocationMap m_IDToLocation{};
	};

	struct BoundingBoxTransform
	{
		Math::vec3 m_Translation;
		Math::vec3 m_Size;
	};

	class NavigationLinksCalculator
	{
	public:
		void CalculateNavigationLinks(Ref<UserInterface> userInterface, ViewportData& viewportData);
	private:
		void CalculateWidgetNavigationLinks(Ref<Widget> currentWidget);
		int32_t CalculateNavigationLink(Direction direction);
		void CompareCurrentAndPotentialWidget(Ref<Widget> potentialWidget);
	private:
		// General state
		Ref<UserInterface> m_UserInterface{ nullptr };
		ViewportData m_ViewportData{};
		// Current window state
		Window* m_CurrentWindow{ nullptr };
		BoundingBoxTransform m_CurrentWindowTransform{};
		// Current widget state
		Ref<Widget> m_CurrentWidget{ nullptr };
		BoundingBoxTransform m_CurrentWidgetParentTransform{};
		Math::vec2 m_CurrentWidgetPosition{};
		Math::vec2 m_CurrentWidgetSize{};
		Math::vec2 m_CurrentWidgetCenterPosition{};
		// Current potential widget info
		BoundingBoxTransform m_PotentialWidgetParentTransform{};

		// Nav-link calculation fields
		int32_t m_CurrentBestChoiceID{ k_InvalidWidgetID };
		float m_CurrentBestDistance{ std::numeric_limits<float>::max() };
		Direction m_CurrentDirection{ Direction::None };
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
		static void OnMouseButtonPressedEvent(const Events::MouseButtonPressedEvent& event);
		static void OnMouseButtonReleasedEvent(const Events::MouseButtonReleasedEvent& mouseEvent);

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
		static void ClearSelectedWidget();
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
		static bool DeleteActiveUIWindow(int32_t windowID);
		static bool DeleteActiveUIWidget(int32_t widgetID);
		static void AddWidgetToContainer(ContainerData* container, Ref<Widget> newWidget);

		//==============================
		// Modify Indicated UI
		//==============================
		static bool DeleteUIWindow(Ref<UserInterface> userInterface, std::size_t windowLocation);
		static bool DeleteUIWidget(Ref<UserInterface> userInterface, int32_t widgetID);

		//==============================
		// Query Active UI
		//==============================
		static const std::string& GetWidgetTextByIndex(WidgetID widgetID);
		static bool IsWidgetSelectedByTag(const std::string& windowTag, const std::string& widgetTag);
		static bool IsWidgetSelectedByIndex(WidgetID widgetID);
		static Ref<Scripting::Script> GetActiveOnMove();
		static Assets::AssetHandle GetActiveOnMoveHandle();
		static std::vector<Window>& GetAllActiveWindows();
		static BoundingBoxTransform GetParentDimensionsFromID(int32_t widgetID, uint32_t viewportWidth, uint32_t viewportHeight);
		static BoundingBoxTransform GetWidgetDimensionsFromID(int32_t widgetID, uint32_t viewportWidth, uint32_t viewportHeight);

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
		static void RecalculateTextData(Widget* widget);
		static void CalculateFixedAspectRatioSize(Widget* widget, uint32_t viewportWidth, uint32_t viewportHeight,
			bool useXValueAsBase);
		static SelectionData* GetSelectionDataFromWidget(Widget* currentWidget);
		static ContainerData* GetContainerDataFromWidget(Widget* currentWidget);
		static ImageData* GetImageDataFromWidget(Widget* currentWidget);
		static SingleLineTextData* GetSingleLineTextDataFromWidget(Widget* currentWidget);
		static MultiLineTextData* GetMultiLineTextDataFromWidget(Widget* currentWidget);

	public:
		//==============================
		// Getters/Setters
		//==============================
		static void SetActiveUI(Ref<UserInterface> userInterface, Assets::AssetHandle uiHandle);
		static void SetActiveUIFromHandle(Assets::AssetHandle uiHandle);
		static bool IsUIActiveFromHandle(Assets::AssetHandle uiHandle);
		static Ref<UserInterface> GetActiveUI();
		static Assets::AssetHandle GetActiveUIHandle();
		static void ClearActiveUI();
		static Ref<Widget> GetWidgetFromTag(const std::string& windowTag, const std::string& widgetTag);
		static Ref<Widget> GetWidgetFromID(int32_t widgetID);
		static Ref<Widget> GetWidgetFromDirections(const std::vector<uint16_t>& locationDirections);
		static IDType CheckIDType(int32_t windowOrWidgetID);
		static Window& GetWindowFromID(int32_t windowID);
		static Window& GetParentWindowFromWidgetID(int32_t widgetID);
		static Ref<Widget> GetParentWidgetFromID(int32_t widgetID);
		static std::vector<uint16_t>* GetLocationFromID(int32_t windowOrWidgetID);
		static std::tuple<Ref<Widget>, Window*> GetWidgetAndWindow(const std::string& windowTag, const std::string& widgetTag);
		static std::tuple<Ref<Widget>, Window*> GetWidgetAndWindow(int32_t widgetID);

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
		static void RevalidateDisplayedWindows();
		static void RevalidateWidgetIDToLocationMap();
		static void RevalidateContainerInLocationMap(IDToLocationMap& locationMap, ContainerData* container, std::vector<uint16_t>& parentLocation);
		static void CalculateSingleLineText(SingleLineTextData& textData);
		static Math::vec2 CalculateSingleLineText(std::string_view textData);
		static size_t CalculateCursorIndexFromMousePosition(SingleLineTextData& textData, float textStartingPosition, float mouseXPosition, float textScalingFactor);
		static void CalculateMultiLineText(MultiLineTextData& textData, const Math::vec3& widgetSize, float textSize);

		//==============================
		// Manage Active UI (Internal)
		//==============================
		static void SetWidgetTextInternal(Ref<Widget> currentWidget, const std::string& newText);
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
		static void RenderSliderLine(const Math::vec4& color, const Math::vec3& translation, const Math::vec3& size);
		static void RenderSlider(const Math::vec4& color, const Math::vec3& translation, const Math::vec3& size);
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
		friend class DropDownWidget;
		friend class ContainerWidget;
		friend class VerticalContainerWidget;
		friend class HorizontalContainerWidget;
		friend class Window;
	};
}

namespace Kargono::Utility
{
	inline const char* WidgetTypeToString(RuntimeUI::WidgetTypes widgetType)
	{
		switch (widgetType)
		{
			case RuntimeUI::WidgetTypes::TextWidget: return "TextWidget";
			case RuntimeUI::WidgetTypes::ButtonWidget: return "ButtonWidget";
			case RuntimeUI::WidgetTypes::CheckboxWidget: return "CheckboxWidget";
			case RuntimeUI::WidgetTypes::ContainerWidget: return "ContainerWidget";
			case RuntimeUI::WidgetTypes::HorizontalContainerWidget: return "HorizontalContainerWidget";
			case RuntimeUI::WidgetTypes::VerticalContainerWidget: return "VerticalContainerWidget";
			case RuntimeUI::WidgetTypes::DropDownWidget: return "DropDownWidget";
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

	inline const char* WidgetTypeToDisplayString(RuntimeUI::WidgetTypes widgetType)
	{
		switch (widgetType)
		{
		case RuntimeUI::WidgetTypes::TextWidget: return "Multi-line Text";
		case RuntimeUI::WidgetTypes::ButtonWidget: return "Button";
		case RuntimeUI::WidgetTypes::CheckboxWidget: return "Checkbox";
		case RuntimeUI::WidgetTypes::ContainerWidget: return "Frame Container";
		case RuntimeUI::WidgetTypes::HorizontalContainerWidget: return "Horizontal Container";
		case RuntimeUI::WidgetTypes::VerticalContainerWidget: return "Vertical Container";
		case RuntimeUI::WidgetTypes::DropDownWidget: return "Drop-Down";
		case RuntimeUI::WidgetTypes::ImageWidget: return "Image";
		case RuntimeUI::WidgetTypes::ImageButtonWidget: return "Image-Button";
		case RuntimeUI::WidgetTypes::InputTextWidget: return "Input Text";
		case RuntimeUI::WidgetTypes::SliderWidget: return "Slider";
		case RuntimeUI::WidgetTypes::None: return "None";
		default:
		{
			KG_ERROR("Invalid Widget Type at WidgetTypeToString");
			return "None";
		}
		}
	}

	Ref<Rendering::Texture2D> WidgetTypeToIcon(RuntimeUI::WidgetTypes widgetType);

	inline RuntimeUI::WidgetTypes StringToWidgetType(std::string_view widgetName)
	{
		if (widgetName == "TextWidget") { return RuntimeUI::WidgetTypes::TextWidget; }
		if (widgetName == "ButtonWidget") { return RuntimeUI::WidgetTypes::ButtonWidget; }
		if (widgetName == "CheckboxWidget") { return RuntimeUI::WidgetTypes::CheckboxWidget; }
		if (widgetName == "ContainerWidget") { return RuntimeUI::WidgetTypes::ContainerWidget; }
		if (widgetName == "HorizontalContainerWidget") { return RuntimeUI::WidgetTypes::HorizontalContainerWidget; }
		if (widgetName == "VerticalContainerWidget") { return RuntimeUI::WidgetTypes::VerticalContainerWidget; }
		if (widgetName == "DropDownWidget") { return RuntimeUI::WidgetTypes::DropDownWidget; }
		if (widgetName == "ImageWidget") { return RuntimeUI::WidgetTypes::ImageWidget; }
		if (widgetName == "ImageButtonWidget") { return RuntimeUI::WidgetTypes::ImageButtonWidget; }
		if (widgetName == "InputTextWidget") { return RuntimeUI::WidgetTypes::InputTextWidget; }
		if (widgetName == "SliderWidget") { return RuntimeUI::WidgetTypes::SliderWidget; }
		if (widgetName == "None") { return RuntimeUI::WidgetTypes::None; }

		KG_ERROR("Invalid Widget Type at StringToWidgetType");
		return RuntimeUI::WidgetTypes::None;
	}

	inline const char* PixelOrPercentToString(RuntimeUI::PixelOrPercent type)
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

	inline RuntimeUI::PixelOrPercent StringToPixelOrPercent(std::string_view type)
	{
		if (type == "Pixel") { return RuntimeUI::PixelOrPercent::Pixel; }
		if (type == "Percent") { return RuntimeUI::PixelOrPercent::Percent; }

		KG_ERROR("Invalid PixelOrPercent at StringToPixelOrPercent");
		return RuntimeUI::PixelOrPercent::Pixel;
	}

	inline const char* RelativeOrAbsoluteToString(RuntimeUI::RelativeOrAbsolute type)
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

	inline RuntimeUI::RelativeOrAbsolute StringToRelativeOrAbsolute(std::string_view type)
	{
		if (type == "Relative") { return RuntimeUI::RelativeOrAbsolute::Relative; }
		if (type == "Absolute") { return RuntimeUI::RelativeOrAbsolute::Absolute; }

		KG_ERROR("Invalid RelativeOrAbsolute at StringToRelativeOrAbsolute");
		return RuntimeUI::RelativeOrAbsolute::Absolute;
	}

	inline const char* ConstraintToString(RuntimeUI::Constraint type)
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

	inline RuntimeUI::Constraint StringToConstraint(std::string_view type)
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
