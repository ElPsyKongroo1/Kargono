#pragma once
#include "Modules/EditorUI/Widgets/EditorUIWidget.h"
#include "Modules/EditorUI/Widgets/EditorUISelectOptionWidget.h"

#include "Kargono/Core/Base.h"
#include "Kargono/Core/FixedString.h"
#include "Kargono/Core/UUID.h"

#include <functional>

namespace Kargono::EditorUI
{

	enum DropDownFlags : WidgetFlags
	{
		DropDown_None = 0,
		DropDown_Indented = BIT(0), // Indents the text (used in collapsing headers usually)
	};

	struct DropDownWidget : public Widget
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		DropDownWidget() : Widget() {}
		~DropDownWidget() = default;
	public:
		//==============================
		// Rendering
		//==============================
		void RenderDropDown();
	public:
		//==============================
		// Modify Option List
		//==============================
		OptionEntry* CreateOption();
		bool SetCurrentOption(UUID handle);
	public:
		//==============================
		// Public Fields
		//==============================
		FixedString32 m_Label{};
		WidgetFlags m_Flags{ DropDown_None };
		std::function<void(const OptionEntry&)> m_ConfirmAction{ nullptr };
		Ref<void> m_ProvidedData{ nullptr };
	private:
		//==============================
		// Internal Fields
		//==============================
		OptionIndex m_CurrentOption{ k_InvalidEntryIndex };
		OptionList m_OptionsList{};
	};
}



