#pragma once

#include "Modules/RuntimeUI/Widgets/RuntimeUIWidget.h"

namespace Kargono::RuntimeUI
{
	class ButtonWidget : public Widget
	{
	public:
		//============================
		// Constructors/Destructors
		//============================
		ButtonWidget(UserInterface* parentInterface)
			: Widget(parentInterface)
		{
			m_WidgetType = WidgetTypes::ButtonWidget;
			m_Tag = "ButtonWidget";
		}
		virtual ~ButtonWidget() override = default;
	public:
		//============================
		// Rendering Methods
		//============================
		virtual void OnRender(RuntimeUIContext* uiContext, Math::vec3 windowTranslation, const Math::vec3& windowSize,
			float viewportWidth) override;

	public:
		//============================
		// Modify State
		//============================
		void SetText(const std::string& newText);

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
		virtual SingleLineTextData* GetSingleLineTextData() override { return &m_TextData; }

	public:
		//============================
		// Re-validation Methods
		//============================
		virtual void RevalidateTextDimensions() override;
	public:
		//============================
		// Public Fields
		//============================
		SingleLineTextData m_TextData;
		SelectionData m_SelectionData;
	};
}