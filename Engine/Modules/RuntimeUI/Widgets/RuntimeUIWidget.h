#pragma once
#include "Kargono/Core/Base.h"
#include "Modules/Assets/Asset.h"
#include "Modules/Scripting/ScriptService.h"
#include "Modules/Rendering/Texture.h"
#include "Modules/RuntimeUI/RuntimeUICommon.h"

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
	class RuntimeUIService;
	struct RuntimeUIContext;
	class Window;
	class Widget;

	struct UserInterfaceCallbacks
	{
		Assets::AssetHandle m_OnMoveHandle{ Assets::EmptyHandle };
		Ref<Scripting::Script> m_OnMove{ nullptr };
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
		int32_t m_LeftWidgetID{ k_InvalidWidgetID };
		int32_t m_RightWidgetID{ k_InvalidWidgetID };
		int32_t m_UpWidgetID{ k_InvalidWidgetID };
		int32_t m_DownWidgetID{ k_InvalidWidgetID };
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
			m_ID = Utility::STLRandomService::GetActiveRandom().GenerateRandomInteger((int32_t)0, k_InvalidWidgetID - 1);
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
		Math::vec2 m_PercentSize{ 0.2f, 0.2f };
		Math::ivec2 m_PixelSize{ 50 };
		WidgetTypes m_WidgetType{ WidgetTypes::None };
	};
}