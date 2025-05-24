#pragma once

#include "Modules/RuntimeUI/Widgets/RuntimeUIWidget.h"

namespace Kargono::RuntimeUI
{
	class HorizontalContainerWidget : public Widget
	{
	public:
		//============================
		// Constructors/Destructors
		//============================
		HorizontalContainerWidget()
			: Widget()
		{
			m_WidgetType = WidgetTypes::HorizontalContainerWidget;
			m_Tag = "HorizontalContainerWidget";
		}
		virtual ~HorizontalContainerWidget() override = default;
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
			return false;
		}

		//============================
		// Public Fields
		//============================
		ContainerData m_ContainerData;
		float m_ColumnWidth{ 0.25f };
		float m_ColumnSpacing{ 0.0f };
	};
}