#pragma once

namespace Kargono::Script
{
	//============================================================
	// Script Glue Class
	//============================================================
	// This class provides static functions that do two things:
	//		1. Initialize and provide C++ functions and function 
	//		declarations to call from the C# Runtime.
	//		2. Register entity components (from ecs system) to 
	//		allow access to the C# runtime.
	class ScriptGlue
	{
	public:
		// This function is called to register components into
		//		unordered map inside C++ file ScriptGlue.cpp.
		//		This map allows components to be queried for each
		//		entity at runtime.
		static void RegisterComponents();
		// This function initializes all of the functions from C# to C++
		//		with the Mono runtime.
		static void RegisterFunctions();
	};
}
