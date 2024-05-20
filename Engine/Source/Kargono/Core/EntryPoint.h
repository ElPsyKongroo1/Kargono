#pragma once
#include "Kargono/Core/Base.h"
#include "Kargono/Core/EngineCore.h"
#include "Kargono/Core/Timers.h"

#ifdef KG_PLATFORM_WINDOWS

#include <Windows.h>
#include <shellapi.h>
	
	extern Kargono::EngineCore* Kargono::InitEngineAndCreateApp(CommandLineArgs args);

	void EntryPoint (int argc, char** argv)
	{
		Kargono::Log::Init();

		Kargono::EngineCore* core = Kargono::InitEngineAndCreateApp({ argc, argv });

		core->Run();
		Kargono::Timers::AsyncBusyTimer::CloseAllTimers();

		delete core;
	}

	int main(int argc, char** argv)
	{
		EntryPoint(argc, argv);
		KG_WARN("Application Shut Down Successfully!");
		return 0;
	}

	int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
	{
		// Get Command Line Arguments in the form of argc and argv line int main(char**, int)
		LPWSTR* argv;
		int argc;
		argv = CommandLineToArgvW(GetCommandLine(), &argc);

		EntryPoint(argc, (char**)argv);

		LocalFree(argv);

		KG_WARN("Application Shut Down Successfully!");
		return 0;

	}
#endif
