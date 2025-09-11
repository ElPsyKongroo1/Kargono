#pragma once
#include "Modules/EditorUI/Widgets/EditorUIWidget.h"

#include "Kargono/Core/Base.h"
#include "Kargono/Core/FixedBufferString.h"

#include <functional>

namespace Kargono::EditorUI
{
	enum EditTextFlags : WidgetFlags
	{
		EditText_None = 0,
		EditText_PopupOnly = BIT(0), // Only use a popup and remove inline text
		EditText_Indented = BIT(1) // Display indented
	};

	struct EditTextSpec : public Widget
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		EditTextSpec() : Widget() {}
		~EditTextSpec() = default;
	public:
		//==============================
		// Rendering
		//==============================
		void RenderText();
	public:
		//==============================
		// Public Fields
		//==============================
		FixedBufStr32 m_Label;
		WidgetFlags m_Flags{ EditText_None };
		std::string m_CurrentOption{};
		std::function<void(EditTextSpec&)> m_ConfirmAction;
		bool m_StartPopup{ false };
		Ref<void> m_ProvidedData{ nullptr };
	private:
		//==============================
		// Internal Fields
		//==============================
		FixedBufStr256 m_Buffer;
	};
}

