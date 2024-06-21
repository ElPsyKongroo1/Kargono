#include "kgpch.h"

#include "Kargono/Scripting/Scripting.h"

#include "Kargono/Core/EngineCore.h"
#include "Kargono/Scripting/ScriptModuleBuilder.h"
#include "Kargono/Assets/AssetManager.h"
#include "Kargono/Scenes/Scene.h"
#include "Kargono/Utility/FileSystem.h"
#include "Kargono/Projects/Project.h"
#include "Kargono/Audio/AudioEngine.h"
#include "Kargono/RuntimeUI/Runtime.h"
#include "Kargono/Input/InputMode.h"
#include "Kargono/Network/Client.h"
#include "Kargono/Scenes/GameState.h"

#ifdef KG_PLATFORM_WINDOWS
#include "API/Platform/WindowsBackendAPI.h"
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
	void ScriptCore::Init()
	{
		s_ScriptingData = new ScriptingData();
		KG_VERIFY(s_ScriptingData, "Scripting System Init");
	}

	void ScriptCore::Terminate()
	{
		if (!s_ScriptingData) { return; }

		if (s_ScriptingData->DLLInstance)
		{
			CloseActiveScriptModule();
		}

		delete s_ScriptingData;
		s_ScriptingData = nullptr;

		KG_VERIFY(!s_ScriptingData, "Close Scripting System")
	}

	void ScriptCore::LoadActiveScriptModule()
	{
		std::filesystem::path dllLocation { Projects::Project::GetAssetDirectory() / "Scripting\\Binary\\ExportBody.dll" };
		
		// Rebuild shared library if no library exists
		static bool attemptedToRebuild = false;
		if (!std::filesystem::exists(dllLocation))
		{
			if (attemptedToRebuild)
			{
				KG_ERROR("Multiple attempts to open dll with no file present! Might be an issue generating shared library.");
				return;
			}
			KG_WARN("Attempt to open scripting dll, however, none exists. Attempting to create new Shared Lib.");
			attemptedToRebuild = true;
			EngineCore::GetCurrentEngineCore().SubmitToMainThread([]()
			{
				ScriptModuleBuilder::CreateScriptModule();
			});
			return;
		}
		attemptedToRebuild = false;

		if (!s_ScriptingData)
		{
			KG_CRITICAL("Attempt to open a new scripting dll, however, ScriptEngine is not initialized");
			return;
		}

		if (s_ScriptingData->DLLInstance)
		{
			KG_INFO("Closing existing script dll");
			CloseActiveScriptModule();
		}

		s_ScriptingData->DLLInstance = new HINSTANCE();

		*(s_ScriptingData->DLLInstance) = LoadLibrary(dllLocation.c_str());

		if (*s_ScriptingData->DLLInstance == NULL)
		{
			KG_CRITICAL("Failed to open dll with path {} with an error code of {}", dllLocation.string(), GetLastError());
			CloseActiveScriptModule();
			return;
		}


		ScriptModuleBuilder::AttachEngineFunctionsToModule();

		KG_VERIFY(s_ScriptingData->DLLInstance, "Scripting DLL Opened");

	}
	void ScriptCore::CloseActiveScriptModule()
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

		KG_VERIFY(!s_ScriptingData->DLLInstance, "Close Scripting DLL");
	}

	void ScriptCore::LoadScriptFunction(Ref<Script> script, WrappedFuncType funcType)
	{
		if (!s_ScriptingData || !s_ScriptingData->DLLInstance || *s_ScriptingData->DLLInstance == NULL)
		{
			KG_CRITICAL("Attempt to load a scripting function, however, ScriptEngine is not valid");
			return;
		}

		switch (funcType)
		{
		case WrappedFuncType::Void_None:
		{
			script->m_Function = CreateRef<WrappedVoidNone>();
			((WrappedVoidNone*)script->m_Function.get())->m_Value = reinterpret_cast<void_none>(GetProcAddress(*s_ScriptingData->DLLInstance, script->m_ScriptName.c_str()));
			break;
		}

		case WrappedFuncType::Void_String:
		{
			script->m_Function = CreateRef<WrappedVoidString>();
			((WrappedVoidString*)script->m_Function.get())->m_Value = reinterpret_cast<void_string>(GetProcAddress(*s_ScriptingData->DLLInstance, script->m_ScriptName.c_str()));
			break;
		}

		case WrappedFuncType::Void_Float:
		{
			script->m_Function = CreateRef<WrappedVoidFloat>();
			((WrappedVoidFloat*)script->m_Function.get())->m_Value = reinterpret_cast<void_float>(GetProcAddress(*s_ScriptingData->DLLInstance, script->m_ScriptName.c_str()));
			break;
		}

		case WrappedFuncType::Void_UInt16:
		{
			script->m_Function = CreateRef<WrappedVoidUInt16>();
			((WrappedVoidUInt16*)script->m_Function.get())->m_Value = reinterpret_cast<void_uint16>(GetProcAddress(*s_ScriptingData->DLLInstance, script->m_ScriptName.c_str()));
			break;
		}
		case WrappedFuncType::Void_UInt32:
		{
			script->m_Function = CreateRef<WrappedVoidUInt32>();
			((WrappedVoidUInt32*)script->m_Function.get())->m_Value = reinterpret_cast<void_uint32>(GetProcAddress(*s_ScriptingData->DLLInstance, script->m_ScriptName.c_str()));
			break;
		}

		case WrappedFuncType::Void_UInt64:
		{
			script->m_Function = CreateRef<WrappedVoidUInt64>();
			((WrappedVoidUInt64*)script->m_Function.get())->m_Value = reinterpret_cast<void_uint64>(GetProcAddress(*s_ScriptingData->DLLInstance, script->m_ScriptName.c_str()));
			break;
		}

		case WrappedFuncType::Void_UInt64Float:
		{
			script->m_Function = CreateRef<WrappedVoidUInt64Float>();
			((WrappedVoidUInt64Float*)script->m_Function.get())->m_Value = reinterpret_cast<void_uint64float>(GetProcAddress(*s_ScriptingData->DLLInstance, script->m_ScriptName.c_str()));
			break;
		}
		case WrappedFuncType::Bool_None:
		{
			script->m_Function = CreateRef<WrappedBoolNone>();
			((WrappedBoolNone*)script->m_Function.get())->m_Value = reinterpret_cast<bool_none>(GetProcAddress(*s_ScriptingData->DLLInstance, script->m_ScriptName.c_str()));
			break;
		}

		case WrappedFuncType::Bool_UInt64:
		{
			script->m_Function = CreateRef<WrappedBoolUInt64>();
			((WrappedBoolUInt64*)script->m_Function.get())->m_Value = reinterpret_cast<bool_uint64>(GetProcAddress(*s_ScriptingData->DLLInstance, script->m_ScriptName.c_str()));
			break;
		}
		case WrappedFuncType::Bool_UInt64UInt64:
		{
			script->m_Function = CreateRef<WrappedBoolUInt64UInt64>();
			((WrappedBoolUInt64UInt64*)script->m_Function.get())->m_Value = reinterpret_cast<bool_uint64uint64>(GetProcAddress(*s_ScriptingData->DLLInstance, script->m_ScriptName.c_str()));
			break;
		}
		case WrappedFuncType::Bool_UInt64UInt16UInt64:
		{
			script->m_Function = CreateRef<WrappedBoolUInt64UInt16UInt64>();
			((WrappedBoolUInt64UInt16UInt64*)script->m_Function.get())->m_Value = reinterpret_cast<bool_uint64uint16uint64>(GetProcAddress(*s_ScriptingData->DLLInstance, script->m_ScriptName.c_str()));
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
	std::function<void(const std::string&, std::function<returnType(__VA_ARGS__)>)> s_Add##name {};

#define ImportInsertFunction(name) \
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
	outputStream << #returnType " " #name "()\n"; \
	outputStream << "{\n"; \
	outputStream << "\t"; \
	if (!std::is_same<returnType, void>::value) \
	{\
	outputStream << "return "; \
	}\
	outputStream << #name "Ptr();\n"; \
	outputStream << "}\n";
#define AddEngineFunctionToCPPFileOneParameters(name, returnType, parameter1)\
	outputStream << "static std::function<" #returnType "(" #parameter1 " a)> " #name "Ptr {};\n"; \
	outputStream << #returnType " " #name "(" #parameter1 " a)\n"; \
	outputStream << "{\n"; \
	outputStream << "\t"; \
	if (!std::is_same<returnType, void>::value) \
	{\
	outputStream << "return "; \
	}\
	outputStream << #name "Ptr(a);\n"; \
	outputStream << "}\n";
#define AddEngineFunctionToCPPFileTwoParameters(name, returnType, parameter1, parameter2)\
	outputStream << "static std::function<" #returnType "(" #parameter1 " a, " #parameter2 " b)> " #name "Ptr {};\n"; \
	outputStream << #returnType " " #name "(" #parameter1 " a, " #parameter2 " b)\n"; \
	outputStream << "{\n"; \
	outputStream << "\t"; \
	if (!std::is_same<returnType, void>::value) \
	{\
	outputStream << "return "; \
	}\
	outputStream << #name "Ptr(a, b);\n"; \
	outputStream << "}\n";

#define AddEngineFunctionToCPPFileThreeParameters(name, returnType, parameter1, parameter2, parameter3)\
	outputStream << "static std::function<" #returnType "(" #parameter1 " a, " #parameter2 " b, " #parameter3 " c)> " #name "Ptr {};\n"; \
	outputStream << #returnType " " #name "(" #parameter1 " a, " #parameter2 " b, " #parameter3 " c)\n"; \
	outputStream << "{\n"; \
	outputStream << "\t"; \
	if (!std::is_same<returnType, void>::value) \
	{\
	outputStream << "return "; \
	}\
	outputStream << #name "Ptr(a, b, c);\n"; \
	outputStream << "}\n";

#define AddEngineFunctionToCPPFileEnd(name) \
	outputStream << "if (funcName == \"" #name "\") { " #name "Ptr = funcPtr; return; }\n";

#define AddEngineFunctionPointerToDll(name, funcDef, funcSignatureName) \
	s_Add##funcSignatureName(#name, funcDef);
}

namespace Kargono::Scripting
{
	// Initial definitions and static members for insertion functions (functions that insert engine pointers into the scripting dll)
	DefineInsertFunction(VoidNone, void)
	DefineInsertFunction(VoidString, void, const std::string&)
	DefineInsertFunction(VoidStringBool, void, const std::string&, bool)
	DefineInsertFunction(VoidStringVoidPtr, void, const std::string&, void*)
	DefineInsertFunction(VoidStringString, void, const std::string&, const std::string&)
	DefineInsertFunction(VoidStringStringBool, void, const std::string&, const std::string&, bool)
	DefineInsertFunction(VoidStringStringString, void, const std::string&, const std::string&, const std::string&)
	DefineInsertFunction(VoidStringStringVec4, void, const std::string&, const std::string&, Math::vec4)
	DefineInsertFunction(BoolUInt64String, bool, uint64_t, const std::string&)
	DefineInsertFunction(UInt16None, uint16_t)
	DefineInsertFunction(Vec3UInt64, Math::vec3, uint64_t)

	// Engine Functions that need to be defined only in this file
	static void Log(const std::string& info)
	{
		KG_WARN(info);
	}

	void ScriptModuleBuilder::CreateScriptModule(bool addDebugSymbols)
	{
		ScriptCore::CloseActiveScriptModule();

		CreateModuleHeaderFile();
		CreateModuleCPPFile();
		CompileModuleCode(addDebugSymbols);

		ScriptCore::LoadActiveScriptModule();
		Assets::AssetManager::DeserializeScriptRegistry();
	}
	void ScriptModuleBuilder::CreateModuleHeaderFile()
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
		outputStream << "#include <sstream>\n";
		outputStream << "#include \"" << "Kargono/Math/MathAliases.h" << "\"\n"; // Include Math Library
		outputStream << "namespace Kargono\n";
		outputStream << "{" << "\n";
		outputStream << "extern \"C\"" << "\n";
		outputStream << "\t{" << "\n";

		AddImportFunctionToHeaderFile(VoidNone, void) 
		AddImportFunctionToHeaderFile(VoidString, void, const std::string&) 
		AddImportFunctionToHeaderFile(VoidStringBool, void, const std::string&, bool) 
		AddImportFunctionToHeaderFile(VoidStringVoidPtr, void, const std::string&, void*) 
		AddImportFunctionToHeaderFile(VoidStringString, void, const std::string&, const std::string&) 
		AddImportFunctionToHeaderFile(VoidStringStringBool, void, const std::string&, const std::string&, bool) 
		AddImportFunctionToHeaderFile(VoidStringStringString, void, const std::string&, const std::string&, const std::string&) 
		AddImportFunctionToHeaderFile(VoidStringStringVec4, void, const std::string&, const std::string&, Math::vec4) 
		AddImportFunctionToHeaderFile(BoolUInt64String, bool, uint64_t, const std::string&) 
		AddImportFunctionToHeaderFile(UInt16None, uint16_t) 
		AddImportFunctionToHeaderFile(Vec3UInt64, Math::vec3, uint64_t)

		// Add Script Function Declarations
		for (auto& [handle, script] : Assets::AssetManager::s_Scripts)
		{
			WrappedVarType returnValue = Utility::WrappedFuncTypeToReturnType(script->m_FuncType);
			std::vector<WrappedVarType> parameters = Utility::WrappedFuncTypeToParameterTypes(script->m_FuncType);

			outputStream << "\t\tKARGONO_API ";
			outputStream << Utility::WrappedVarTypeToCPPString(returnValue) << " " << script->m_ScriptName << "(";

			// Write out parameters into function signature
			char letterIteration{ 'a' };
			for (uint32_t iteration{ 0 }; static_cast<size_t>(iteration) < parameters.size(); iteration++)
			{
				outputStream << Utility::WrappedVarTypeToCPPString(parameters.at(iteration)) << " " << letterIteration;
				if (iteration != parameters.size() - 1)
				{
					outputStream << ',';
				}
				letterIteration++;
			}

			outputStream << ");" << "\n";
		}

		outputStream << "\t}" << "\n";
		outputStream << "}" << "\n";

		std::filesystem::path headerFile = { Projects::Project::GetAssetDirectory() / "Scripting/Binary/ExportHeader.h" };

		Utility::FileSystem::WriteFileString(headerFile, outputStream.str());
	}

	void ScriptModuleBuilder::CreateModuleCPPFile()
	{
		std::stringstream outputStream {};
		outputStream << "#include \"ExportHeader.h\"\n";
		outputStream << "#include <iostream>\n";
		outputStream << "namespace Kargono\n";
		outputStream << "{\n";

		// Insert Callable Function Definitions into CPP file
		AddEngineFunctionToCPPFileNoParameters(EnableReadyCheck, void)
		AddEngineFunctionToCPPFileNoParameters(RequestUserCount, void)
		AddEngineFunctionToCPPFileNoParameters(GetActiveSessionSlot, uint16_t)
		AddEngineFunctionToCPPFileOneParameters(Log, void, const std::string&)
		AddEngineFunctionToCPPFileOneParameters(PlaySoundFromName, void, const std::string&)
		AddEngineFunctionToCPPFileOneParameters(PlayStereoSoundFromName, void, const std::string&)
		AddEngineFunctionToCPPFileOneParameters(LoadInputModeByName, void, const std::string&)
		AddEngineFunctionToCPPFileOneParameters(LoadUserInterfaceFromName, void, const std::string&)
		AddEngineFunctionToCPPFileOneParameters(TransitionSceneFromName, void, const std::string&)
		AddEngineFunctionToCPPFileOneParameters(TransformComponent_GetTranslation, Math::vec3, uint64_t)
		AddEngineFunctionToCPPFileTwoParameters(CheckHasComponent, bool, uint64_t, const std::string&)
		AddEngineFunctionToCPPFileTwoParameters(SetDisplayWindow, void, const std::string&, bool)
		AddEngineFunctionToCPPFileTwoParameters(SetSelectedWidget, void, const std::string&, const std::string&)
		AddEngineFunctionToCPPFileTwoParameters(SetGameStateField, void, const std::string&, void*)
		AddEngineFunctionToCPPFileThreeParameters(SetWidgetSelectable, void, const std::string&, const std::string&, bool)
		AddEngineFunctionToCPPFileThreeParameters(SetWidgetText, void, const std::string&, const std::string&, const std::string&)
		AddEngineFunctionToCPPFileThreeParameters(SetWidgetTextColor, void, const std::string&, const std::string&, Math::vec4)
		AddEngineFunctionToCPPFileThreeParameters(SetWidgetBackgroundColor, void, const std::string&, const std::string&, Math::vec4)

		// Insert FuncPointer Importing for DLL processing
		AddImportFunctionToCPPFile(VoidNone, void)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(EnableReadyCheck)
		AddEngineFunctionToCPPFileEnd(RequestUserCount)
		outputStream << "}\n"; 
		AddImportFunctionToCPPFile(VoidString, void, const std::string&)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(Log)
		AddEngineFunctionToCPPFileEnd(PlaySoundFromName)
		AddEngineFunctionToCPPFileEnd(PlayStereoSoundFromName)
		AddEngineFunctionToCPPFileEnd(LoadInputModeByName)
		AddEngineFunctionToCPPFileEnd(TransitionSceneFromName)
		AddEngineFunctionToCPPFileEnd(LoadUserInterfaceFromName)
		outputStream << "}\n";
		AddImportFunctionToCPPFile(VoidStringBool, void, const std::string&, bool)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(SetDisplayWindow)
		outputStream << "}\n";
		AddImportFunctionToCPPFile(VoidStringVoidPtr, void, const std::string&, void*)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(SetGameStateField)
		outputStream << "}\n";
		AddImportFunctionToCPPFile(VoidStringString, void, const std::string&, const std::string&)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(SetSelectedWidget)
		outputStream << "}\n";
		AddImportFunctionToCPPFile(VoidStringStringBool, void, const std::string&, const std::string&, bool)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(SetWidgetSelectable)
		outputStream << "}\n";
		AddImportFunctionToCPPFile(VoidStringStringString, void, const std::string&, const std::string&, const std::string&)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(SetWidgetText)
		outputStream << "}\n";
		AddImportFunctionToCPPFile(VoidStringStringVec4, void, const std::string&, const std::string&, Math::vec4)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(SetWidgetTextColor)
		AddEngineFunctionToCPPFileEnd(SetWidgetBackgroundColor)
		outputStream << "}\n";
		AddImportFunctionToCPPFile(BoolUInt64String, bool, uint64_t, const std::string&)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(CheckHasComponent)
		outputStream << "}\n";
		AddImportFunctionToCPPFile(UInt16None, uint16_t)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(GetActiveSessionSlot)
		outputStream << "}\n";
		AddImportFunctionToCPPFile(Vec3UInt64, Math::vec3, uint64_t)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(TransformComponent_GetTranslation)
		outputStream << "}\n";

		// Write scripts into a single cpp file
		for (auto& [handle, asset] : Assets::AssetManager::s_ScriptRegistry)
		{
			outputStream << Utility::FileSystem::ReadFileString(Projects::Project::GetAssetDirectory() / asset.Data.IntermediateLocation);
			outputStream << '\n';
		}
		outputStream << "}\n";

		std::filesystem::path file = { Projects::Project::GetAssetDirectory() / "Scripting/Binary/ExportBody.cpp" };

		Utility::FileSystem::WriteFileString(file, outputStream.str());
	}

	void ScriptModuleBuilder::CompileModuleCode(bool addDebugSymbols)
	{
		//Utility::FileSystem::CreateNewDirectory(Projects::Project::GetAssetDirectory() / "Scripting/Intermediates");
		Utility::FileSystem::CreateNewDirectory(Projects::Project::GetAssetDirectory() / "Scripting/Binary");

		//std::filesystem::path intermediatePath { Projects::Project::GetAssetDirectory() / "Scripting/Intermediates/" };
		std::filesystem::path binaryPath { Projects::Project::GetAssetDirectory() / "Scripting/Binary/" };
		std::filesystem::path binaryFile { binaryPath / "ExportBody.dll" };
		std::filesystem::path objectPath { binaryPath / "ExportBody.obj" };

		UUID pdbID = UUID();
		std::string pdbFileName = std::string(pdbID) + ".pdb";
		std::filesystem::path debugSymbolsPath { binaryPath / pdbFileName };
		std::filesystem::path sourcePath { Projects::Project::GetAssetDirectory() / "Scripting/Binary/ExportBody.cpp" };

		std::stringstream outputStream {};
		// Access visual studio toolset console
		outputStream << "\"C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Auxiliary\\Build\\vcvars64.bat\" && ";

		// Cl command for compiling binary code
		outputStream << "cl "; // Add Command
		outputStream << "/c "; // Tell cl command to only compile code
		outputStream << "/MDd "; // Specify Runtime Library
		outputStream << "/std:c++20 "; // Specify Language Version
		outputStream << "/I../Dependencies/glm "; // Include GLM
		outputStream << "/I../Engine/Source "; // Include Kargono as Include Directory
		outputStream << "/EHsc "; // Specifies the handling of exceptions and call stack unwinding. Uses the commands /EH, /EHs, and /EHc together
		outputStream << "/DKARGONO_EXPORTS "; // Define Macros/Defines

		if (addDebugSymbols)
		{
			outputStream << "/Z7 "; // Add debug info to executable
		}
		outputStream << "/Fo" << binaryPath.string() << ' '; // Define Intermediate Location
		outputStream << sourcePath.string() << " "; // Add File(s) to compile

		outputStream << " && "; // Combine commands
		// Start Linking Stage
		outputStream << "link "; // Start link command
		outputStream << "/DLL "; // Specify output as a shared library
		outputStream << "/ignore:4099 "; // Ignores warning about missing pbd files for .obj files (I generate the symbols inside of the .obj file with /Z7 flag) 
		if (addDebugSymbols)
		{
			outputStream << "/DEBUG "; // Specifies output as debug files
			outputStream << "/PDB:" << debugSymbolsPath.string() << " "; // Specify .pdb file location/name
		}
		outputStream << "/OUT:" << binaryFile.string() << " "; // Specify output directory
		outputStream << objectPath.string(); // Object File to Link
		system(outputStream.str().c_str());
	}


	void ScriptModuleBuilder::AttachEngineFunctionsToModule()
	{
		ImportInsertFunction(VoidNone)
		ImportInsertFunction(VoidString) 
		ImportInsertFunction(VoidStringBool) 
		ImportInsertFunction(VoidStringVoidPtr) 
		ImportInsertFunction(VoidStringString) 
		ImportInsertFunction(VoidStringStringBool) 
		ImportInsertFunction(VoidStringStringString) 
		ImportInsertFunction(VoidStringStringVec4) 
		ImportInsertFunction(BoolUInt64String)
		ImportInsertFunction(UInt16None)
		ImportInsertFunction(Vec3UInt64)
		AddEngineFunctionPointerToDll(EnableReadyCheck, Network::Client::EnableReadyCheck,VoidNone) 
		AddEngineFunctionPointerToDll(RequestUserCount, Network::Client::RequestUserCount,VoidNone) 
		AddEngineFunctionPointerToDll(Log, Scripting::Log,VoidString) 
		AddEngineFunctionPointerToDll(PlaySoundFromName, Audio::AudioEngine::PlaySoundFromName,VoidString) 
		AddEngineFunctionPointerToDll(PlayStereoSoundFromName, Audio::AudioEngine::PlayStereoSoundFromName,VoidString) 
		AddEngineFunctionPointerToDll(LoadInputModeByName, Input::InputMode::SetActiveInputModeByName,VoidString) 
		AddEngineFunctionPointerToDll(LoadUserInterfaceFromName, RuntimeUI::Runtime::LoadUserInterfaceFromName,VoidString) 
		AddEngineFunctionPointerToDll(TransitionSceneFromName, Scenes::Scene::TransitionSceneFromName,VoidString) 
		AddEngineFunctionPointerToDll(SetDisplayWindow, RuntimeUI::Runtime::SetDisplayWindow,VoidStringBool) 
		AddEngineFunctionPointerToDll(SetGameStateField, Scenes::GameState::SetActiveGameStateField, VoidStringVoidPtr) 
		AddEngineFunctionPointerToDll(SetWidgetText, RuntimeUI::Runtime::SetWidgetText,VoidStringStringString) 
		AddEngineFunctionPointerToDll(SetSelectedWidget, RuntimeUI::Runtime::SetSelectedWidget,VoidStringString) 
		AddEngineFunctionPointerToDll(SetWidgetTextColor, RuntimeUI::Runtime::SetWidgetTextColor,VoidStringStringVec4) 
		AddEngineFunctionPointerToDll(SetWidgetBackgroundColor, RuntimeUI::Runtime::SetWidgetBackgroundColor,VoidStringStringVec4) 
		AddEngineFunctionPointerToDll(SetWidgetSelectable, RuntimeUI::Runtime::SetWidgetSelectable,VoidStringStringBool) 
		AddEngineFunctionPointerToDll(CheckHasComponent, Scenes::Scene::CheckHasComponent, BoolUInt64String)
		AddEngineFunctionPointerToDll(GetActiveSessionSlot, Network::Client::GetActiveSessionSlot, UInt16None)
		AddEngineFunctionPointerToDll(TransformComponent_GetTranslation, Scenes::SceneEngine::TransformComponent_GetTranslation, Vec3UInt64)
	}
}

