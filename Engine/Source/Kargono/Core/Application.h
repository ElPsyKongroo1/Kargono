#pragma once

#include "Kargono/Core/Timestep.h"
#include "Kargono/Events/Event.h"

namespace Kargono 
{

	//==============================
	// Application Class
	//==============================
	class Application
	{
	public:
		//==============================
		// Constructor/Destructor
		//==============================
		Application(const std::string& name = "Layer");
		virtual ~Application() = default;

		//==============================
		// LifeCycle Functions
		//==============================
		virtual void Init() {}
		virtual void Terminate() {}

		//==============================
		// Event Functions
		//==============================
		virtual void OnUpdate(Timestep ts) {}
		virtual void OnEditorUIRender() {}
		virtual bool OnInputEvent(Events::Event* event) { return false; };
		virtual bool OnPhysicsEvent(Events::Event* event) { return false; };
		virtual bool OnApplicationEvent(Events::Event* event) { return false; };
		virtual bool OnNetworkEvent(Events::Event* event) { return false; };
		virtual bool OnSceneEvent(Events::Event* event) { return false; };

		//==============================
		// Getter/Setter
		//==============================
		const std::string& GetName() const { return m_DebugName; }
	protected:
		std::string m_DebugName;

	};
}
