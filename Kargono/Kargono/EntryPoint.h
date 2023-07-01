#pragma once


#ifdef KG_PLATFORM_WINDOWS
	
	extern Karg::ApplicationV2* Karg::CreateApplication();

	int main(int argc, char** argv)
	{
		Karg::Log::Init();
		KG_CORE_WARN("Initialized Log!");
		int a = 5;
		KG_INFO("Hello! Var={0}", a);

		auto app = Karg::CreateApplication();
		app->Run();
		delete app;
	}
#endif