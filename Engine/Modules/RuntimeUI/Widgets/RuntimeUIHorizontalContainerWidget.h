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
		HorizontalContainerWidget(UserInterface* parentInterface)
			: Widget(parentInterface)
		{
			m_WidgetType = WidgetTypes::HorizontalContainerWidget;
			m_Tag = "HorizontalContainerWidget";
		}
		virtual ~HorizontalContainerWidget() override = default;
	public:
		//============================
		// Rendering Methods
		//============================
		virtual void OnRender(RuntimeUIContext* uiContext, Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth) override;

	public:
		//============================
		// Getters/Setters
		//============================
		virtual ContainerData* GetContainerData() override { return &m_ContainerData; }

	public:
		//============================
		// Public Fields
		//============================
		ContainerData m_ContainerData;
		float m_ColumnWidth{ 0.25f };
		float m_ColumnSpacing{ 0.0f };
	};
}