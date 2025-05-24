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
#include "Modules/RuntimeUI/Widgets/RuntimeUImageWidget.h"
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

	struct RuntimeUIContext
	{
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
		// Lifecycle Functions
		//==============================
		static void Init();
		static void Terminate();
		static void OnUpdate(Timestep ts);

		//==============================
		// On Event Functions
		//==============================
		static bool OnKeyTypedEvent(Events::KeyTypedEvent event);
		static bool OnKeyPressedEvent(Events::KeyPressedEvent event);
		static void OnMouseButtonPressedEvent(const Events::MouseButtonPressedEvent& event);
		static void OnMouseButtonReleasedEvent(const Events::MouseButtonReleasedEvent& mouseEvent);

	public:
		//==============================
		// Rendering API
		//==============================
		static void OnRender(const Math::mat4& cameraViewMatrix, uint32_t viewportWidth, uint32_t viewportHeight);
		static void OnRender(uint32_t viewportWidth, uint32_t viewportHeight);

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
