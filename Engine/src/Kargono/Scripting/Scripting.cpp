#include "kgpch.h"

#include "Kargono/Scripting/Scripting.h"

#include "Kargono/Scripting/ScriptModuleBuilder.h"
#include "Kargono/Assets/AssetManager.h"
#include "Kargono/Core/FileSystem.h"
#include "Kargono/Projects/Project.h"
#include "Kargono/Audio/AudioEngine.h"

#ifdef KG_PLATFORM_WINDOWS
#include "Windows.h"
#endif

namespace Kargono::Scripting
{
	struct ScriptingData
	{
		HINSTANCE* DLLInstance = nullptr;
	};

	static ScriptingData* s_ScriptingData = nullptr;
}

namespace Kargono::Scripting
{
	typedef void (*void_none)();
	typedef bool (*bool_none)();

	

	void ScriptCore::Init()
	{
		s_ScriptingData = new ScriptingData();
	}

	void ScriptCore::Terminate()
	{
		if (!s_ScriptingData) { return; }

		if (s_ScriptingData->DLLInstance)
		{
			CloseDll();
		}

		delete s_ScriptingData;
	}

	void ScriptCore::OpenDll()
	{
		std::filesystem::path dllLocation { Projects::Project::GetAssetDirectory() / "Scripting\\Binary\\ExportBody.dll" };
		if (!s_ScriptingData)
		{
			KG_CRITICAL("Attempt to open a new scripting dll, however, ScriptEngine is not initialized");
			return;
		}

		if (s_ScriptingData->DLLInstance)
		{
			KG_INFO("Closing existing script dll");
			CloseDll();
		}

		s_ScriptingData->DLLInstance = new HINSTANCE();

		*(s_ScriptingData->DLLInstance) = LoadLibrary(dllLocation.c_str());

		if (*s_ScriptingData->DLLInstance == NULL)
		{
			KG_CRITICAL("Failed to open dll with path {} with an error code of {}", dllLocation.string(), GetLastError());
			CloseDll();
			return;
		}

		KG_INFO("Script Dll successfully opened!");

		ScriptModuleBuilder::AddEngineFuncsToDll();

	}
	void ScriptCore::CloseDll()
	{
		if (!s_ScriptingData)
		{
			KG_CRITICAL("Attempt to close scripting dll, however, ScriptEngine is not initialized");
			return;
		}

		if (!s_ScriptingData->DLLInstance)
		{
			KG_WARN("Attempt to close scripting dll, however, DLLInstance is a nullptr");
			return;
		}

		if (*s_ScriptingData->DLLInstance == NULL)
		{
			KG_WARN("Attempt to close scripting dll, however, DLLInstance value is 0 (NULL)");
			s_ScriptingData->DLLInstance = nullptr;
			return;
		}

		FreeLibrary(*s_ScriptingData->DLLInstance);

		delete s_ScriptingData->DLLInstance;
		s_ScriptingData->DLLInstance = nullptr;
	}

	void ScriptCore::LoadScriptFunction(Ref<Script> script, WrappedFuncType funcType)
	{
		if (!s_ScriptingData || !s_ScriptingData->DLLInstance || *s_ScriptingData->DLLInstance == NULL)
		{
			KG_CRITICAL("Attempt to load a scripting function, however, ScriptEngine is not valid");
			return;
		}

		std::string funcName {"KG_FUNC_" + std::string(script->m_ID)};

		switch (funcType)
		{
		case WrappedFuncType::Void_None:
		{
			script->m_Function = CreateRef<WrappedVoidNone>();
			((WrappedVoidNone*)script->m_Function.get())->m_Value = reinterpret_cast<void_none>(GetProcAddress(*s_ScriptingData->DLLInstance, funcName.c_str()));
			break;
		}
		case WrappedFuncType::Bool_None:
		{
			script->m_Function = CreateRef<WrappedVoidNone>();
			((WrappedBoolNone*)script->m_Function.get())->m_Value = reinterpret_cast<bool_none>(GetProcAddress(*s_ScriptingData->DLLInstance, funcName.c_str()));
			break;
		}
		default:
		{
			KG_CRITICAL("Invalid WrappedFuncType in LoadScriptFunction!");
			return;
		}
		}
	}
}

