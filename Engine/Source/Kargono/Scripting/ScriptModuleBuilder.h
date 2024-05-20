#pragma once

namespace Kargono::Scripting
{
	class ScriptModuleBuilder
	{
	public:
		static void CreateDll(bool addDebugSymbols = true);
		static void CreateDllHeader();
		static void CreateDllCPPFiles();
		static void CompileDll(bool addDebugSymbols);
		static void AddEngineFuncsToDll();
	};
}
