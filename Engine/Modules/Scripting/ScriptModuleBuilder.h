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
		// Internal Functionality to Support Creation
		//==============================
		static void CreateModuleHeaderFile();
		static bool CreateModuleCPPFile();
		static bool CompileModuleCodeMSVC(bool createDebug);
		static bool CompileModuleCodeGCC(bool createDebug);
		static void AttachEngineFunctionsToModule();
	public:
		friend ScriptService;
	};
}
