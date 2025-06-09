#pragma once
#include "Modules/Rendering/Texture.h"
#include "Modules/RuntimeUI/RuntimeUICommon.h"
#include "Modules/RuntimeUI/Font.h"

#include "Modules/Scripting/ScriptService.h"
#include "Kargono/Core/Base.h"
#include "Modules/Assets/Asset.h"
#include "Kargono/Core/Window.h"

#include <cstdint>
#include <vector>

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
	class Window;
	class Widget;
	struct RuntimeUIContext;
	class UserInterface;

	enum class WidgetTypes
	{
		None = 0, TextWidget, ButtonWidget, CheckboxWidget, ContainerWidget,
		DropDownWidget, ImageWidget, ImageButtonWidget,
		InputTextWidget, SliderWidget, VerticalContainerWidget, HorizontalContainerWidget
	};

	struct NavigationLinks
	{
		WidgetID m_LeftWidgetID{ k_InvalidWidgetID };
		WidgetID m_RightWidgetID{ k_InvalidWidgetID };
		WidgetID m_UpWidgetID{ k_InvalidWidgetID };
		WidgetID m_DownWidgetID{ k_InvalidWidgetID };
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

	struct WidgetCallbacks
	{
		Assets::AssetHandle m_OnPressHandle{ Assets::EmptyHandle };
		Ref<Scripting::Script> m_OnPress{ nullptr };
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
	public:
		//============================
		// Modify Container
		//============================
		void AddWidget(Ref<Widget> newWidget);
	public:
		//============================
		// Revalidation Functions
		//============================
		void RevalidateTextDimensions();
	public:
		//============================
		// Public Fields
		//============================
		std::vector<Ref<Widget>> m_ContainedWidgets;
		Math::vec4 m_BackgroundColor{ 0.5f };
	};

	struct ImageData
	{
	public:
		//============================
		// Rendering
		//============================
		void RenderImage(RuntimeUIContext* uiContext, const Math::vec3& translation, const Math::vec3 size);
	public:
		//============================
		// Public Fields
		//============================
		Ref<Rendering::Texture2D> m_ImageRef{ nullptr };
		Assets::AssetHandle m_ImageHandle{ Assets::EmptyHandle };
		bool m_FixedAspectRatio{ false };
	};

	struct SingleLineTextData
	{
	public:
		//============================
		// Rendering
		//============================
		void OnRender(RuntimeUIContext* uiContext, const Math::vec3& textStartingPoint, float textScalingFactor);
		void RenderTextCursor(RuntimeUIContext* uiContext, const Math::vec3& textStartingPoint, float textScalingFactor);
	public:
		//============================
		// Modify State
		//============================
		void SetText(UserInterface* parentUI, std::string_view text);
	public:
		//============================
		// Revalidate Text
		//============================
		void RevalidateTextDimensions(UserInterface* parentUI);
	public:
		//============================
		// Query State
		//============================
		Math::vec3 GetTextStartingPosition(const Math::vec3& translation, const Math::vec3 size, float textScalingFactor);
		Math::vec2 GetTextDimensions(UserInterface* parentUI, std::string_view text) const;
		size_t GetCursorIndexFromMousePosition(Ref<Font> font, float textStartingPosition, float mouseXPosition, float textScalingFactor);
	public:
		//============================
		// Public Fields
		//============================
		std::string m_Text{ "..." };
		float m_TextSize{ 0.3f };
		Math::vec4 m_TextColor{ 1.0f };
		Constraint m_TextAlignment{ Constraint::Center };

		// Runtime calculated data
		Math::vec2 m_CachedTextDimensions{};
		size_t m_CaretIndex{ 0 };
	};

	struct MultiLineTextData
	{
	public:
		//============================
		// Revalidate Text
		//============================
		void RevalidateTextDimensions(UserInterface* parentUI, const Math::vec3& widgetSize, float textSize);
	public:
		//============================
		// Public Fields
		//============================
		std::string m_Text{ "New Text Widget" };
		float m_TextSize{ 0.3f };
		Math::vec4 m_TextColor{ 1.0f };
		Constraint m_TextAlignment{ Constraint::Center };
		bool m_TextWrapped{ false };

		// Runtime calculated data
		MultiLineTextDimensions m_CachedTextDimensions{};
	};

	class Widget
	{
	public:
		//============================
		// Constructors/Destructors
		//============================
		Widget(UserInterface* parentInterface)
		{
			KG_ASSERT(parentInterface);
			i_ParentUI = parentInterface;

			m_ID = Utility::STLRandomService::GetActiveRandom().GenerateRandomInteger((int32_t)0, k_InvalidWidgetID - 1);
		}
		virtual ~Widget() = default;
	public:
		//============================
		// Rendering Methods
		//============================
		virtual void OnRender(RuntimeUIContext* uiContext, Math::vec3 translation, 
			const Math::vec3& scale, float viewportWidth) = 0;
	protected:
		// Rendering helper function(s)
		void RenderBackground(RuntimeUIContext* uiContext, const Math::vec4& color, const Math::vec3& translation, const Math::vec3 size);
	public:
		//============================
		// Query Widget State
		//============================
		Math::vec3 CalculateWidgetSize(const Math::vec3& windowSize);
		Math::vec3 CalculateWorldPosition(const Math::vec3& windowTranslation, const Math::vec3& windowSize);
		Math::vec3 CalculateWindowPosition(Math::vec2 worldPosition, const Math::vec3& windowTranslation, const Math::vec3& windowSize);
	public:
		//============================
		// Query Widget Type Info/Data
		//============================
		virtual bool Selectable() { return false; }
		virtual SelectionData* GetSelectionData() { return nullptr; }
		virtual SingleLineTextData* GetSingleLineTextData() { return nullptr; }
		virtual MultiLineTextData* GetMultiLineTextData() { return nullptr; }
		virtual ContainerData* GetContainerData() { return nullptr; }
		virtual ImageData* GetImageData() { return nullptr; }

		//============================
		// Revalidate Widget Type Data
		//============================
		virtual void RevalidateTextDimensions() {};
		void RevalidateImageSize(ViewportData viewportData, bool useXValueAsBase);
	public:
		//============================
		// Public Fields
		//============================
		std::string m_Tag{ "None" };
		WidgetID m_ID{ k_InvalidWidgetID };
		PixelOrPercent m_SizeType{ PixelOrPercent::Percent };
		PixelOrPercent m_XPositionType{ PixelOrPercent::Percent };
		PixelOrPercent m_YPositionType{ PixelOrPercent::Percent };
		RelativeOrAbsolute m_XRelativeOrAbsolute{ RelativeOrAbsolute::Absolute };
		RelativeOrAbsolute m_YRelativeOrAbsolute{ RelativeOrAbsolute::Absolute };
		Constraint m_XConstraint{ Constraint::None };
		Constraint m_YConstraint{ Constraint::None };
		Math::vec2 m_PercentPosition{ 0.0f };
		Math::ivec2 m_PixelPosition{ 0 };
		Math::vec2 m_PercentSize{ 0.2f, 0.2f };
		Math::ivec2 m_PixelSize{ 50 };
		WidgetTypes m_WidgetType{ WidgetTypes::None };
	public:
		//============================
		// Injected Dependencies
		//============================
		UserInterface* i_ParentUI{ nullptr };
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