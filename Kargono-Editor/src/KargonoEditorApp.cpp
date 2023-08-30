#include <Kargono.h>
#include "Kargono/Core/EntryPoint.h"
#include "EditorLayer.h"

namespace Kargono {

	class KargonoEditor : public Application
	{
	public:
		KargonoEditor(const ApplicationSpecification& spec)
			: Application(spec)
		{
			//PushLayer(new ExampleLayer()); 
			PushLayer(new EditorLayer());
		}
		~KargonoEditor()
		{

		}
	};

	Application* CreateApplication(ApplicationCommandLineArgs args)
	{
		ApplicationSpecification spec;
		spec.Name = "Hazelnut";
		spec.CommandLineArgs = args;

		return new KargonoEditor(spec);
	}
}
