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
		ButtonWidget()
			: Widget()
		{
			m_WidgetType = WidgetTypes::ButtonWidget;
			m_Tag = "ButtonWidget";
		}
		virtual ~ButtonWidget() override = default;
	public:
		//============================
		// Rendering Methods
		//============================
		virtual void OnRender(Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth) override;

	public:
		//============================
		// Modify State
		//============================
		void SetText(const std::string& newText);

		//============================
		// Query State
		//============================
		virtual bool Selectable() override
		{
			return m_SelectionData.m_Selectable;
		}

	public:
		//============================
		// Re-validation Methods
		//============================
		void CalculateTextSize();
	public:
		//============================
		// Public Fields
		//============================
		SingleLineTextData m_TextData;
		SelectionData m_SelectionData;
	private:
		friend class RuntimeUIService;
		friend class Assets::UserInterfaceManager;
	};
}