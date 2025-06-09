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
		InputTextWidget(UserInterface* parentInterface)
			: Widget(parentInterface)
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

	public:
		//============================
		// Modify State
		//============================
		void SetText(const std::string& newText);

	public:
		//============================
		// Revalidation Methods
		//============================
		virtual void RevalidateTextDimensions() override;
	public:
		//============================
		// Rendering Methods
		//============================
		virtual void OnRender(RuntimeUIContext* uiContext, Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth) override;

	public:
		//============================
		// Getters/Setters
		//============================
		virtual SelectionData* GetSelectionData() override { return &m_SelectionData; }
		virtual SingleLineTextData* GetSingleLineTextData() override { return &m_TextData; }

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