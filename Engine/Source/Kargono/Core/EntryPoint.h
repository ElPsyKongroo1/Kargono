#pragma once
#include "Kargono/Core/Base.h"
#include "Kargono/Core/EngineCore.h"
#include "Kargono/Utility/Timers.h"

#ifdef KG_PLATFORM_WINDOWS
#include "API/Platform/WindowsBackendAPI.h"
	
	extern Kargono::EngineCore* Kargono::InitEngineAndCreateApp(CommandLineArgs args);
	//==============================
	// General Entry Point
	//==============================
	void EntryPoint (int argc, char** argv)
	{
		Kargono::Log::Init();
		KG_INFO("Starting Application");
		Kargono::EngineCore* core = Kargono::InitEngineAndCreateApp({ argc, argv });
		KG_VERIFY(core, "Engine Core Initialization");
		core->Run();
		Kargono::Utility::AsyncBusyTimer::CloseAllTimers();
		delete core;
		core = nullptr;
		KG_VERIFY(!core, "Core Closed");
	}
	//==============================
	// Main with console
	//==============================
	int main(int argc, char** argv)
	{
		EntryPoint(argc, argv);
		KG_INFO("Application Shut Down Successfully!");
		return 0;
	}
	//==============================
	// Main without console
	//==============================
	int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
	{
		// Get Command Line Arguments in the form of argc and argv line int main(char**, int)
		LPWSTR* argv;
		int argc;
		argv = CommandLineToArgvW(GetCommandLine(), &argc);

		EntryPoint(argc, (char**)argv);

		LocalFree(argv);

		KG_INFO("Application Shut Down Successfully!");
		return 0;

	}
#endif
