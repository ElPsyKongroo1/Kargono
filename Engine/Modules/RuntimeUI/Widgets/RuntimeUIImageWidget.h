#pragma once

#include "Modules/RuntimeUI/Widgets/RuntimeUIWidget.h"

namespace Kargono::RuntimeUI
{
	class ImageWidget : public Widget
	{
	public:
		//============================
		// Constructors/Destructors
		//============================
		ImageWidget(UserInterface* parentInterface)
			: Widget(parentInterface)
		{
			m_WidgetType = WidgetTypes::ImageWidget;
			m_Tag = "ImageWidget";
		}
		virtual ~ImageWidget() override = default;

	public:
		//============================
		// Rendering Methods
		//============================
		virtual void OnRender(RuntimeUIContext* uiContext, Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth) override;

	public:
		//============================
		// Getters/Setters
		//============================
		virtual ImageData* GetImageData() override { return &m_ImageData; }

	public:
		//============================
		// Public Fields
		//============================
		ImageData m_ImageData;
	};
}