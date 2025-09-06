#pragma once
#include "Modules/EditorUI/Widgets/EditorUIWidget.h"

#include "Kargono/Core/Base.h"
#include "Kargono/Core/FixedString.h"
#include "Kargono/Math/MathAliases.h"

#include <functional>

namespace Kargono::EditorUI
{
	enum EditIntegerFlags : WidgetFlags
	{
		EditInteger_None = 0,
		EditInteger_Indented = BIT(0)
	};

	struct EditIntegerWidget : public Widget
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		EditIntegerWidget() : Widget() {}
		~EditIntegerWidget() = default;
	public:
		//==============================
		// Rendering
		//==============================
		void RenderInteger();
	public:
		//==============================
		// Public Fields
		//==============================
		FixedString32 m_Label{};
		WidgetFlags m_Flags{ EditInteger_None };
		int32_t m_CurrentInteger{};
		std::array<int32_t, 2> m_Bounds{ 0, 0 };
		int32_t m_ScrollSpeed{ 1 };
		std::function<void(EditIntegerWidget&)> m_ConfirmAction{ nullptr };
		Ref<void> m_ProvidedData{ nullptr };
		bool m_Editing{ false };
	};

	enum EditIVec2Flags : WidgetFlags
	{
		EditIVec2_None = 0,
		EditIVec2_Indented = BIT(0)
	};

	struct EditIVec2Widget : public Widget
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		EditIVec2Widget() : Widget() {}
		~EditIVec2Widget() = default;
	public:
		//==============================
		// Rendering
		//==============================
		void RenderIVec2();
	public:
		//==============================
		// Public Fields
		//==============================
		FixedString32 m_Label{};
		WidgetFlags m_Flags{ EditIVec2_None };
		Math::ivec2 m_CurrentIVec2{};
		std::array<int32_t, 2> m_Bounds{ 0, 10'000 };
		int32_t m_ScrollSpeed{ 1 };
		std::function<void(EditIVec2Widget&)> m_ConfirmAction{ nullptr };
		Ref<void> m_ProvidedData{ nullptr };
		bool m_Editing{ false };
	};

	enum EditIVec3Flags : WidgetFlags
	{
		EditIVec3_None = 0,
		EditIVec3_Indented = BIT(0)
	};

	struct EditIVec3Widget : public Widget
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		EditIVec3Widget() : Widget() {}
		~EditIVec3Widget() = default;
	public:
		//==============================
		// Rendering
		//==============================
		void RenderIVec3();
	public:
		//==============================
		// Public Fields
		//==============================
		FixedString32 m_Label{};
		WidgetFlags m_Flags{ EditIVec3_None };
		Math::ivec3 m_CurrentIVec3{};
		std::array<int32_t, 2> m_Bounds{ 0, 10'000 };
		int32_t m_ScrollSpeed{ 1 };
		std::function<void(EditIVec3Widget&)> m_ConfirmAction{ nullptr };
		Ref<void> m_ProvidedData{ nullptr };
		bool m_Editing{ false };
	};

	enum EditIVec4Flags : WidgetFlags
	{
		EditIVec4_None = 0,
		EditIVec4_Indented = BIT(0)
	};

	struct EditIVec4Widget : public Widget
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		EditIVec4Widget() : Widget() {}
		~EditIVec4Widget() = default;
	public:
		//==============================
		// Rendering
		//==============================
		void RenderIVec4();
	public:
		//==============================
		// Public Fields
		//==============================
		FixedString32 m_Label{};
		WidgetFlags m_Flags{ EditIVec4_None };
		Math::ivec4 m_CurrentIVec4{};
		std::array<int32_t, 2> m_Bounds{ 0, 10'000 };
		int32_t m_ScrollSpeed{ 1 };
		std::function<void(EditIVec4Widget&)> m_ConfirmAction{ nullptr };
		Ref<void> m_ProvidedData{ nullptr };
		bool m_Editing{ false };
	};
}
