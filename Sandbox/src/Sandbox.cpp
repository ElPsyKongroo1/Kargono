#include <Kargono.h>
#include "Kargono/Core/EntryPoint.h"
#include "Sandbox2D.h"

class Sandbox : public Kargono::Application 
{
public:
	Sandbox(const Kargono::ApplicationSpecification& specification)
		: Kargono::Application(specification)
	{
		//PushLayer(new ExampleLayer()); 
		PushLayer(new Sandbox2D()); 
	}
	~Sandbox() 
	{
		
	}
};

Kargono::Application* Kargono::CreateApplication(Kargono::ApplicationCommandLineArgs args)
{
	ApplicationSpecification spec;
	spec.Name = "Sandbox";
	spec.WorkingDirectory = "../Kargono-Editor";
	spec.CommandLineArgs = args;

	return new Sandbox(spec);
}
