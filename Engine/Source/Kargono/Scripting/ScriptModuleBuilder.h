#pragma once

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
		static void CreateScriptModule(bool addDebugSymbols = true);
	private:
		//==============================
		// Internal Functionality to Support Creation
		//==============================
		static void CreateModuleHeaderFile();
		static void CreateModuleCPPFile();
		static void CompileModuleCode(bool addDebugSymbols);
		static void AttachEngineFunctionsToModule();
	public:
		friend ScriptCore;
	};
}
