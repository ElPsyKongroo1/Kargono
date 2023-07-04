#include <Kargono.h>

class Breakout : public Kargono::ApplicationV2 
{
public:
	Breakout() 
	{
		
	}
	~Breakout() 
	{
		
	}
};


Kargono::ApplicationV2* Kargono::CreateApplication()
{
	return new Breakout();
}