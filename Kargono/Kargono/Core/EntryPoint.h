#pragma once
#include "Kargono/Core/Base.h"

#ifdef KG_PLATFORM_WINDOWS
	
	extern Kargono::Application* Kargono::CreateApplication();

	int main(int argc, char** argv)
	{
		Kargono::Log::Init();

		KG_PROFILE_BEGIN_SESSION("Startup", "KargonoProfile-Startup.json");
		auto app = Kargono::CreateApplication();
		KG_PROFILE_END_SESSION();

		KG_PROFILE_BEGIN_SESSION("Runtime", "KargonoProfile-Runtime.json");
		app->Run();
		KG_PROFILE_END_SESSION();

		KG_PROFILE_BEGIN_SESSION("Shutdown", "KargonoProfile-Shutdown.json");
		delete app;
		KG_PROFILE_END_SESSION();
	}
#endif