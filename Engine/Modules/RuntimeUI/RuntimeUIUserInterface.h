#pragma once

#include "Modules/RuntimeUI/RuntimeUICommon.h"

#include "Modules/RuntimeUI/Widgets/RuntimeUIWidget.h"
#include "Modules/RuntimeUI/Widgets/RuntimeUIWidget.h"
#include "Modules/RuntimeUI/Widgets/RuntimeUIButtonWidget.h"
#include "Modules/RuntimeUI/Widgets/RuntimeUICheckboxWidget.h"
#include "Modules/RuntimeUI/Widgets/RuntimeUIContainerWidget.h"
#include "Modules/RuntimeUI/Widgets/RuntimeUIDropdownWidget.h"
#include "Modules/RuntimeUI/Widgets/RuntimeUIHorizontalContainerWidget.h"
#include "Modules/RuntimeUI/Widgets/RuntimeUIVerticalContainerWidget.h"
#include "Modules/RuntimeUI/Widgets/RuntimeUIImageButtonWidget.h"
#include "Modules/RuntimeUI/Widgets/RuntimeUIImageWidget.h"
#include "Modules/RuntimeUI/Widgets/RuntimeUISliderWidget.h"
#include "Modules/RuntimeUI/Widgets/RuntimeUITextWidget.h"
#include "Modules/RuntimeUI/Widgets/RuntimeUIInputTextWidget.h"
#include "Modules/RuntimeUI/RuntimeUIWindow.h"

#include "Modules/Scripting/ScriptService.h"
#include "Modules/Assets/AssetsTypes.h"
#include "Kargono/Core/Directions.h"
#include "Modules/Core/Engine.h"
#include "Modules/Events/KeyEvent.h"
#include "Modules/Events/MouseEvent.h"
#include "Kargono/Core/Window.h"

#include <vector>
#include <unordered_map>

namespace Kargono::RuntimeUI
{
	class UserInterface;
	struct RuntimeUIContext;

	using UILocation = std::vector<uint16_t>;
	using IDToLocationMap = std::unordered_map<WidgetOrWindowID, UILocation>;

	struct UserInterfaceCallbacks
	{
		Assets::AssetHandle m_OnMoveHandle{ Assets::EmptyHandle };
		Ref<Scripting::Script> m_OnMove{ nullptr };
		Assets::AssetHandle m_OnHoverHandle{ Assets::EmptyHandle };
		Ref<Scripting::Script> m_OnHover{ nullptr };
	};

	struct Bounds
	{
		Math::vec3 m_Translation;
		Math::vec3 m_Size;
	};

	struct UIWindowsState;

	class NavigationLinksCalculator
	{
	private:
		//============================
		// Constructors/Destructors
		//============================
		NavigationLinksCalculator(UIWindowsState* parentWindows, ViewportData viewportData) : i_WindowsState(parentWindows), i_ViewportData(viewportData)
		{
			KG_ASSERT(parentWindows);
		}
		~NavigationLinksCalculator() = default;
	public:
		//============================
		// Calculate Links
		//============================
		void CalculateNavigationLinks();
	private:
		// Helper functions
		void CalculateWidgetNavigationLinks(Widget* currentWidget);
		WidgetID CalculateNavigationLink(Direction direction);
		void CompareCurrentAndPotentialWidget(Widget* potentialWidget);
	private:
		//============================
		// Internal Fields
		//============================
		// Current window state
		Window* m_CurrentWindow{ nullptr };
		Bounds m_CurrentWindowTransform{};
		// Current widget state
		Widget* m_CurrentWidget{ nullptr };
		Bounds m_CurrentWidgetParentTransform{};
		Math::vec2 m_CurrentWidgetPosition{};
		Math::vec2 m_CurrentWidgetSize{};
		Math::vec2 m_CurrentWidgetCenterPosition{};
		// Current potential widget info
		Bounds m_PotentialWidgetParentTransform{};
		// Nav-link calculation fields
		WidgetID m_CurrentBestChoiceID{ k_InvalidWidgetID };
		float m_CurrentBestDistance{ std::numeric_limits<float>::max() };
		Direction m_CurrentDirection{ Direction::None };

