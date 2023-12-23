#include <Kargono.h>
#include "Kargono/Core/EntryPoint.h"

#include "RuntimeLayer.h"

namespace Kargono {

	//============================================================
	// Runtime Class
	//============================================================
	// This class represents the actual runtime application. The runtime
	//		application holds layers that can further subdivide the application.
	class Runtime : public Application
	{
	public:
		//==========================
		// Constructors and Destructors
		//==========================

		// This constructor calls its parent constructor and pushes
		//		the RuntimeLayer onto its Layer Stack. This initializes
		//		the RuntimeLayer and calls OnAttach().
		Runtime(const ApplicationSpecification& spec)
			: Application(spec)
		{
			PushLayer(new RuntimeLayer());
		}
		~Runtime() = default;
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
		spec.Name = "Runtime";
		spec.CommandLineArgs = args;
		spec.WorkingDirectory = std::filesystem::current_path();
		spec.Width = 1600;
		spec.Height = 900;

		return new Runtime(spec);
	}
}
