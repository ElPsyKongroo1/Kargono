#pragma once
#include "Modules/EditorUI/Widgets/EditorUIWidget.h"

#include "Kargono/Core/Base.h"
#include "Kargono/Core/FixedString.h"

#include <functional>

namespace Kargono::EditorUI
{
	enum EditMultiLineTextFlags : WidgetFlags
	{
		EditMultiLineText_None = 0,
		EditMultiLineText_PopupOnly = BIT(0), // Only use a popup and remove inline text
		EditMultiLineText_Indented = BIT(1) // Display indented
	};

	struct EditMultiLineTextWidget : public Widget
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		EditMultiLineTextWidget() : Widget() {}
		~EditMultiLineTextWidget() = default;
	public:
		//==============================
		// Rendering
		//==============================
		void RenderText();
	public:
		//==============================
		// Public Fields
		//==============================
		FixedString32 m_Label;
		WidgetFlags m_Flags{ EditMultiLineText_None };
		std::string m_CurrentOption{};
		std::function<void(EditMultiLineTextWidget&)> m_ConfirmAction;
		bool m_StartPopup{ false };
		Ref<void> m_ProvidedData{ nullptr };
	private:
		//==============================
		// Internal Fields
		//==============================
		FixedString1024 m_Buffer{};
	};
}


