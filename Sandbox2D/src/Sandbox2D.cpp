#include <Kargono.h>

class Sandbox2D : public Kargono::Application 
{
public:
	Sandbox2D() 
	{
		
	}
	~Sandbox2D() 
	{
		
	}
};


Kargono::Application* Kargono::CreateApplication()
{
	return new Sandbox2D();
}