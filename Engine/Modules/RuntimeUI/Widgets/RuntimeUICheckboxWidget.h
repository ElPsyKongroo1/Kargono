#pragma once

#include "Modules/RuntimeUI/Widgets/RuntimeUIWidget.h"

namespace Kargono::RuntimeUI
{
	//============================
	// Checkbox Widget Class (Derived)
	//============================
	class CheckboxWidget : public Widget
	{
	public:
		//============================
		// Constructors/Destructors
		//============================
		CheckboxWidget()
			: Widget()
		{
			m_WidgetType = WidgetTypes::CheckboxWidget;
			m_Tag = "CheckboxWidget";
		}
		virtual ~CheckboxWidget() override = default;
	public:
		//============================
		// Rendering Methods
		//============================
		virtual void OnRender(Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth) override;

		//============================
		// Query State
		//============================
		virtual bool Selectable() override
		{
			return m_SelectionData.m_Selectable;
		}

		//============================
		// Public Fields
		//============================
		ImageData m_ImageChecked;
		ImageData m_ImageUnChecked;
		SelectionData m_SelectionData;
		bool m_Checked{ false };
	};
}