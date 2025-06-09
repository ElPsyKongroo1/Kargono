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
		ContainerWidget(UserInterface* parentInterface)
			: Widget(parentInterface)
		{
			m_WidgetType = WidgetTypes::ContainerWidget;
			m_Tag = "FrameContainerWidget";
		}
		virtual ~ContainerWidget() override = default;
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
	};
}