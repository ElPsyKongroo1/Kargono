#include <Kargono.h>
#include "Kargono/Core/EntryPoint.h"

#include "RuntimeLayer.h"

namespace Kargono {

	class Runtime : public Application
	{
	public:
		Runtime(const ApplicationSpecification& spec)
			: Application(spec)
		{
			PushLayer(new RuntimeLayer());
		}
		~Runtime() = default;
	};

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
