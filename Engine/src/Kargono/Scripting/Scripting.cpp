#include "kgpch.h"

#include "Kargono/Scripting/Scripting.h"
#include "Kargono/Assets/AssetManager.h"
#include "Kargono/Core/FileSystem.h"
#include "Kargono/Projects/Project.h"

#ifdef KG_PLATFORM_WINDOWS
#include "Windows.h"
#endif


namespace Kargono::Scripting
{
	std::function<void(const std::string&, std::function<void()>)> ScriptCore::s_AddVoidNone {};

	struct ScriptingData
	{
		HINSTANCE* DLLInstance = nullptr;
	};

	static ScriptingData* s_ScriptingData = nullptr;


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
			KG_CRITICAL("Attempt to open new scripting dll when a dll already exists in the engine");
			return;
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

		AddEngineFuncsToDll();

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
			KG_CRITICAL("Attempt to close scripting dll, however, DLLInstance is a nullptr");
			return;
		}

		if (*s_ScriptingData->DLLInstance == NULL)
		{
			KG_CRITICAL("Attempt to close scripting dll, however, DLLInstance value is 0 (NULL)");
			s_ScriptingData->DLLInstance = nullptr;
			return;
		}

		FreeLibrary(*s_ScriptingData->DLLInstance);

		delete s_ScriptingData->DLLInstance;
		s_ScriptingData->DLLInstance = nullptr;
	}
	void ScriptCore::CreateDll(bool addDebugSymbols)
	{
		CloseDll();

		CreateDllHeader();
		CreateDllCPPFiles();
		CompileDll(addDebugSymbols);

		OpenDll();
		Assets::AssetManager::DeserializeScriptRegistry();
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

	void ScriptCore::CreateDllHeader()
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

		outputStream << "KARGONO_API void AddVoidNone(const std::string& funcName, std::function<void()> funcPtr);\n";
		//outputStream << "KARGONO_API void AddBoolNone(const std::string& funcName, std::function<bool()> funcPtr);\n";

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

		std::filesystem::path headerFile = { Projects::Project::GetAssetDirectory() / "Scripting/ExportSource/ExportHeader.h"};

		FileSystem::WriteFileString(headerFile, outputStream.str());
	}
	void ScriptCore::CreateDllCPPFiles()
	{
		std::stringstream outputStream {};
		outputStream << "#include \"ExportHeader.h\"\n";
		outputStream << "#include <iostream>\n";
		outputStream << "namespace Kargono\n";
		outputStream << "{\n";

		outputStream << "static std::function<void()> testFuncPtr {};\n";
		outputStream << "void testFunction()\n";
		outputStream << "{\n";
		outputStream << "testFuncPtr();\n";
		outputStream << "}\n";

		outputStream << "void AddVoidNone(const std::string& funcName, std::function<void()> funcPtr)\n";

		outputStream << "{\n";
		outputStream << "if (funcName == \"testFunction\") { testFuncPtr = funcPtr; return; }\n";
		outputStream << "}\n";

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

	void ScriptCore::CompileDll(bool addDebugSymbols)
	{
		FileSystem::CreateNewDirectory(Projects::Project::GetAssetDirectory() / "Scripting/Intermediates");
		FileSystem::CreateNewDirectory(Projects::Project::GetAssetDirectory() / "Scripting/Binary");

		std::filesystem::path intermediatePath { Projects::Project::GetAssetDirectory() / "Scripting/Intermediates/" };
		std::filesystem::path binaryPath { Projects::Project::GetAssetDirectory() / "Scripting/Binary/" };
		std::filesystem::path sourcePath { Projects::Project::GetAssetDirectory() / "Scripting/ExportSource/ExportBody.cpp" };

		std::stringstream outputStream {};
		// Access visual studio toolset console
		//outputStream << "\"C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\Common7\\Tools\\VsDevCmd.bat\" && ";
		outputStream << "\"C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Auxiliary\\Build\\vcvars64.bat\" && ";

		// Cl command for compiling binary code
		outputStream << "cl "; // Add Command
		outputStream << "/LD "; // Make Output Shared Library
		outputStream << "/DKARGONO_EXPORTS "; // Define Macros/Defines

		if (addDebugSymbols)
		{
			outputStream << "/Z7 "; // Add debug info to executable
		}
		outputStream << "/Fo" << intermediatePath.string() << ' '; // Define Intermediate Location
		outputStream << "/Fe" << binaryPath.string() << ' '; // Define Macros/Defines
		outputStream << sourcePath.string(); // Add File(s) to compile
		system(outputStream.str().c_str());
	}

	void TestFunc()
	{
		std::cout << "This function was called in the engine ahahahahahhaa\n";
	}

	void ScriptCore::AddEngineFuncsToDll()
	{
		s_AddVoidNone = reinterpret_cast<void_string_funcvoidnone>(GetProcAddress(*s_ScriptingData->DLLInstance, "AddVoidNone"));

		if (!s_AddVoidNone)
		{
			KG_CRITICAL("Could not load AddVoidNone function from scripting dll");
			return;
		}

		s_AddVoidNone("testFunction", TestFunc);
	}
}
