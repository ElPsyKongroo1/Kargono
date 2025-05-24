#pragma once

#include "Modules/RuntimeUI/Widgets/RuntimeUIWidget.h"

namespace Kargono::RuntimeUI
{
	class ContainerWidget : public Widget
	{
	public:
		//============================
		// Constructors/Destructors
		//============================
		ContainerWidget()
			: Widget()
		{
			m_WidgetType = WidgetTypes::ContainerWidget;
			m_Tag = "FrameContainerWidget";
		}
		virtual ~ContainerWidget() override = default;
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
	};
}