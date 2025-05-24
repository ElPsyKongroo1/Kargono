#pragma once

#include "Modules/RuntimeUI/Widgets/RuntimeUIWidget.h"

namespace Kargono::RuntimeUI
{
	class InputTextWidget : public Widget
	{
	public:
		//============================
		// Constructors/Destructors
		//============================
		InputTextWidget()
			: Widget()
		{
			m_WidgetType = WidgetTypes::InputTextWidget;
			m_Tag = "InputTextWidget";
		}
		virtual ~InputTextWidget() override = default;

	public:
		//============================
		// Query State
		//============================
		virtual bool Selectable() override
		{
			return m_SelectionData.m_Selectable;
		}
		//============================
		// Modify State
		//============================
		void SetText(const std::string& newText);

	public:
		//============================
		// Re-validation Methods
		//============================
		void CalculateTextSize();
	public:
		//============================
		// Rendering Methods
		//============================
		virtual void OnRender(Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth) override;

	public:
		//============================
		// Public Fields
		//============================
		SingleLineTextData m_TextData;
		SelectionData m_SelectionData;
		Assets::AssetHandle m_OnMoveCursorHandle{ Assets::EmptyHandle };
		Ref<Scripting::Script> m_OnMoveCursor{ nullptr };
	};
}