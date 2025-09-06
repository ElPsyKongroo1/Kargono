#pragma once

#include "Modules/RuntimeUI/Widgets/RuntimeUIWidget.h"

namespace Kargono::RuntimeUI
{
	//============================
	// Checkbox Widget Class (Derived)
	//============================
	class CheckboxWidget : public Widget
	{
	public:
		//============================
		// Constructors/Destructors
		//============================
		CheckboxWidget(UserInterface* parentInterface)
			: Widget(parentInterface)
		{
			m_WidgetType = WidgetTypes::CheckboxWidget;
			m_Tag = "CheckboxWidget";
		}
		virtual ~CheckboxWidget() override = default;
	public:
		//============================
		// Rendering Methods
		//============================
		virtual void OnRender(RuntimeUIContext* uiContext, Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth) override;

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
		// Getters/Setters
		//============================
		virtual SelectionData* GetSelectionData() override { return &m_SelectionData; }
		virtual ImageData* GetImageData() override;

	public:
		//============================
		// Public Fields
		//============================
		ImageData m_ImageChecked;
		ImageData m_ImageUnChecked;
		SelectionData m_SelectionData;
		bool m_Checked{ false };
	};
}