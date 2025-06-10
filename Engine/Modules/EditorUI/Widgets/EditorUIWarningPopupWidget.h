#pragma once
#include "Modules/EditorUI/Widgets/EditorUIWidget.h"

#include "Kargono/Core/Base.h"
#include "Kargono/Core/FixedString.h"

#include <functional>

namespace Kargono::EditorUI
{
	struct WarningPopupWidget : public Widget
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		WarningPopupWidget() : Widget() {}
		~WarningPopupWidget() = default;

	public:
		//==============================
		// Rendering
		//==============================
		void RenderPopup();
	public:
		//==============================
		// Public Fields
		//==============================
		FixedString32 m_Label;
		float m_PopupWidth{ 700.0f };
		std::function<void()> m_PopupContents{ nullptr };
		bool m_OpenPopup{ false };
	};
}

