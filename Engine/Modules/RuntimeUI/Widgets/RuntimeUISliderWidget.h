#pragma once

#include "Modules/RuntimeUI/Widgets/RuntimeUIWidget.h"

namespace Kargono::RuntimeUI
{
	class SliderWidget : public Widget
	{
	public:
		//============================
		// Constructors/Destructors
		//============================
		SliderWidget(UserInterface* parentInterface)
			: Widget(parentInterface)
		{
			m_WidgetType = WidgetTypes::SliderWidget;
			m_Tag = "SliderWidget";
		}
		virtual ~SliderWidget() override = default;
	public:
		//============================
		// Rendering Methods
		//============================
		virtual void OnRender(RuntimeUIContext* uiContext, Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth) override;
	private:
		// Rendering helper functions
		void RenderSliderLine(RuntimeUIContext* uiContext, const Math::vec4& color, const Math::vec3& translation, const Math::vec3& size);
		void RenderSlider(RuntimeUIContext* uiContext, const Math::vec4& color, const Math::vec3& translation, const Math::vec3& size);

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
	public:
		//============================
		// Public Fields
		//============================
		SelectionData m_SelectionData;
		Math::vec2 m_Bounds{ 0.0f, 1.0f };
		Assets::AssetHandle m_OnMoveSliderHandle{ Assets::EmptyHandle };
		Ref<Scripting::Script> m_OnMoveSlider{ nullptr };
		Math::vec4 m_SliderColor{ 1.0f };
		Math::vec4 m_LineColor{ 1.0f };

		// Runtime Value
		float m_CurrentValue{ 0.0f };
	};
}