#pragma once


/// @brief Conditional statement for Windows platform
#ifdef KG_PLATFORM_WINDOWS
	
/// @brief Declaration of the CreateApplication function
	extern Kargono::Application* Kargono::CreateApplication();

/// @brief The entry point of the application
	int main(int argc, char** argv)
	{
/// @brief Initialize the logging system
		Kargono::Log::Init();
/// @brief Log a warning message
		KG_CORE_WARN("Initialized Log!");
/// @brief Declare and initialize an integer variable
		int a = 5;
/// @brief Log an informational message with a variable value
		KG_INFO("Hello! Var={0}", a);

/// @brief Create the application object
		auto app = Kargono::CreateApplication();
/// @brief Run the application
		app->Run();
/// @brief Delete the application object
		delete app;
	}
#endif