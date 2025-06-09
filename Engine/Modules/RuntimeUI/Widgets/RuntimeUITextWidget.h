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
		TextWidget(UserInterface* parentInterface)
			: Widget(parentInterface)
		{
			m_WidgetType = WidgetTypes::TextWidget;
			m_Tag = "TextWidget";
		}
		virtual ~TextWidget() override = default;
	public:
		//============================
		// Rendering Methods
		//============================
		virtual void OnRender(RuntimeUIContext* uiContext, Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth) override;
	public:
		//============================
		// Modify State
		//============================
		void SetText(const std::string& newText);

	public:
		//============================
		// Getters/Setters
		//============================
		virtual MultiLineTextData* GetMultiLineTextData() override { return &m_TextData; }

	public:
		//============================
		// Revalidation Methods
		//============================
		virtual void RevalidateTextDimensions() override;
	public:
		//============================
		// Public Fields
		//============================
		MultiLineTextData m_TextData;
	};
}