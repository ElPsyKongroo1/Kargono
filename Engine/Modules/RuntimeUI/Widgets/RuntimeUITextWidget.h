#pragma once
#include "Modules/RuntimeUI/Widgets/RuntimeUIWidget.h"

namespace Kargono::RuntimeUI
{
	class TextWidget : public Widget
	{
	public:
		//============================
		// Constructors/Destructors
		//============================
		TextWidget()
			: Widget()
		{
			m_WidgetType = WidgetTypes::TextWidget;
			m_Tag = "TextWidget";
		}
		virtual ~TextWidget() override = default;
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
			return false;
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
		MultiLineTextData m_TextData;
	private:
		friend class RuntimeUIService;
		friend class Assets::UserInterfaceManager;
	};
}