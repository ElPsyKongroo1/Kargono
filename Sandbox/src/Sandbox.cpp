#include <Kargono.h>
#include "Kargono/Core/EntryPoint.h"
#include "Sandbox2D.h"

class Sandbox : public Kargono::Application 
{
public:
	Sandbox() 
	{
		//PushLayer(new ExampleLayer()); 
		PushLayer(new Sandbox2D()); 
	}
	~Sandbox() 
	{
		
	}
};

Kargono::Application* Kargono::CreateApplication()
{
	return new Sandbox();
}