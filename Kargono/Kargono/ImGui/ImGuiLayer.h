#pragma once

#include "Kargono/Core/Layer.h"
#include "Kargono/Events/ApplicationEvent.h"
#include "Kargono/Events/KeyEvent.h"
#include "Kargono/Events/MouseEvent.h"


namespace Kargono 
{
	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer() = default;


		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& e) override;

		void Begin();
		void End();
		
		void BlockEvents(bool block) { m_BlockEvents = block; }

		void SetDarkThemeColors();

		uint32_t GetActiveWidgetID() const;
	private:
		bool m_BlockEvents = true;

	};
}
