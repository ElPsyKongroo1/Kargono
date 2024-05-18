#pragma once

#include "Kargono/Core/Timestep.h"
#include "Kargono/Events/Event.h"

namespace Kargono 
{
	class Application
	{
	public:
		Application(const std::string& name = "Layer");
		virtual ~Application() = default;

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(Timestep ts) {}
		virtual void OnEditorUIRender() {}
		virtual void OnEvent(Events::Event& event) {}

		const std::string& GetName() const { return m_DebugName; }
	protected:
		std::string m_DebugName;

	};
}
