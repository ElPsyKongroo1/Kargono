#pragma once

#include "Modules/Scripting/ScriptService.h"

namespace Kargono::Scripting
{
	//==============================
	// Script Module Builder Class	
	//==============================
	class ScriptModuleBuilder
	{
	public:
		//==============================
		// Create Script Module
		//==============================
		static void CreateScriptModule();

	private:
		//==============================
		// Generate Module Code
		//==============================
		static void CreateModuleHeaderFile();
		static bool CreateModuleCPPFile();
		//==============================
		// Compile Module
		//==============================
		static bool CompileModuleCodeMSVC(bool createDebug);
		static bool CompileModuleCodeGCC(bool createDebug);
		//==============================
		// Connect Engine To Module
		//==============================
		static void AttachEngineFunctionsToModule();
	public:
		friend ScriptService;
	};
}
