#pragma once
#include "Kargono/Core/Base.h"
#include "Kargono/Core/Engine.h"
#include "Kargono/Utility/Timers.h"

#ifdef KG_PLATFORM_WINDOWS
#include "API/Platform/WindowsBackendAPI.h"
#endif

extern void Kargono::InitEngineAndCreateApp(CommandLineArguments args);
//==============================
// General Entry Point
//==============================
void EntryPoint (int argc, char** argv)
{
	Kargono::Log::Init();
	KG_INFO("Starting Application");
	Kargono::InitEngineAndCreateApp({ argc, argv });
	Kargono::EngineService::Run();
	Kargono::Utility::AsyncBusyTimer::CloseAllTimers();
	Kargono::EngineService::Terminate();
		
}

#ifdef KG_PLATFORM_WINDOWS
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
