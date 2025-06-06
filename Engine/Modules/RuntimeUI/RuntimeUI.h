#pragma once

#include "Modules/Rendering/EditorPerspectiveCamera.h"
#include "Modules/Scripting/ScriptService.h"
#include "Kargono/Core/Base.h"
#include "Modules/RuntimeUI/Font.h"
#include "Modules/Assets/Asset.h"
#include "Kargono/Math/Math.h"
#include "Kargono/Core/Directions.h"
#include "Modules/RuntimeUI/RuntimeUICommon.h"
#include "Kargono/Core/Window.h"
#include "Kargono/Utility/Random.h"
#include "Modules/Rendering/Shader.h"

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

#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include <tuple>

namespace Kargono::RuntimeUI
{
	using IDToLocationMap = std::unordered_map<int32_t, std::vector<uint16_t>>;

	class UserInterface
	{
	public:
		// Config data
		Ref<Font> m_Font{ nullptr };
		Assets::AssetHandle m_FontHandle {Assets::EmptyHandle};
		Math::vec4 m_SelectColor {1.0f};
		Math::vec4 m_HoveredColor{ 0.5f };
		Math::vec4 m_EditingColor{ 0.15f, 0.15f, 0.15f, 1.0f };
		UserInterfaceCallbacks m_FunctionPointers{};

		// Runtime Data
		std::vector<Window> m_Windows {};
		std::vector<Window*> m_DisplayedWindows{};
		std::vector<size_t> m_DisplayedWindowIndices{};
		bool m_IBeamVisible{ true };
		float m_IBeamAccumulator{ 0.0f };
		float m_IBeamVisiblilityInterval{ 0.75f };
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
		//============================
		// Constructors/Destructors
		//============================
		NavigationLinksCalculator(UserInterface* parentInterface, ViewportData viewportData)
		{
			// Store the user interface and viewport data
			KG_ASSERT(parentInterface);
			i_UserInterface = parentInterface;
			i_ViewportData = viewportData;
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
		int32_t CalculateNavigationLink(Direction direction);
		void CompareCurrentAndPotentialWidget(Widget* potentialWidget);
	private:
		//============================
		// Internal Fields
		//============================
		// Current window state
		Window* m_CurrentWindow{ nullptr };
		BoundingBoxTransform m_CurrentWindowTransform{};
		// Current widget state
		Widget* m_CurrentWidget{ nullptr };
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

		//============================
		// Injected Data
		//============================
		UserInterface* i_UserInterface{ nullptr };
		ViewportData i_ViewportData{};
	};

	struct RuntimeUIContext
	{
	public:
		//==============================
		// Lifecycle Functions
		//==============================
		void Init();
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
		// Modify Active UI
		//==============================
		void SetSelectedWidgetColor(const Math::vec4& color);
		void SetActiveWidgetTextByTag(const std::string& windowTag, const std::string& widgetTag, const std::string& newText);
		void SetActiveWidgetTextByIndex(WidgetID widgetID, const std::string& newText);
		void SetWidgetImageByIndex(WidgetID widgetID, Assets::AssetHandle textureHandle);
		void SetActiveOnMove(Assets::AssetHandle functionHandle, Ref<Scripting::Script> function);
		void SetActiveOnHover(Assets::AssetHandle functionHandle, Ref<Scripting::Script> function);
		void SetActiveFont(Ref<Font> newFont, Assets::AssetHandle fontHandle);
		void SetWidgetTextColorByTag(const std::string& windowTag, const std::string& widgetTag, const Math::vec4& color);
		void SetWidgetTextColorByIndex(WidgetID widgetID, const Math::vec4& color);
		void SetSelectedWidgetByTag(const std::string& windowTag, const std::string& widgetTag);
		void SetSelectedWidgetByIndex(WidgetID widgetID);
		void ClearSelectedWidget();
		void SetEditingWidgetByIndex(WidgetID widgetID);
		void SetHoveredWidgetByIndex(WidgetID widgetID);
		void ClearHoveredWidget();
		void ClearEditingWidget();
		void SetWidgetBackgroundColorByTag(const std::string& windowTag, const std::string& widgetTag, const Math::vec4& color);
		void SetWidgetBackgroundColorByIndex(WidgetID widgetID, const Math::vec4& color);
		void SetWidgetSelectableByTag(const std::string& windowTag, const std::string& widgetTag, bool selectable);
		void SetWidgetSelectableByIndex(WidgetID widgetID, bool selectable);
		void SetDisplayWindowByTag(const std::string& windowTag, bool display);
		void SetDisplayWindowByIndex(WindowID widgetID, bool display);
		void AddActiveWindow(Window& window);
		bool DeleteActiveUIWindow(int32_t windowID);
		bool DeleteActiveUIWidget(int32_t widgetID);
		void AddWidgetToContainer(ContainerData* container, Ref<Widget> newWidget);

