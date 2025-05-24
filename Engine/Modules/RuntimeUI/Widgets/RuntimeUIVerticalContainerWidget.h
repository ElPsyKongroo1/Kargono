#pragma once

#include "Modules/RuntimeUI/Widgets/RuntimeUIWidget.h"

namespace Kargono::RuntimeUI
{
	class VerticalContainerWidget : public Widget
	{
	public:
		//============================
		// Constructors/Destructors
		//============================
		VerticalContainerWidget()
			: Widget()
		{
			m_WidgetType = WidgetTypes::VerticalContainerWidget;
			m_Tag = "VerticalContainerWidget";
		}
		virtual ~VerticalContainerWidget() override = default;
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
		float m_RowHeight{ 0.25f };
		float m_RowSpacing{ 0.0f };
	};
}