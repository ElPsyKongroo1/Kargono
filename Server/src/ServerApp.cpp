#include <Kargono.h>
#include "Kargono/Core/EntryPoint.h"

#include "ServerLayer.h"

namespace Kargono {

	//============================================================
	// Server Class
	//============================================================
	// This class represents the actual runtime application. The runtime
	//		application holds layers that can further subdivide the application.
	class ServerApp : public Application
	{
	public:
		//==========================
		// Constructors and Destructors
		//==========================

		// This constructor calls its parent constructor and pushes
		//		the ServerLayer onto its Layer Stack. This initializes
		//		the ServerLayer and calls OnAttach().
		ServerApp(const ApplicationSpecification& spec)
			: Application(spec)
		{
			PushLayer(new ServerLayer());
		}
		~ServerApp() = default;
	};

	//============================================================
	// CreateApplication Function
	//============================================================
	// This function is defined in the engine in Core/Application.h.
	//		This function is linked by the linker and provides an external
	//		method for starting the application.
	Application* CreateApplication(ApplicationCommandLineArgs args)
	{
		ApplicationSpecification spec;
		spec.Name = "Server";
		spec.CommandLineArgs = args;
		spec.WorkingDirectory = std::filesystem::current_path();
		spec.Width = 0;
		spec.Height = 0;

		return new ServerApp(spec);
	}
}