	public:
		//==============================
		// Rendering API
		//==============================
		void OnRender(const Math::mat4& cameraViewMatrix, uint32_t viewportWidth, uint32_t viewportHeight);
		void OnRender(uint32_t viewportWidth, uint32_t viewportHeight);

	public:
		//==============================
		// Modify Indicated UI
		//==============================
		bool DeleteUIWindow(Ref<UserInterface> userInterface, std::size_t windowLocation);
		bool DeleteUIWidget(Ref<UserInterface> userInterface, int32_t widgetID);

		//==============================
		// Query Active UI
		//==============================
		const std::string& GetWidgetTextByIndex(WidgetID widgetID);
		bool IsWidgetSelectedByTag(const std::string& windowTag, const std::string& widgetTag);
		bool IsWidgetSelectedByIndex(WidgetID widgetID);
		Ref<Scripting::Script> GetActiveOnMove();
		Assets::AssetHandle GetActiveOnMoveHandle();
		std::vector<Window>& GetAllActiveWindows();
		BoundingBoxTransform GetParentDimensionsFromID(int32_t widgetID, uint32_t viewportWidth, uint32_t viewportHeight);
		BoundingBoxTransform GetWidgetDimensionsFromID(int32_t widgetID, uint32_t viewportWidth, uint32_t viewportHeight);

		//==============================
		// Interact With Active UI
		//==============================
		void MoveRight();
		void MoveLeft();
		void MoveUp();
		void MoveDown();
		void OnPress();
		void OnPressByIndex(WidgetID widgetID);

		//==============================
		// Revalidate UI Context
		//==============================
		void RecalculateTextData(Widget* widget);
		void CalculateFixedAspectRatioSize(Widget* widget, uint32_t viewportWidth, uint32_t viewportHeight,
			bool useXValueAsBase);
		SelectionData* GetSelectionDataFromWidget(Widget* currentWidget);
		ContainerData* GetContainerDataFromWidget(Widget* currentWidget);
		ImageData* GetImageDataFromWidget(Widget* currentWidget);
		SingleLineTextData* GetSingleLineTextDataFromWidget(Widget* currentWidget);
		MultiLineTextData* GetMultiLineTextDataFromWidget(Widget* currentWidget);

	public:
		//==============================
		// Getters/Setters
		//==============================
		void SetActiveUI(Ref<UserInterface> userInterface, Assets::AssetHandle uiHandle);
		void SetActiveUIFromHandle(Assets::AssetHandle uiHandle);
		bool IsUIActiveFromHandle(Assets::AssetHandle uiHandle);
		Ref<UserInterface> GetActiveUI();
		Assets::AssetHandle GetActiveUIHandle();
		void ClearActiveUI();
		Ref<Widget> GetWidgetFromTag(const std::string& windowTag, const std::string& widgetTag);
		Ref<Widget> GetWidgetFromID(int32_t widgetID);
		Ref<Widget> GetWidgetFromDirections(const std::vector<uint16_t>& locationDirections);
		IDType CheckIDType(int32_t windowOrWidgetID);
		Window& GetWindowFromID(int32_t windowID);
		Window& GetParentWindowFromWidgetID(int32_t widgetID);
		Ref<Widget> GetParentWidgetFromID(int32_t widgetID);
		std::vector<uint16_t>* GetLocationFromID(int32_t windowOrWidgetID);
		std::tuple<Ref<Widget>, Window*> GetWidgetAndWindow(const std::string& windowTag, const std::string& widgetTag);
		std::tuple<Ref<Widget>, Window*> GetWidgetAndWindow(int32_t widgetID);

