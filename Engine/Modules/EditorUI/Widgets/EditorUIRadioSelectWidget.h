#pragma once
#include "Modules/EditorUI/Widgets/EditorUIWidget.h"

#include "Kargono/Core/Base.h"
#include "Kargono/Core/FixedString.h"

#include <functional>

namespace Kargono::EditorUI
{
	enum RadioSelectFlags : WidgetFlags
	{
		RadioSelect_None = 0,
		RadioSelect_Indented = BIT(0)
	};

	struct RadioSelectWidget : public Widget
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		RadioSelectWidget() : Widget() {}
		~RadioSelectWidget() = default;
	public:
		//==============================
		// Rendering
		//==============================
		void RenderRadio();
	public:
		//==============================
		// Public Fields
		//==============================
		FixedString32 m_Label;
		WidgetFlags m_Flags{ RadioSelect_None };
		uint16_t m_SelectedOption{ 0 };
		std::string m_FirstOptionLabel{ "None" };
		std::string m_SecondOptionLabel{ "None" };
		bool m_Editing{ false };
		std::function<void()> m_SelectAction{ nullptr };
	};
}