namespace Kargono::Utility
{
#define DefineInsertFunction(name, returnType,...) \
	typedef void (*Void_String_Func##name)(const std::string&, std::function<returnType(__VA_ARGS__)>); \
	std::function<returnType(const std::string&, std::function<returnType(__VA_ARGS__)>)> s_Add##name {};

#define ImportInsertFunction(name, returnType) \
	s_Add##name = reinterpret_cast<Void_String_Func##name>(GetProcAddress(*s_ScriptingData->DLLInstance, "Add"#name ));\
	if (!s_Add##name)\
	{\
		KG_CRITICAL("Could not load {} function from scripting dll", "" #name);\
		return;\
	}
#define AddImportFunctionToHeaderFile(name, returnType, ...) \
	outputStream << "KARGONO_API void Add" << #name << "(const std::string& funcName, std::function<" << #returnType <<"(" << (#__VA_ARGS__ ")> funcPtr);\n");

#define AddImportFunctionToCPPFile(name, returnType, ...) \
	outputStream << "void Add" << #name << "(const std::string& funcName, std::function<" << #returnType <<"(" << (#__VA_ARGS__ ")> funcPtr)\n");

#define AddEngineFunctionToCPPFileNoParameters(name, returnType) \
	outputStream << "static std::function<" #returnType "()> " #name "Ptr {};\n"; \
	outputStream << #returnType " PlaySoundFromName()\n"; \
	outputStream << "{\n"; \
	outputStream << "\t" #name "Ptr();\n"; \
	outputStream << "}\n";
#define AddEngineFunctionToCPPFileOneParameters(name, returnType, parameter1)\
	outputStream << "static std::function<" #returnType "(" #parameter1 " a)> " #name "Ptr {};\n"; \
	outputStream << #returnType " PlaySoundFromName(" #parameter1 " a)\n"; \
	outputStream << "{\n"; \
	outputStream << "\t" #name "Ptr(a);\n"; \
	outputStream << "}\n";
#define AddEngineFunctionToCPPFileTwoParameters(name, returnType, parameter1, parameter2)\
	outputStream << "static std::function<" #returnType "(" #parameter1 " a, " #parameter2 " b)> " #name "Ptr {};\n"; \
	outputStream << #returnType " PlaySoundFromName(" #parameter1 " a, " #parameter2 " b)\n"; \
	outputStream << "{\n"; \
	outputStream << "\t" #name "Ptr(a, b);\n"; \
	outputStream << "}\n";

#define AddEngineFunctionToCPPFileEnd(name) \
	outputStream << "if (funcName == \"" #name "\") { " #name "Ptr = funcPtr; return; }\n";

#define AddEngineFunctionPointerToDll(name, funcDef, funcSignatureName) \
	s_Add##funcSignatureName(#name, funcDef);
}

namespace Kargono::Scripting
{
	// Initial definitions and static members for insertion functions (functions that insert engine pointers into the dll)
	DefineInsertFunction(VoidNone, void)
	DefineInsertFunction(VoidString, void, const std::string&)

	// This macro adds insertion function declarations to the header file
#define AddEngineFunctionsToHeaderFiles() \
	AddImportFunctionToHeaderFile(VoidNone, void) \
	AddImportFunctionToHeaderFile(VoidString, void, const std::string&)

	// This	macro adds insertion function definitions and func pointers into the CPP file
#define AddEngineFunctionsToCPPFiles() \
	AddEngineFunctionToCPPFileOneParameters(PlaySoundFromName, void, const std::string&)\
	AddImportFunctionToCPPFile(VoidNone, void) \
	outputStream << "{\n"; \
	outputStream << "}\n"; \
	AddImportFunctionToCPPFile(VoidString, void, const std::string&) \
	outputStream << "{\n"; \
	AddEngineFunctionToCPPFileEnd(PlaySoundFromName) \
	outputStream << "}\n";

	// This macro provide the point where insertion functions are pulled from the dll when opening
#define AddEngineFunctionsPointersToDll() \
	ImportInsertFunction(VoidNone, void)\
	ImportInsertFunction(VoidString, void) \
	AddEngineFunctionPointerToDll(PlaySoundFromName, Audio::AudioEngine::PlaySoundFromName,VoidString)

	void ScriptModuleBuilder::CreateDll(bool addDebugSymbols)
	{
		ScriptCore::CloseDll();

		CreateDllHeader();
		CreateDllCPPFiles();
		CompileDll(addDebugSymbols);

		ScriptCore::OpenDll();
		Assets::AssetManager::DeserializeScriptRegistry();
	}

	void ScriptModuleBuilder::CreateDllHeader()
	{
		// Write out return value and function name
		std::stringstream outputStream {};
		outputStream << "#pragma once\n";
		outputStream << "#ifdef KARGONO_EXPORTS\n";
		outputStream << "#define KARGONO_API __declspec(dllexport)\n";
		outputStream << "#else\n";
		outputStream << "#define KARGONO_API __declspec(dllimport)\n";
		outputStream << "#endif\n";

		outputStream << "#include <functional>\n";
		outputStream << "#include <string>\n";

		outputStream << "namespace Kargono\n";
		outputStream << "{" << "\n";
		outputStream << "extern \"C\"" << "\n";
		outputStream << "\t{" << "\n";

		AddEngineFunctionsToHeaderFiles()

		// Add Script Function Declarations
		for (auto& [handle, script] : Assets::AssetManager::s_Scripts)
		{
			outputStream << "\t\tKARGONO_API ";
			outputStream << Utility::WrappedVarTypeToCPPString(script->m_ReturnValue) << " KG_FUNC_" << handle << "(";

			// Write out parameters into function signature
			char letterIteration{ 'a' };
			for (uint32_t iteration{ 0 }; static_cast<size_t>(iteration) < script->m_Parameters.size(); iteration++)
			{
				outputStream << Utility::WrappedVarTypeToCPPString(script->m_Parameters.at(iteration)) << " " << letterIteration;
				if (iteration != script->m_Parameters.size() - 1)
				{
					outputStream << ',';
				}
				letterIteration++;
			}

			outputStream << ");" << "\n";
		}

		outputStream << "\t}" << "\n";
		outputStream << "}" << "\n";

		std::filesystem::path headerFile = { Projects::Project::GetAssetDirectory() / "Scripting/ExportSource/ExportHeader.h" };

		FileSystem::WriteFileString(headerFile, outputStream.str());
	}

	void ScriptModuleBuilder::CreateDllCPPFiles()
	{
		std::stringstream outputStream {};
		outputStream << "#include \"ExportHeader.h\"\n";
		outputStream << "#include <iostream>\n";
		outputStream << "namespace Kargono\n";
		outputStream << "{\n";

		AddEngineFunctionsToCPPFiles()

		// Write scripts into a single cpp file
		for (auto& [handle, asset] : Assets::AssetManager::s_ScriptRegistry)
		{
			outputStream << FileSystem::ReadFileString(Projects::Project::GetAssetDirectory() / asset.Data.IntermediateLocation);
			outputStream << '\n';
		}
		outputStream << "}\n";

		std::filesystem::path file = { Projects::Project::GetAssetDirectory() / "Scripting/ExportSource/ExportBody.cpp" };

		FileSystem::WriteFileString(file, outputStream.str());
	}

	void ScriptModuleBuilder::CompileDll(bool addDebugSymbols)
	{
		FileSystem::CreateNewDirectory(Projects::Project::GetAssetDirectory() / "Scripting/Intermediates");
		FileSystem::CreateNewDirectory(Projects::Project::GetAssetDirectory() / "Scripting/Binary");

		std::filesystem::path intermediatePath { Projects::Project::GetAssetDirectory() / "Scripting/Intermediates/" };
		std::filesystem::path binaryPath { Projects::Project::GetAssetDirectory() / "Scripting/Binary/" };
		std::filesystem::path binaryFile { binaryPath / "ExportBody.dll" };
		std::filesystem::path objectPath { intermediatePath / "ExportBody.obj" };

		UUID pdbID = UUID();
		std::string pdbFileName = std::string(pdbID) + ".pdb";
		std::filesystem::path debugSymbolsPath { binaryPath / pdbFileName };
		std::filesystem::path sourcePath { Projects::Project::GetAssetDirectory() / "Scripting/ExportSource/ExportBody.cpp" };

		std::stringstream outputStream {};
		// Access visual studio toolset console
		outputStream << "\"C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Auxiliary\\Build\\vcvars64.bat\" && ";

		// Cl command for compiling binary code
		outputStream << "cl "; // Add Command
		outputStream << "/c "; // Tell cl command to only compile code
		outputStream << "/MDd "; // Specify Runtime Library
		outputStream << "/DKARGONO_EXPORTS "; // Define Macros/Defines

		if (addDebugSymbols)
		{
			outputStream << "/Z7 "; // Add debug info to executable
		}
		outputStream << "/Fo" << intermediatePath.string() << ' '; // Define Intermediate Location
		outputStream << sourcePath.string() << " "; // Add File(s) to compile

		outputStream << " && "; // Combine commands
		// Start Linking Stage
		outputStream << "link "; // Start link command
		outputStream << "/DLL "; // Specify output as a shared library
		if (addDebugSymbols)
		{
			outputStream << "/DEBUG "; // Specifies output as debug files
			outputStream << "/PDB:" << debugSymbolsPath.string() << " "; // Specify .pdb file location/name
		}
		outputStream << "/OUT:" << binaryFile.string() << " "; // Specify output directory
		outputStream << objectPath.string(); // Object File to Link
		system(outputStream.str().c_str());
	}


	void ScriptModuleBuilder::AddEngineFuncsToDll()
	{
		AddEngineFunctionsPointersToDll()
	}
}

