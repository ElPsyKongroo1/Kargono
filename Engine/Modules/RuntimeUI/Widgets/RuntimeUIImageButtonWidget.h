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
		ImageButtonWidget()
			: Widget()
		{
			m_WidgetType = WidgetTypes::ImageButtonWidget;
			m_Tag = "ImageButtonWidget";
		}
		virtual ~ImageButtonWidget() override = default;
	public:
		//============================
		// Rendering Methods
		//============================
		virtual void OnRender(Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth) override;

	public:
		//============================
		// Query State
		//============================
		virtual bool Selectable() override
		{
			return m_SelectionData.m_Selectable;
		}

	public:
		//============================
		// Public Fields
		//============================
		ImageData m_ImageData;
		SelectionData m_SelectionData;
	private:
		friend class RuntimeUIService;
		friend class Assets::UserInterfaceManager;
	};
}