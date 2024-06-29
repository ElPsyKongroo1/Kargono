#include "kgpch.h"

#include "Kargono/Scripting/Scripting.h"

#include "Kargono/Core/EngineCore.h"
#include "Kargono/Scripting/ScriptModuleBuilder.h"
#include "Kargono/Assets/AssetManager.h"
#include "Kargono/Scenes/Scene.h"
#include "Kargono/Utility/FileSystem.h"
#include "Kargono/Projects/Project.h"
#include "Kargono/Audio/Audio.h"
#include "Kargono/RuntimeUI/RuntimeUI.h"
#include "Kargono/Input/InputMode.h"
#include "Kargono/Input/InputPolling.h"
#include "Kargono/Network/Client.h"
#include "Kargono/Scenes/GameState.h"
#include "Kargono/Utility/Random.h"

#ifdef KG_PLATFORM_WINDOWS
#include "API/Platform/WindowsBackendAPI.h"
#endif

namespace Kargono::Utility
{
	// Pseudo-Random Number Generation for GenerateEngineScripts Func
	static constexpr uint64_t s_Seed { 0xc3bc4ead8efa4c3a };
	static uint64_t s_State {s_Seed};
	static constexpr uint64_t s_Multiplier {6364136223846793005ULL};
	static constexpr uint64_t s_Modulus { std::numeric_limits<uint64_t>::max() };
	static uint64_t GeneratePseudoRandomNumber()
	{
		s_State = (s_State * s_Multiplier) % s_Modulus;
		return s_State;
	}
}

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
	std::vector<Ref<Script>> ScriptService::s_AllEngineScripts {};

	static void GenerateEngineScripts(std::vector<Ref<Script>>& engineScripts)
	{
		Utility::s_State = Utility::s_Seed;
		engineScripts.clear();
		// RuntimeUI
		Ref<Script> RuntimeUI_MoveUp = CreateRef<Script>();
		RuntimeUI_MoveUp->m_ScriptName = "MoveUp";
		RuntimeUI_MoveUp->m_ID = Utility::GeneratePseudoRandomNumber();
		RuntimeUI_MoveUp->m_ScriptType = ScriptType::Engine;
		RuntimeUI_MoveUp->m_FuncType = WrappedFuncType::Void_None;
		RuntimeUI_MoveUp->m_SectionLabel = "RuntimeUI";
		RuntimeUI_MoveUp->m_Function = CreateRef<WrappedVoidNone>(RuntimeUI::RuntimeUIService::MoveUp);
		engineScripts.push_back(RuntimeUI_MoveUp);

		Ref<Script> RuntimeUI_MoveDown = CreateRef<Script>();
		RuntimeUI_MoveDown->m_ScriptName = "MoveDown";
		RuntimeUI_MoveDown->m_ID = Utility::GeneratePseudoRandomNumber();
		RuntimeUI_MoveDown->m_ScriptType = ScriptType::Engine;
		RuntimeUI_MoveDown->m_FuncType = WrappedFuncType::Void_None;
		RuntimeUI_MoveDown->m_SectionLabel = "RuntimeUI";
		RuntimeUI_MoveDown->m_Function = CreateRef<WrappedVoidNone>(RuntimeUI::RuntimeUIService::MoveDown);
		engineScripts.push_back(RuntimeUI_MoveDown);

		Ref<Script> RuntimeUI_MoveLeft = CreateRef<Script>();
		RuntimeUI_MoveLeft->m_ScriptName = "MoveLeft";
		RuntimeUI_MoveLeft->m_ID = Utility::GeneratePseudoRandomNumber();
		RuntimeUI_MoveLeft->m_ScriptType = ScriptType::Engine;
		RuntimeUI_MoveLeft->m_FuncType = WrappedFuncType::Void_None;
		RuntimeUI_MoveLeft->m_SectionLabel = "RuntimeUI";
		RuntimeUI_MoveLeft->m_Function = CreateRef<WrappedVoidNone>(RuntimeUI::RuntimeUIService::MoveLeft);
		engineScripts.push_back(RuntimeUI_MoveLeft);

		Ref<Script> RuntimeUI_MoveRight = CreateRef<Script>();
		RuntimeUI_MoveRight->m_ScriptName = "MoveRight";
		RuntimeUI_MoveRight->m_ID = Utility::GeneratePseudoRandomNumber();
		RuntimeUI_MoveRight->m_ScriptType = ScriptType::Engine;
		RuntimeUI_MoveRight->m_FuncType = WrappedFuncType::Void_None;
		RuntimeUI_MoveRight->m_SectionLabel = "RuntimeUI";
		RuntimeUI_MoveRight->m_Function = CreateRef<WrappedVoidNone>(RuntimeUI::RuntimeUIService::MoveRight);
		engineScripts.push_back(RuntimeUI_MoveRight);

		Ref<Script> RuntimeUI_OnPress = CreateRef<Script>();
		RuntimeUI_OnPress->m_ScriptName = "OnPress";
		RuntimeUI_OnPress->m_ID = Utility::GeneratePseudoRandomNumber();
		RuntimeUI_OnPress->m_ScriptType = ScriptType::Engine;
		RuntimeUI_OnPress->m_FuncType = WrappedFuncType::Void_None;
		RuntimeUI_OnPress->m_SectionLabel = "RuntimeUI";
		RuntimeUI_OnPress->m_Function = CreateRef<WrappedVoidNone>(RuntimeUI::RuntimeUIService::OnPress);
		engineScripts.push_back(RuntimeUI_OnPress);

		Ref<Script> EngineCore_CloseApplication = CreateRef<Script>();
		EngineCore_CloseApplication->m_ScriptName = "CloseApplication";
		EngineCore_CloseApplication->m_ID = Utility::GeneratePseudoRandomNumber();
		EngineCore_CloseApplication->m_ScriptType = ScriptType::Engine;
		EngineCore_CloseApplication->m_FuncType = WrappedFuncType::Void_None;
		EngineCore_CloseApplication->m_SectionLabel = "EngineCore";
		EngineCore_CloseApplication->m_Function = CreateRef<WrappedVoidNone>(EngineCore::CloseApplication);
		engineScripts.push_back(EngineCore_CloseApplication);

		Ref<Script> Client_SessionReadyCheck = CreateRef<Script>();
		Client_SessionReadyCheck->m_ScriptName = "SessionReadyCheck";
		Client_SessionReadyCheck->m_ID = Utility::GeneratePseudoRandomNumber();
		Client_SessionReadyCheck->m_ScriptType = ScriptType::Engine;
		Client_SessionReadyCheck->m_FuncType = WrappedFuncType::Void_None;
		Client_SessionReadyCheck->m_SectionLabel = "Client";
		Client_SessionReadyCheck->m_Function = CreateRef<WrappedVoidNone>(Network::Client::SessionReadyCheck);
		engineScripts.push_back(Client_SessionReadyCheck);
	}

	void ScriptService::Init()
	{
		s_ScriptingData = new ScriptingData();
		GenerateEngineScripts(s_AllEngineScripts);
		KG_VERIFY(s_ScriptingData, "Scripting System Init");
	}

	void ScriptService::Terminate()
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

	void ScriptService::LoadActiveScriptModule()
	{
#ifdef KG_DEBUG
		std::filesystem::path dllLocation { Projects::Project::GetAssetDirectory() / "Scripting\\Binary\\ExportBodyDebug.dll" };
#else
		std::filesystem::path dllLocation { Projects::Project::GetAssetDirectory() / "Scripting\\Binary\\ExportBody.dll" };
#endif
		
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
	void ScriptService::CloseActiveScriptModule()
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

	void ScriptService::LoadScriptFunction(Ref<Script> script, WrappedFuncType funcType)
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
	DefineInsertFunction(VoidUInt16, void, uint16_t)
	DefineInsertFunction(VoidStringBool, void, const std::string&, bool)
	DefineInsertFunction(VoidStringVoidPtr, void, const std::string&, void*)
	DefineInsertFunction(VoidStringString, void, const std::string&, const std::string&)
	DefineInsertFunction(VoidStringStringBool, void, const std::string&, const std::string&, bool)
	DefineInsertFunction(VoidStringStringString, void, const std::string&, const std::string&, const std::string&)
	DefineInsertFunction(VoidStringStringVec4, void, const std::string&, const std::string&, Math::vec4)
	DefineInsertFunction(VoidUInt64StringVoidPtr, void, uint64_t, const std::string&, void*)
	DefineInsertFunction(VoidPtrString, void*, const std::string&)
	DefineInsertFunction(VoidPtrUInt64String, void*, uint64_t, const std::string&)
	DefineInsertFunction(VoidUInt64Vec2, void, uint64_t, Math::vec2)
	DefineInsertFunction(VoidUInt64Vec3, void, uint64_t, Math::vec3)
	DefineInsertFunction(VoidUInt64Vec3Vec2, void, uint64_t, Math::vec3, Math::vec2)
	DefineInsertFunction(BoolUInt64String, bool, uint64_t, const std::string&)
	DefineInsertFunction(BoolUInt16, bool, uint16_t)
	DefineInsertFunction(UInt16None, uint16_t)
	DefineInsertFunction(Int32Int32Int32, int32_t, int32_t, int32_t)
	DefineInsertFunction(UInt64String, uint64_t, const std::string&)
	DefineInsertFunction(Vec2UInt64, Math::vec2, uint64_t)
	DefineInsertFunction(Vec3UInt64, Math::vec3, uint64_t)
	DefineInsertFunction(StringUInt64, const std::string&, uint64_t)

	// Engine Functions that need to be defined only in this file
	static void Log(const std::string& info)
	{
		KG_WARN(info);
	}

	void ScriptModuleBuilder::CreateScriptModule()
	{
		ScriptService::CloseActiveScriptModule();

		CreateModuleHeaderFile();
		CreateModuleCPPFile();
		CompileModuleCode(true);
		CompileModuleCode(false);

		ScriptService::LoadActiveScriptModule();
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
		outputStream << "#include <limits>\n";
		outputStream << "#include \"" << "Kargono/Math/MathAliases.h" << "\"\n"; // Include Math Library
		outputStream << "#include \"" << "Kargono/Core/KeyCodes.h" << "\"\n"; // Include KeyCodes
		outputStream << "#include \"" << "Kargono/Core/MouseCodes.h" << "\"\n"; // Include MouseCodes

		// Conversion Function from RValueToLValue
		outputStream << "template<typename T>\n";
		outputStream << "T& RValueToLValue(T && value)\n";
		outputStream << "{" << "\n";
		outputStream << "return value;\n";
		outputStream << "}" << "\n";

		outputStream << "namespace Kargono\n";
		outputStream << "{" << "\n";
		outputStream << "extern \"C\"" << "\n";
		outputStream << "\t{" << "\n";

		AddImportFunctionToHeaderFile(VoidNone, void) 
		AddImportFunctionToHeaderFile(VoidString, void, const std::string&) 
		AddImportFunctionToHeaderFile(VoidUInt16, void, uint16_t) 
		AddImportFunctionToHeaderFile(VoidStringBool, void, const std::string&, bool) 
		AddImportFunctionToHeaderFile(VoidStringVoidPtr, void, const std::string&, void*) 
		AddImportFunctionToHeaderFile(VoidStringString, void, const std::string&, const std::string&) 
		AddImportFunctionToHeaderFile(VoidStringStringBool, void, const std::string&, const std::string&, bool) 
		AddImportFunctionToHeaderFile(VoidStringStringString, void, const std::string&, const std::string&, const std::string&) 
		AddImportFunctionToHeaderFile(VoidStringStringVec4, void, const std::string&, const std::string&, Math::vec4) 
		AddImportFunctionToHeaderFile(VoidUInt64StringVoidPtr, void, uint64_t, const std::string&, void*)
		AddImportFunctionToHeaderFile(VoidPtrString, void*, const std::string&) 
		AddImportFunctionToHeaderFile(VoidPtrUInt64String, void*, uint64_t, const std::string&)
		AddImportFunctionToHeaderFile(VoidUInt64Vec3, void, uint64_t, Math::vec3)
		AddImportFunctionToHeaderFile(VoidUInt64Vec2, void, uint64_t, Math::vec2)
		AddImportFunctionToHeaderFile(VoidUInt64Vec3Vec2, void, uint64_t, Math::vec3, Math::vec2)
		AddImportFunctionToHeaderFile(BoolUInt64String, bool, uint64_t, const std::string&) 
		AddImportFunctionToHeaderFile(BoolUInt16, bool, uint16_t) 
		AddImportFunctionToHeaderFile(UInt16None, uint16_t) 
		AddImportFunctionToHeaderFile(UInt64String, uint64_t, const std::string&)
		AddImportFunctionToHeaderFile(Int32Int32Int32, int32_t, int32_t, int32_t)
		AddImportFunctionToHeaderFile(Vec2UInt64, Math::vec2, uint64_t)
		AddImportFunctionToHeaderFile(Vec3UInt64, Math::vec3, uint64_t)
		AddImportFunctionToHeaderFile(StringUInt64, const std::string&, uint64_t)
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
		outputStream << "namespace Kargono\n";
		outputStream << "{\n";

		// Insert Callable Function Definitions into CPP file
		AddEngineFunctionToCPPFileNoParameters(EnableReadyCheck, void)
		AddEngineFunctionToCPPFileNoParameters(RequestUserCount, void)
		AddEngineFunctionToCPPFileNoParameters(RequestJoinSession, void)
		AddEngineFunctionToCPPFileNoParameters(LeaveCurrentSession, void)
		AddEngineFunctionToCPPFileNoParameters(GetActiveSessionSlot, uint16_t)
		AddEngineFunctionToCPPFileOneParameters(Log, void, const std::string&)
		AddEngineFunctionToCPPFileOneParameters(TagComponent_GetTag, const std::string&, uint64_t)
		AddEngineFunctionToCPPFileOneParameters(PlaySoundFromName, void, const std::string&)
		AddEngineFunctionToCPPFileOneParameters(IsKeyPressed, bool, uint16_t)
		AddEngineFunctionToCPPFileOneParameters(SignalAll, void, uint16_t)
		AddEngineFunctionToCPPFileOneParameters(GetGameStateField, void*, const std::string&)
		AddEngineFunctionToCPPFileOneParameters(PlayStereoSoundFromName, void, const std::string&)
		AddEngineFunctionToCPPFileOneParameters(LoadInputModeByName, void, const std::string&)
		AddEngineFunctionToCPPFileOneParameters(LoadUserInterfaceFromName, void, const std::string&)
		AddEngineFunctionToCPPFileOneParameters(TransitionSceneFromName, void, const std::string&)
		AddEngineFunctionToCPPFileOneParameters(TransformComponent_GetTranslation, Math::vec3, uint64_t)
		AddEngineFunctionToCPPFileOneParameters(Rigidbody2DComponent_GetLinearVelocity, Math::vec2, uint64_t)
		AddEngineFunctionToCPPFileOneParameters(FindEntityHandleByName, uint64_t, const std::string&)
		AddEngineFunctionToCPPFileTwoParameters(CheckHasComponent, bool, uint64_t, const std::string&)
		AddEngineFunctionToCPPFileTwoParameters(GenerateRandomNumber, int32_t, int32_t, int32_t)
		AddEngineFunctionToCPPFileTwoParameters(SetDisplayWindow, void, const std::string&, bool)
		AddEngineFunctionToCPPFileTwoParameters(SetSelectedWidget, void, const std::string&, const std::string&)
		AddEngineFunctionToCPPFileTwoParameters(SetGameStateField, void, const std::string&, void*)
		AddEngineFunctionToCPPFileTwoParameters(SendAllEntityLocation, void, uint64_t, Math::vec3)
		AddEngineFunctionToCPPFileTwoParameters(Rigidbody2DComponent_SetLinearVelocity, void, uint64_t, Math::vec2)
		AddEngineFunctionToCPPFileTwoParameters(TransformComponent_SetTranslation, void, uint64_t, Math::vec3)
		AddEngineFunctionToCPPFileTwoParameters(GetEntityFieldByName, void*, uint64_t, const std::string&)
		AddEngineFunctionToCPPFileThreeParameters(SetWidgetSelectable, void, const std::string&, const std::string&, bool)
		AddEngineFunctionToCPPFileThreeParameters(SetWidgetText, void, const std::string&, const std::string&, const std::string&)
		AddEngineFunctionToCPPFileThreeParameters(SetWidgetTextColor, void, const std::string&, const std::string&, Math::vec4)
		AddEngineFunctionToCPPFileThreeParameters(SetWidgetBackgroundColor, void, const std::string&, const std::string&, Math::vec4)
		AddEngineFunctionToCPPFileThreeParameters(SetEntityFieldByName, void, uint64_t, const std::string&, void*)
		AddEngineFunctionToCPPFileThreeParameters(SendAllEntityPhysics, void, uint64_t, Math::vec3, Math::vec2)

		// Insert FuncPointer Importing for DLL processing
		AddImportFunctionToCPPFile(VoidNone, void)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(EnableReadyCheck)
		AddEngineFunctionToCPPFileEnd(RequestUserCount)
		AddEngineFunctionToCPPFileEnd(RequestJoinSession)
		AddEngineFunctionToCPPFileEnd(LeaveCurrentSession)
		outputStream << "}\n";
		AddImportFunctionToCPPFile(VoidUInt16, void, uint16_t)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(SignalAll)
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
		AddImportFunctionToCPPFile(VoidUInt64Vec3, void, uint64_t, Math::vec3)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(SendAllEntityLocation)
		AddEngineFunctionToCPPFileEnd(TransformComponent_SetTranslation)
		outputStream << "}\n";
		AddImportFunctionToCPPFile(VoidUInt64Vec2, void, uint64_t, Math::vec2)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(Rigidbody2DComponent_SetLinearVelocity)
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
		AddImportFunctionToCPPFile(Vec2UInt64, Math::vec2, uint64_t)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(Rigidbody2DComponent_GetLinearVelocity)
		outputStream << "}\n";
		AddImportFunctionToCPPFile(UInt64String, uint64_t, const std::string&)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(FindEntityHandleByName)
		outputStream << "}\n";
		AddImportFunctionToCPPFile(VoidUInt64StringVoidPtr, void, uint64_t, const std::string&, void*)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(SetEntityFieldByName)
		outputStream << "}\n";
		AddImportFunctionToCPPFile(VoidUInt64Vec3Vec2, void, uint64_t, Math::vec3, Math::vec2)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(SendAllEntityPhysics)
		outputStream << "}\n";
		AddImportFunctionToCPPFile(VoidPtrUInt64String, void*, uint64_t, const std::string&)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(GetEntityFieldByName)
		outputStream << "}\n";
		AddImportFunctionToCPPFile(VoidPtrString, void*, const std::string&)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(GetGameStateField)
		outputStream << "}\n";
		AddImportFunctionToCPPFile(Int32Int32Int32, int32_t, int32_t, int32_t)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(GenerateRandomNumber)
		outputStream << "}\n";
		AddImportFunctionToCPPFile(BoolUInt16, bool, uint16_t)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(IsKeyPressed)
		outputStream << "}\n";
		AddImportFunctionToCPPFile(StringUInt64, const std::string&, uint64_t)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(TagComponent_GetTag)
		outputStream << "}\n";	

		// Write scripts into a single cpp file
		for (auto& [handle, asset] : Assets::AssetManager::s_ScriptRegistry)
		{
			if (asset.Data.GetSpecificFileData<Assets::ScriptMetaData>()->ScriptType == ScriptType::Engine)
			{
				continue;
			}
			outputStream << Utility::FileSystem::ReadFileString(Projects::Project::GetAssetDirectory() / asset.Data.IntermediateLocation);
			outputStream << '\n';
		}
		outputStream << "}\n";

		std::filesystem::path file = { Projects::Project::GetAssetDirectory() / "Scripting/Binary/ExportBody.cpp" };

		Utility::FileSystem::WriteFileString(file, outputStream.str());
	}

	void ScriptModuleBuilder::CompileModuleCode(bool createDebug)
	{
		Utility::FileSystem::CreateNewDirectory(Projects::Project::GetAssetDirectory() / "Scripting/Binary");
		std::filesystem::path binaryPath { Projects::Project::GetAssetDirectory() / "Scripting/Binary/" };
		std::filesystem::path binaryFile;
		std::filesystem::path objectPath;
		if (createDebug)
		{
			binaryFile = { binaryPath / "ExportBodyDebug.dll" };
			objectPath = { binaryPath / "ExportBody.obj" };
		}
		else
		{
			binaryFile = { binaryPath / "ExportBody.dll" };
			objectPath = { binaryPath / "ExportBody.obj" };
		}

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
		if (createDebug)
		{
			outputStream << "/MDd "; // Specify Debug Multi-threaded DLL Runtime Library
		}
		else
		{
			outputStream << "/MD "; // Specify Multi-threaded DLL Runtime Library
		}
		outputStream << "/std:c++20 "; // Specify Language Version
		outputStream << "/I../Dependencies/glm "; // Include GLM
		outputStream << "/I../Engine/Source "; // Include Kargono as Include Directory
		outputStream << "/EHsc "; // Specifies the handling of exceptions and call stack unwinding. Uses the commands /EH, /EHs, and /EHc together
		outputStream << "/DKARGONO_EXPORTS "; // Define Macros for Exporting DLL Functions

		if (createDebug)
		{
			outputStream << "/Z7 "; // Add debug info to executable
		}
		outputStream << "/Fo" << binaryPath.string() << ' '; // Define Intermediate Location
		outputStream << sourcePath.string() << " "; // Compile scripting source file (ExportBody.cpp)

		outputStream << " && "; // Combine commands
		// Start Linking Stage
		outputStream << "link "; // Start link command
		outputStream << "/DLL "; // Specify output as a shared library
		outputStream << "/ignore:4099 "; // Ignores warning about missing pbd files for .obj files (I generate the symbols inside of the .obj file with /Z7 flag) 
		if (createDebug)
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
		ImportInsertFunction(VoidUInt16) 
		ImportInsertFunction(VoidString) 
		ImportInsertFunction(VoidPtrString) 
		ImportInsertFunction(VoidStringBool) 
		ImportInsertFunction(VoidStringVoidPtr) 
		ImportInsertFunction(VoidStringString) 
		ImportInsertFunction(VoidStringStringBool) 
		ImportInsertFunction(VoidStringStringString) 
		ImportInsertFunction(VoidStringStringVec4) 
		ImportInsertFunction(VoidUInt64StringVoidPtr)
		ImportInsertFunction(VoidPtrUInt64String)
		ImportInsertFunction(VoidUInt64Vec3)
		ImportInsertFunction(VoidUInt64Vec2)
		ImportInsertFunction(VoidUInt64Vec3Vec2)
		ImportInsertFunction(BoolUInt64String)
		ImportInsertFunction(BoolUInt16)
		ImportInsertFunction(UInt16None)
		ImportInsertFunction(UInt64String)
		ImportInsertFunction(Int32Int32Int32)
		ImportInsertFunction(Vec2UInt64)
		ImportInsertFunction(Vec3UInt64)
		ImportInsertFunction(StringUInt64)
		AddEngineFunctionPointerToDll(LeaveCurrentSession, Network::Client::LeaveCurrentSession,VoidNone) 
		AddEngineFunctionPointerToDll(EnableReadyCheck, Network::Client::EnableReadyCheck,VoidNone) 
		AddEngineFunctionPointerToDll(RequestJoinSession, Network::Client::RequestJoinSession,VoidNone) 
		AddEngineFunctionPointerToDll(SendAllEntityPhysics, Network::Client::SendAllEntityPhysics,VoidUInt64Vec3Vec2)
		AddEngineFunctionPointerToDll(RequestUserCount, Network::Client::RequestUserCount,VoidNone)
		AddEngineFunctionPointerToDll(GetActiveSessionSlot, Network::Client::GetActiveSessionSlot, UInt16None)
		AddEngineFunctionPointerToDll(SendAllEntityLocation, Network::Client::SendAllEntityLocation, VoidUInt64Vec3)
		AddEngineFunctionPointerToDll(SignalAll, Network::Client::SignalAll, VoidUInt16)
		AddEngineFunctionPointerToDll(Log, Scripting::Log,VoidString) 
		AddEngineFunctionPointerToDll(PlaySoundFromName, Audio::AudioService::PlaySoundFromName,VoidString) 
		AddEngineFunctionPointerToDll(PlayStereoSoundFromName, Audio::AudioService::PlayStereoSoundFromName,VoidString) 
		AddEngineFunctionPointerToDll(LoadInputModeByName, Input::InputModeService::SetActiveInputModeByName,VoidString) 
		AddEngineFunctionPointerToDll(IsKeyPressed, Input::InputPolling::IsKeyPressed,BoolUInt16) 
		AddEngineFunctionPointerToDll(LoadUserInterfaceFromName, RuntimeUI::RuntimeUIService::LoadUserInterfaceFromName,VoidString) 
		AddEngineFunctionPointerToDll(TransitionSceneFromName, Scenes::Scene::TransitionSceneFromName,VoidString) 
		AddEngineFunctionPointerToDll(SetDisplayWindow, RuntimeUI::RuntimeUIService::SetDisplayWindow,VoidStringBool) 
		AddEngineFunctionPointerToDll(SetGameStateField, Scenes::GameState::SetActiveGameStateField, VoidStringVoidPtr) 
		AddEngineFunctionPointerToDll(GetGameStateField, Scenes::GameState::GetActiveGameStateField, VoidPtrString) 
		AddEngineFunctionPointerToDll(SetWidgetText, RuntimeUI::RuntimeUIService::SetWidgetText,VoidStringStringString) 
		AddEngineFunctionPointerToDll(SetSelectedWidget, RuntimeUI::RuntimeUIService::SetSelectedWidget,VoidStringString) 
		AddEngineFunctionPointerToDll(SetWidgetTextColor, RuntimeUI::RuntimeUIService::SetWidgetTextColor,VoidStringStringVec4) 
		AddEngineFunctionPointerToDll(SetWidgetBackgroundColor, RuntimeUI::RuntimeUIService::SetWidgetBackgroundColor,VoidStringStringVec4) 
		AddEngineFunctionPointerToDll(SetWidgetSelectable, RuntimeUI::RuntimeUIService::SetWidgetSelectable,VoidStringStringBool) 
		AddEngineFunctionPointerToDll(CheckHasComponent, Scenes::Scene::CheckHasComponent, BoolUInt64String)
		AddEngineFunctionPointerToDll(SetEntityFieldByName, Scenes::SceneService::SetEntityFieldByName, VoidUInt64StringVoidPtr)
		AddEngineFunctionPointerToDll(GetEntityFieldByName, Scenes::SceneService::GetEntityFieldByName, VoidPtrUInt64String)
		AddEngineFunctionPointerToDll(FindEntityHandleByName, Scenes::Scene::FindEntityHandleByName, UInt64String)
		AddEngineFunctionPointerToDll(TransformComponent_GetTranslation, Scenes::SceneService::TransformComponentGetTranslation, Vec3UInt64)
		AddEngineFunctionPointerToDll(TransformComponent_SetTranslation, Scenes::SceneService::TransformComponentSetTranslation, VoidUInt64Vec3)
		AddEngineFunctionPointerToDll(Rigidbody2DComponent_SetLinearVelocity, Scenes::SceneService::Rigidbody2DComponent_SetLinearVelocity, VoidUInt64Vec2)
		AddEngineFunctionPointerToDll(Rigidbody2DComponent_GetLinearVelocity, Scenes::SceneService::Rigidbody2DComponent_GetLinearVelocity, Vec2UInt64)
		AddEngineFunctionPointerToDll(TagComponent_GetTag, Scenes::SceneService::TagComponentGetTag, StringUInt64)
		AddEngineFunctionPointerToDll(GenerateRandomNumber, Utility::RandomService::GenerateRandomNumber, Int32Int32Int32)
	}
}