	private:
		//============================
		// Injected Data
		//============================
		UIWindowsState* i_WindowsState{ nullptr };
		ViewportData i_ViewportData{};

	private:
		//============================
		// Owning Class Declare
		//============================
		friend struct UIWindowsState;
	};

	struct UIConfig
	{
	private:
		//============================
		// Constructors/Destructors
		//============================
		UIConfig() = default;
		~UIConfig() = default;
	private:
		//============================
		// Lifecycle Function(s)
		//============================
		void Init(UIWindowsState* windowsState, Ref<Font> defaultFont);
		void Terminate();
	public:
		//============================
		// Modify State
		//============================
		void SetFont(Ref<Font> newFont, Assets::AssetHandle fontHandle);
		void SetOnMove(Assets::AssetHandle functionHandle, Ref<Scripting::Script> function);
		void SetOnHover(Assets::AssetHandle functionHandle, Ref<Scripting::Script> function);
	public:
		//============================
		// Public Fields
		//============================
		Ref<Font> m_Font{ nullptr };
		Assets::AssetHandle m_FontHandle{ Assets::EmptyHandle };
		Math::vec4 m_SelectColor{ 1.0f };
		Math::vec4 m_HoveredColor{ 0.5f };
		Math::vec4 m_EditingColor{ 0.15f, 0.15f, 0.15f, 1.0f };
		UserInterfaceCallbacks m_FunctionPointers{};
	private:
		//============================
		// Internal Fields
		//============================
		// State
		bool m_Active{ false };
	private:
		//============================
		// Injected Dependencies
		//============================
		UIWindowsState* i_WindowsState{ nullptr };
	private:
		//============================
		// Owning Class Declare
		//============================
		friend class UserInterface;
	};

	struct UIInteractionState
	{
	private:
		//============================
		// Constructors/Destructors
		//============================
		UIInteractionState() = default;
		~UIInteractionState() = default;

	private:
		//============================
		// Lifecycle Function(s)
		//============================
		void Init(UIWindowsState* windowsState, UIConfig* config);
		void Terminate();
	public:
		//============================
		// Modify State
		//============================
		// Update
		void SetSelectedWidgetByTag(std::string_view windowTag, std::string_view widgetTag);
		void SetSelectedWidgetByHandle(WidgetHandle widgetHandle);
		void SetHoveredWidgetByHandle(WidgetHandle widgetHandle);
		void SetEditingWidgetByHandle(WidgetHandle widgetHandle);
		void SetSelectedWidgetColor(const Math::vec4& color);
		// Clear/Delete
		void ClearSelectedWidget();
		void ClearHoveredWidget();
		void ClearEditingWidget();
	private:
		// Helpers
		void SetSelectedWidgetInternal(Ref<Widget> newSelectedWidget);
		void SetHoveredWidgetInternal(Ref<Widget> newHoveredWidget);
	public:
		//============================
		// Query State
		//============================
		// Selected
		bool IsWidgetSelectedByTag(std::string_view windowTag, std::string_view widgetTag);
		bool IsWidgetSelectedByHandle(WidgetHandle widgetHandle);
	private:
		// Query helper functions
		bool IsWidgetSelectedInternal(Ref<Widget> currentWidget);
	public:
		//============================
		// Public Fields
		//============================
		// Interaction widgets
		Widget* m_SelectedWidget{ nullptr };
		Widget* m_HoveredWidget{ nullptr };
		Widget* m_EditingWidget{ nullptr };
		Widget* m_PressedWidget{ nullptr };
		Window* m_ActiveWindow{ nullptr };

