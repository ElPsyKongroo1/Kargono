#pragma once
#include "Modules/EditorUI/Widgets/EditorUIWidget.h"

#include "Kargono/Core/Base.h"
#include "Kargono/Core/FixedString.h"

#include <functional>

namespace Kargono::EditorUI
{
	enum CheckboxFlags : WidgetFlags
	{
		Checkbox_None = 0,
		Checkbox_LeftLean = BIT(0), // Check box aligns to the left
		Checkbox_Indented = BIT(1)
	};

	struct CheckboxWidget : public Widget
	{
	public:
		CheckboxWidget() : Widget() {};
	public:
		void RenderCheckbox();
	public:
		FixedString16 m_Label;
		WidgetFlags m_Flags{ Checkbox_LeftLean };
		bool m_CurrentBoolean{ false };
		std::function<void(CheckboxWidget&)> m_ConfirmAction;
		Ref<void> m_ProvidedData{ nullptr };
	private:
		bool m_Editing{ false };
	};
}