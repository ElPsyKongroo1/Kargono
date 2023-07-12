#pragma once


/// @brief Preprocessor directive to check if the current platform is Windows
#ifdef KG_PLATFORM_WINDOWS
	
/// @brief External declaration of the CreateApplication function in the Kargono namespace
	extern Kargono::Application* Kargono::CreateApplication();

/// @brief The main function of the game engine
	int main(int argc, char** argv)
	{
/// @brief Initializes the logging system in the Kargono namespace
		Kargono::Log::Init();
/// @brief Logs a warning with the message 'Initialized Log!' in the core logger
		KG_CORE_WARN("Initialized Log!");
/// @brief Declaration and initialization of the variable 'a' with the value 5
		int a = 5;
/// @brief Logs an information message with the message 'Hello! Var=[value of a]' in the core logger
		KG_CORE_INFO("Hello! Var={0}", a);

/// @brief Creates an instance of the application using the CreateApplication function in the Kargono namespace
		auto app = Kargono::CreateApplication();
/// @brief Runs the application
		app->Run();
/// @brief Deletes the application instance
		delete app;
	}
#endif