	private:
		//==============================
		// Revalidate UI Context (Internal)
		//==============================
		void RevalidateDisplayedWindows();
		void RevalidateWidgetIDToLocationMap();
		void RevalidateContainerInLocationMap(IDToLocationMap& locationMap, ContainerData* container, std::vector<uint16_t>& parentLocation);
		void CalculateSingleLineText(SingleLineTextData& textData);
		Math::vec2 CalculateSingleLineText(std::string_view textData);
		size_t CalculateCursorIndexFromMousePosition(SingleLineTextData& textData, float textStartingPosition, float mouseXPosition, float textScalingFactor);
		void CalculateMultiLineText(MultiLineTextData& textData, const Math::vec3& widgetSize, float textSize);

		//==============================
		// Manage Active UI (Internal)
		//==============================
		void SetWidgetTextInternal(Ref<Widget> currentWidget, const std::string& newText);
		void SetSelectedWidgetInternal(Ref<Widget> newSelectedWidget);
		void SetHoveredWidgetInternal(Ref<Widget> newSelectedWidget);
		void SetWidgetTextColorInternal(Ref<Widget> currentWidget, const Math::vec4& newColor);
		void SetWidgetSelectableInternal(Ref<Widget> currentWidget, bool selectable);
		bool IsWidgetSelectedInternal(Ref<Widget> currentWidget);
		void SetWidgetBackgroundColorInternal(Ref<Widget> currentWidget, const Math::vec4& newColor);


		//==============================
		// Interact With Active UI (Internal)
		//==============================
		void OnPressInternal(Widget* currentWidget);
		void OnMoveCursorInternal(Widget* currentWidget);

		//==============================
		// Rendering API (Internal)
		//==============================
		void RenderBackground(const Math::vec4& color, const Math::vec3& translation, const Math::vec3 size);
		void RenderImage(const ImageData& imageData, const Math::vec3& translation, const Math::vec3 size);
		void RenderSingleLineText(const SingleLineTextData& textData, const Math::vec3& textStartingPoint, float textScalingFactor);
		void RenderTextCursor(const SingleLineTextData& textData, const Math::vec3& renderLocation, float textScalingFactor);
		void RenderSliderLine(const Math::vec4& color, const Math::vec3& translation, const Math::vec3& size);
		void RenderSlider(const Math::vec4& color, const Math::vec3& translation, const Math::vec3& size);
		Math::vec3 GetSingleLineTextStartingPosition(const SingleLineTextData& textData, const Math::vec3& translation, const Math::vec3 size, float textScalingFactor);
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
		friend struct RuntimeUIContext;

	public:
		// TODO: Make these private
		//==============================
		// Public Fields
		//==============================
		Ref<UserInterface> m_ActiveUI{ nullptr };
		Assets::AssetHandle m_ActiveUIHandle{ Assets::EmptyHandle };
		Ref<Font> m_DefaultFont{ nullptr };
		Rendering::RendererInputSpec m_BackgroundInputSpec{};
		Rendering::RendererInputSpec m_ImageInputSpec{};
	};

	//============================
	// Runtime UI Service Class
	//============================
	class RuntimeUIService
	{
	public:
		//==============================
		// Create RuntimeUI Context
		//==============================
		static void CreateRuntimeUIContext()
		{
			// Initialize RuntimeUIWorld
			if (!s_RuntimeUIContext)
			{
				s_RuntimeUIContext = CreateRef<RuntimeUIContext>();
			}

			// Verify init is successful
			KG_VERIFY(s_RuntimeUIContext, "RuntimeUI Service System Initiated");
		}
		static void RemoveRuntimeUIContext()
		{
			// Clear RuntimeUIWorld
			s_RuntimeUIContext.reset();
			s_RuntimeUIContext = nullptr;

			// Verify terminate is successful
			KG_VERIFY(!s_RuntimeUIContext, "RuntimeUI Service System Initiated");
		}
		//==============================
		// Getters/Setters
		//==============================
		static RuntimeUIContext& GetActiveContext() { return *s_RuntimeUIContext; }
		static bool IsContextActive() { return (bool)s_RuntimeUIContext; }
	private:
		//==============================
		// Internal Fields
		//==============================
		static inline Ref<RuntimeUIContext> s_RuntimeUIContext{ nullptr };
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
