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
		ImageWidget()
			: Widget()
		{
			m_WidgetType = WidgetTypes::ImageWidget;
			m_Tag = "ImageWidget";
		}
		virtual ~ImageWidget() override = default;

	public:
		//============================
		// Query State
		//============================
		virtual bool Selectable() override
		{
			return false;
		}

	public:
		//============================
		// Rendering Methods
		//============================
		virtual void OnRender(Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth) override;

	public:
		//============================
		// Public Fields
		//============================
		ImageData m_ImageData;
	};
}