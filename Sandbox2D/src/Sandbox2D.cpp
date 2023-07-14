#include <Kargono.h>

class ExampleLayer : public Kargono::Layer
{
public:
	ExampleLayer() : Layer("Example") {}

	void OnUpdate() override
	{
		if (Kargono::Input::IsKeyPressed(KG_KEY_TAB)) { KG_TRACE("Tab key is pressed!"); }
	}

	void OnEvent(Kargono::Event& event) override 
	{
		Kargono::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<Kargono::KeyTypedEvent>([](Kargono::KeyTypedEvent& newEvent)
		{
			KG_TRACE("{0}", (char)newEvent.GetKeyCode());
			return true;
		});
		//KG_TRACE("{0}", event); FIXME
	}
};

class Sandbox2D : public Kargono::Application 
{
public:
	Sandbox2D() 
	{
		PushLayer(new ExampleLayer()); 
	}
	~Sandbox2D() 
	{
		
	}
};

Kargono::Application* Kargono::CreateApplication()
{
	return new Sandbox2D();
}