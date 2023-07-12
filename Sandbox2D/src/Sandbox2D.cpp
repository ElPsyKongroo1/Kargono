#include <Kargono.h>

class ExampleLayer : public Kargono::Layer
{
public:
	ExampleLayer() : Layer("Example") {}

	void OnUpdate() override
	{
		KG_INFO("ExampleLayer::Update");
	}

	void OnEvent(Kargono::Event& event) override 
	{
		KG_TRACE("{0}", event);
	}
};

class Sandbox2D : public Kargono::Application 
{
public:
	Sandbox2D() 
	{
		PushLayer(new ExampleLayer()); 
		PushOverlay(new Kargono::ImGuiLayer());
	}
	~Sandbox2D() 
	{
		
	}
};

Kargono::Application* Kargono::CreateApplication()
{
	return new Sandbox2D();
}