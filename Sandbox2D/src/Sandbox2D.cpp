#include <Kargono.h>

class Breakout : public Kargono::Application 
{
public:
	Breakout() 
	{
		
	}
	~Breakout() 
	{
		
	}
};


Kargono::Application* Kargono::CreateApplication()
{
	return new Breakout();
}