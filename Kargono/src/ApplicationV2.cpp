#include "src/kgpch.h"
#include "ApplicationV2.h"
#include "Events/ApplicationEvent.h"
#include "Log.h"


namespace Kargono
{

	ApplicationV2::ApplicationV2()
	{
		
	}
	ApplicationV2::~ApplicationV2()
	{
	
	}

	void ApplicationV2::Run()
	{
		WindowResizeEvent e(1280, 720);
		KG_TRACE(e);
		while (true);
	}

}