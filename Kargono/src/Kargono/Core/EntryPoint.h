#pragma once
#include "Kargono/Core/Base.h"
#include "Kargono/Core/Application.h"

#ifdef KG_PLATFORM_WINDOWS
	
	extern Kargono::Application* Kargono::CreateApplication(ApplicationCommandLineArgs args);

	int main(int argc, char** argv)
	{
		Kargono::Log::Init();

		auto app = Kargono::CreateApplication({ argc, argv });

		app->Run();

		delete app;
	}
#endif
