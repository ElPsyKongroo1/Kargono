#pragma once

#include "Kargono/Layer.h"
#include "Kargono/Events/ApplicationEvent.h"
#include "Kargono/Events/KeyEvent.h"
#include "Kargono/Events/MouseEvent.h"


namespace Kargono 
{
	class KG_API ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();


		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnImGuiRender() override;

		void Begin();
		void End();
		


	private:
		float m_Time = 0.0f;

	};
}