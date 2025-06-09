#pragma once

#include "Modules/RuntimeUI/Widgets/RuntimeUIWidget.h"

namespace Kargono::RuntimeUI
{
	class ImageButtonWidget : public Widget
	{
	public:
		//============================
		// Constructors/Destructors
		//============================
		ImageButtonWidget(UserInterface* parentInterface)
			: Widget(parentInterface)
		{
			m_WidgetType = WidgetTypes::ImageButtonWidget;
			m_Tag = "ImageButtonWidget";
		}
		virtual ~ImageButtonWidget() override = default;
	public:
		//============================
		// Rendering Methods
		//============================
		virtual void OnRender(RuntimeUIContext* uiContext, Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth) override;

	public:
		//============================
		// Query State
		//============================
		virtual bool Selectable() override { return m_SelectionData.m_Selectable; }

	public:
		//============================
		// Getters/Setters
		//============================
		virtual SelectionData* GetSelectionData() override { return &m_SelectionData; }
		virtual ImageData* GetImageData() override { return &m_ImageData; }

	public:
		//============================
		// Public Fields
		//============================
		ImageData m_ImageData;
		SelectionData m_SelectionData;
	};
}