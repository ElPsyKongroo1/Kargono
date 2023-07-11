#pragma once


#ifdef KG_PLATFORM_WINDOWS
	
	extern Kargono::Application* Kargono::CreateApplication();

	int main(int argc, char** argv)
	{
		Kargono::Log::Init();
		KG_CORE_WARN("Initialized Log!");
		int a = 5;
		KG_CORE_INFO("Hello! Var={0}", a);

		auto app = Kargono::CreateApplication();
		app->Run();
		delete app;
	}
#endif