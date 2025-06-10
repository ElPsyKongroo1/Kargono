#pragma once
#include "Modules/EditorUI/Widgets/EditorUIWidget.h"

#include "Kargono/Core/Base.h"
#include "Kargono/Core/FixedString.h"

#include <functional>

namespace Kargono::EditorUI
{
	struct Button
	{
		FixedString32 m_Label{ "Click Me" };
		std::function<void(Button&)> m_OnPress{ nullptr };
		Ref<void> m_ProvidedData{ nullptr };
	};

	enum ButtonFlags : WidgetFlags
	{
		Button_None = 0,
		Button_Indented = BIT(0)
	};

	struct ButtonSpec : public Widget
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		ButtonSpec() : Widget() {}
		~ButtonSpec() = default;
	public:
		//==============================
		// Rendering
		//==============================
		bool RenderButton();
	public:
		//==============================
		// Public Fields
		//==============================
		FixedString32 m_Label;
		WidgetFlags m_Flags{ ButtonFlags::Button_None };
		Button m_Button;
	};
}
