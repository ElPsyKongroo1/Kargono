#pragma once
#include "Modules/EditorUI/Widgets/EditorUIWidget.h"

#include "Kargono/Core/Base.h"
#include "Kargono/Core/FixedString.h"
#include "Kargono/Math/MathAliases.h"

#include <functional>
#include <array>

namespace Kargono::EditorUI
{
	enum EditFloatFlags : WidgetFlags
	{
		EditFloat_None = 0,
		EditFloat_Indented = BIT(0)
	};

	struct EditFloatWidget : public Widget
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		EditFloatWidget() : Widget() {}
		~EditFloatWidget() = default;
	public:
		//==============================
		// Rendering
		//==============================
		void RenderFloat();
	public:
		//==============================
		// Public Fields
		//==============================
		FixedString32 m_Label{};
		WidgetFlags m_Flags{ EditFloat_None };
		float m_CurrentFloat{};
		std::function<void(EditFloatWidget&)> m_ConfirmAction{ nullptr };
		Ref<void> m_ProvidedData{ nullptr };
		std::array<float, 2> m_Bounds{ 0.0f, 0.0f };
		float m_ScrollSpeed{ 0.01f };
		bool m_Editing{ false };
	};

	enum EditVec2Flags : WidgetFlags
	{
		EditVec2_None = 0,
		EditVec2_Indented = BIT(0)
	};

	struct EditVec2Widget : public Widget
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		EditVec2Widget() : Widget() {}
		~EditVec2Widget() = default;
	public:
		//==============================
		// Rendering
		//==============================
		void RenderVec2();
	public:
		//==============================
		// Public Fields
		//==============================
		FixedString32 m_Label{};
		WidgetFlags m_Flags{ EditVec2_None };
		Math::vec2 m_CurrentVec2{};
		std::function<void(EditVec2Widget&)> m_ConfirmAction{ nullptr };
		Ref<void> m_ProvidedData{ nullptr };
		std::array<float, 2> m_Bounds{ 0.0f,0.0f };
		float m_ScrollSpeed{ 0.01f };
		bool m_Editing{ false };
	};

	enum EditVec3Flags : WidgetFlags
	{
		EditVec3_None = 0,
		EditVec3_Indented = BIT(0)
	};

	struct EditVec3Widget : public Widget
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		EditVec3Widget() : Widget() {}
		~EditVec3Widget() = default;
	public:
		//==============================
		// Rendering
		//==============================
		void RenderVec3();
	public:
		//==============================
		// Public Fields
		//==============================
		FixedString32 m_Label{};
		WidgetFlags m_Flags{ EditVec3_None };
		Math::vec3 m_CurrentVec3{};
		std::function<void(EditVec3Widget&)> m_ConfirmAction{ nullptr };
		Ref<void> m_ProvidedData{ nullptr };
		std::array<float, 2> m_Bounds{ 0.0f, 0.0f };
		float m_ScrollSpeed{ 0.01f };
		bool m_Editing{ false };
	};

	enum EditVec4Flags : WidgetFlags
	{
		EditVec4_None = 0,
		EditVec4_Indented = BIT(0),
		EditVec4_RGBA = BIT(1),
		EditVec4_HandleEditButtonExternally = BIT(2)
	};

	struct EditVec4Widget : public Widget
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		EditVec4Widget() : Widget() {}
		~EditVec4Widget() = default;
	public:
		//==============================
		// Rendering
		//==============================
		void RenderVec4();
	public:
		//==============================
		// Public Fields
		//==============================
		FixedString32 m_Label{};
		WidgetFlags m_Flags{ EditVec4_None };
		Math::vec4 m_CurrentVec4{};
		std::function<void(EditVec4Widget&)> m_ConfirmAction{ nullptr };
		std::function<void(EditVec4Widget&)> m_OnEdit{ nullptr };
		Ref<void> m_ProvidedData{ nullptr };
		std::array<float, 2> m_Bounds{ 0.0f, 0.0f };
		float m_ScrollSpeed{ 0.01f };
		bool m_Editing{ false };
	};
}


