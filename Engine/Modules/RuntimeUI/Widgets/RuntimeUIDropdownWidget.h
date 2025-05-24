#pragma once

#include "Modules/RuntimeUI/Widgets/RuntimeUIWidget.h"

namespace Kargono::RuntimeUI
{
	class DropDownWidget : public Widget
	{
	public:
		//============================
		// Constructors/Destructors
		//============================
		DropDownWidget()
			: Widget()
		{
			m_WidgetType = WidgetTypes::DropDownWidget;
			m_Tag = "DropDownWidget";
		}
		virtual ~DropDownWidget() override = default;
	public:
		//============================
		// Rendering Methods
		//============================
		virtual void OnRender(Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth) override;

		//============================
		// Query State
		//============================
		virtual bool Selectable() override
		{
			return m_SelectionData.m_Selectable;
		}

		//============================
		// Re-validation Methods
		//============================
		void CalculateTextSize();
	public:
		//============================
		// Public Fields
		//============================
		SelectionData m_SelectionData{};
		std::vector<SingleLineTextData> m_DropDownOptions{};
		Math::vec4 m_DropDownBackground{ 1.0f };
		Assets::AssetHandle m_OnSelectOptionHandle{ Assets::EmptyHandle };
		Ref<Scripting::Script> m_OnSelectOption{ nullptr };

		// Runtime Values
		size_t m_CurrentOption{ 0 };
		bool m_DropDownOpen{ false };
	};
}