#pragma once
#include "Modules/EditorUI/Widgets/EditorUIWidget.h"

#include "Kargono/Core/Base.h"
#include "Kargono/Core/FixedString.h"

#include <functional>

namespace Kargono::EditorUI
{
	struct GenericPopupWidget : public Widget
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		GenericPopupWidget() : Widget() {}
		~GenericPopupWidget() = default;

	public:
		//==============================
		// Rendering
		//==============================
		void RenderPopup();
	public:
		//==============================
		// Interact w/ Popup
		//==============================
		void CloseActivePopup();
	public:
		//==============================
		// Public Fields
		//==============================
		FixedString32 m_Label;
		float m_PopupWidth{ 700.0f };
		std::function<void()> m_PopupContents{ nullptr };
		std::function<void()> m_ConfirmAction{ nullptr };
		std::function<void()> m_DeleteAction{ nullptr };
		std::function<void()> m_PopupAction{ nullptr };
		std::function<void()> m_CancelAction{ nullptr };
		bool m_OpenPopup{ false };
	private:
		//==============================
		// Internal Fields
		//==============================
		bool m_CloseActivePopup{ false };
	};
}

