#pragma once

#include "Kargono/Scripting/ScriptService.h"

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
		// Internal Functionality to Support Creation
		//==============================
		static void CreateModuleHeaderFile();
		static bool CreateModuleCPPFile();
		static bool CompileModuleCode(bool createDebug);
		static void AttachEngineFunctionsToModule();
	public:
		friend ScriptService;
	};
}