	private:
		//============================
		// Internal Fields
		//============================
		// State
		bool m_Active{ false };
	private:
		//============================
		// Injected Dependencies
		//============================
		UIWindowsState* i_WindowsState{ nullptr };
		UIConfig* i_Config{ nullptr };
	private:
		//============================
		// Owning Class Declare
		//============================
		friend class UserInterface;
	};

	struct ResultWidgetWindow
	{
		Ref<Widget> m_Widget;
		Window* m_Window;
	};

	struct UIWindowsState
	{
	private:
		//============================
		// Constructors/Destructors
		//============================
		UIWindowsState() = default;
		~UIWindowsState() = default;

	private:
		//============================
		// Lifecycle Function(s)
		//============================
		void Init();
		void Terminate();

	public:
		void OnRender(RuntimeUIContext* uiContext, ViewportData viewportData);
	public:
		//============================
		// Modify Windows
		//============================
		// Add/Create
		void AddWindow(Window& window);
		// Delete
		bool DeleteWindow(WindowID windowID);
		// Update
		void SetDisplayWindowByTag(std::string_view windowTag, bool display);
		void SetDisplayWindowByHandle(WindowHandle windowHandle, bool display);
	private:
		// Modify window helpers
		bool DeleteWindowInternal(WindowIndex windowIndex);

	public:
		//============================
		// Modify Widgets
		//============================
		// Delete
		bool DeleteWidget(WidgetID widgetID);
		// Update
		void SetWidgetSelectableByTag(std::string_view windowTag, std::string_view widgetTag, bool selectable);
		void SetWidgetSelectableByHandle(WidgetHandle widgetHandle, bool selectable);
		void SetWidgetImageByHandle(WidgetHandle widgetHandle, Assets::AssetHandle textureHandle);
		void SetWidgetTextColorByTag(std::string_view windowTag, std::string_view widgetTag, const Math::vec4& color);
		void SetWidgetTextColorByHandle(WidgetHandle widgetHandle, const Math::vec4& color);
		void SetWidgetBackgroundColorByTag(std::string_view windowTag, std::string_view widgetTag, const Math::vec4& color);
		void SetWidgetBackgroundColorByHandle(WidgetHandle widgetHandle, const Math::vec4& color);
		void SetActiveWidgetTextByTag(std::string_view windowTag, std::string_view widgetTag, std::string_view newText);
		void SetActiveWidgetTextByHandle(WidgetHandle widgetHandle, std::string_view newText);
	private:
		// Modify widget helpers
		bool DeleteWidgetInternal(WidgetID widgetID);
		void SetWidgetTextInternal(Ref<Widget> currentWidget, std::string_view newText);
		void SetWidgetTextColorInternal(Ref<Widget> currentWidget, const Math::vec4& newColor);
		void SetWidgetSelectableInternal(Ref<Widget> currentWidget, bool selectable);
		void SetWidgetBackgroundColorInternal(Ref<Widget> currentWidget, const Math::vec4& newColor);

	public:
		//============================
		// Query Widget/Window Fields
		//============================
		// Text
		std::string_view GetWidgetTextByHandle(WidgetHandle widgetHandle);
		// Bounds
		Bounds GetParentBoundsFromID(WidgetID widgetID, ViewportData viewportData);
		Bounds GetWidgetBoundsFromID(WidgetID widgetID, ViewportData viewportData);
		// ID Type
		IDType CheckIDType(WidgetOrWindowID windowOrWidgetID);
	public:
		//============================
		// Get Widgets/Windows
		//============================
		// Widget
		Ref<Widget> GetWidgetFromTag(std::string_view windowTag, std::string_view widgetTag);
		Ref<Widget> GetWidgetFromID(WidgetID widgetID);
		Ref<Widget> GetWidgetFromDirections(const UILocation& locationDirections);
		Ref<Widget> GetParentWidgetFromID(WidgetID widgetID);
		// Window
		Window& GetWindowFromID(WindowID windowID);
		Window& GetParentWindowFromWidgetID(WidgetID widgetID);
		// Window/Widgets
		UILocation* GetLocationFromID(WidgetOrWindowID windowOrWidgetID);
		ResultWidgetWindow GetWidgetAndWindow(std::string_view windowTag, const std::string_view widgetTag);
		ResultWidgetWindow GetWidgetAndWindow(WidgetID widgetID);

