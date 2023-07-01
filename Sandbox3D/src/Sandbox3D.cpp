#include <Kargono.h>

class Breakout : public Karg::ApplicationV2 
{
public:
	Breakout() 
	{
		
	}
	~Breakout() 
	{
		
	}
};


Karg::ApplicationV2* Karg::CreateApplication()
{
	return new Breakout();
}