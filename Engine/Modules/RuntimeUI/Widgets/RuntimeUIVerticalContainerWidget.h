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
		VerticalContainerWidget(UserInterface* parentInterface)
			: Widget(parentInterface)
		{
			m_WidgetType = WidgetTypes::VerticalContainerWidget;
			m_Tag = "VerticalContainerWidget";
		}
		virtual ~VerticalContainerWidget() override = default;
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
		float m_RowHeight{ 0.25f };
		float m_RowSpacing{ 0.0f };
	};
}