	public:
		//==============================
		// Revalidation Functions
		//==============================
		void RevalidateDisplayedWindows();
		void RevalidateIDToLocationMap();
		void RevalidateTextDimensions();
		void RevalidateDefaultWidgets();
		void RevalidateNavigationLinks();
	private:
		// Revalidation helper functions
		void RevalidateContainerInLocationMap(ContainerData* container, UILocation& parentLocation);
	public:
		//============================
		// Public Fields
		//============================
		// Window storage
		std::vector<Window> m_Windows{};
		// Window state/references/meta-data
		std::vector<Window*> m_DisplayedWindows{};
		std::vector<WindowIndex> m_DisplayedWindowIndices{};
		IDToLocationMap m_IDToLocation{};

	private:
		//============================
		// Internal Fields
		//============================
		bool m_Active{ false };
	private:
		//============================
		// Owning Class Declare
		//============================
		friend class UserInterface;
	};

	struct UITextCaretState
	{
	private:
		//============================
		// Constructors/Destructors
		//============================
		UITextCaretState() = default;
		~UITextCaretState() = default;
	public:
		//============================
		// Public Fields
		//============================
		bool m_CaretVisible{ true };
		float m_CaretAccumulator{ 0.0f /*sec*/ };
		float m_CaretVisibleInterval{ 0.75f /*sec*/};
	private:
		//============================
		// Owning Class Declare
		//============================
		friend class UserInterface;
	};

	class UserInterface
	{
	public:
		//============================
		// Constructors/Destructors
		//============================
		UserInterface() = default;
		~UserInterface() = default;
	public:
		//============================
		// Lifecycle Functions
		//============================
		void Init(RuntimeUIContext* parentContext);
		void Terminate();
		void OnUpdate(Timestep ts);
	public:
		//==============================
		// On Event Functions
		//==============================
		bool OnKeyTypedEvent(Events::KeyTypedEvent event);
		bool OnKeyPressedEvent(Events::KeyPressedEvent event);
		void OnMouseButtonPressedEvent(const Events::MouseButtonPressedEvent& event);
		void OnMouseButtonReleasedEvent(const Events::MouseButtonReleasedEvent& mouseEvent);
	public:
		//==============================
		// Rendering API
		//==============================
		void OnRenderCamera(const Math::mat4& cameraViewMatrix, ViewportData viewportData);
		void OnRenderViewport(ViewportData viewportData);
	public:
		//==============================
		// Interact With UI
		//==============================
		void MoveRight();
		void MoveLeft();
		void MoveUp();
		void MoveDown();
		void OnPress();
		void OnPressByIndex(WidgetHandle widgetHandle);
	private:
		// Interaction helpers
		void OnPressInternal(Widget* currentWidget);
		void OnMoveCursorInternal(Widget* currentWidget);
	public:
		//============================
		// Getters/Setters
		//============================
		UIConfig& GetConfig() { return m_Config; }
		UIInteractionState& GetInteractState() { return m_InteractState; }
		UIWindowsState& GetWindowsState() { return m_WindowsState; }
		UITextCaretState& GetTextCaretState() { return m_CaretState; }
	public:
		//============================
		// Public Fields
		//============================
		// Config data
		UIConfig m_Config{};
		// Runtime data
		UIInteractionState m_InteractState{};
		UIWindowsState m_WindowsState{};
		UITextCaretState m_CaretState{};
	private:
		//============================
		// Internal Fields
		//============================
		bool m_Active{ false };
	private:
		//============================
		// Injected Dependencies
		//============================
		RuntimeUIContext* i_ParentContext{ nullptr };
	};
}