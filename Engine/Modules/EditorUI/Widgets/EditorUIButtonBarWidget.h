#pragma once
#include "Modules/EditorUI/Widgets/EditorUIWidget.h"
#include "Modules/EditorUI/Widgets/EditorUIButtonWidget.h"

#include "Kargono/Core/Base.h"
#include "Kargono/Core/FixedString.h"

#include <functional>

namespace Kargono::EditorUI
{
	constexpr size_t k_MaxButtonBarSize{ 4 };

	enum ButtonBarFlags : WidgetFlags
	{
		ButtonBar_None = 0,
		ButtonBar_Indented = BIT(0)
	};

	struct ButtonBarWidget : public Widget
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		ButtonBarWidget() : Widget() {}
	public:
		//==============================
		// Rendering
		//==============================
		void RenderBar();

	public:
		//==============================
		// Modify Buttons
		//==============================
		// Add
		bool AddButton(Button& button);
		bool AddButton(std::string_view label, std::function<void(Button&)> onClick, Ref<void> providedData = nullptr);
		// Clear
		void ClearButtons();

	public:
		//==============================
		// Query Buttons
		//==============================
		Button* GetButton(size_t index);
	public:
		//==============================
		// Public Fields
		//==============================
		FixedString32 m_Label;
		WidgetFlags m_Flags{ ButtonBarFlags::ButtonBar_None };
	private:
		//==============================
		// Internal Fields
		//==============================
		std::array<Button, k_MaxButtonBarSize> m_Buttons;
		size_t m_ButtonCount{ 0 };
	};
}

