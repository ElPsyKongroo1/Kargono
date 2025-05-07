#pragma once

#include "Kargono/Core/Timestep.h"
#include "EventModule/Event.h"
#include "Kargono/Core/Window.h"

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
		Application(const char* name = "Layer");
		virtual ~Application() = default;

		//==============================
		// LifeCycle Functions
		//==============================
		[[nodiscard]] virtual bool Init() = 0;
		[[nodiscard]] virtual bool Terminate() = 0;

		//==============================
		// Event Functions
		//==============================
		virtual void OnUpdate(Timestep ts) 
		{
			UNREFERENCED_PARAMETER(ts);
		}
		virtual void OnEditorUIRender() {}
		virtual bool OnInputEvent(Events::Event* event) 
		{ 
			UNREFERENCED_PARAMETER(event);
			return false; 
		}
		virtual bool OnPhysicsEvent(Events::Event* event) 
		{ 
			UNREFERENCED_PARAMETER(event);
			return false; 
		}
		virtual bool OnApplicationEvent(Events::Event* event) 
		{ 
			UNREFERENCED_PARAMETER(event);
			return false; 
		}
		virtual bool OnNetworkEvent(Events::Event* event) 
		{ 
			UNREFERENCED_PARAMETER(event);
			return false; 
		}
		virtual bool OnSceneEvent(Events::Event* event) 
		{ 
			UNREFERENCED_PARAMETER(event);
			return false; 
		}
		virtual bool OnAssetEvent(Events::Event* event) 
		{ 
			UNREFERENCED_PARAMETER(event);
			return false; 
		}
		virtual bool OnLogEvent(Events::Event* event) 
		{ 
			UNREFERENCED_PARAMETER(event);
			return false; 
		}
		virtual bool OnEditorEvent(Events::Event* event) 
		{
			UNREFERENCED_PARAMETER(event);
			return false; 
		}

		//==============================
		// Query Application State
		//==============================
		virtual Math::vec2 GetMouseViewportPosition() = 0;
		virtual ViewportData* GetViewportData() = 0;

		//==============================
		// Getter/Setter
		//==============================
		const char* GetName() const { return m_DebugName; }
	protected:
		FixedString32 m_DebugName;

	};
}
