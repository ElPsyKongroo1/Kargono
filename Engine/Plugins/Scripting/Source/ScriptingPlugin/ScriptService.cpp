#include "kgpch.h"

#include "ScriptingPlugin/ScriptService.h"

#include "Kargono/Core/Engine.h"
#include "ScriptingPlugin/ScriptModuleBuilder.h"
#include "AssetsPlugin/AssetService.h"
#include "Kargono/Scenes/Scene.h"
#include "FileSystemPlugin/FileSystem.h"
#include "Kargono/Projects/Project.h"
#include "AudioPlugin/Audio.h"
#include "RuntimeUIPlugin/RuntimeUI.h"
#include "InputMapPlugin/InputMap.h"
#include "InputPlugin/InputService.h"
#include "NetworkPlugin/Client.h"
#include "Kargono/Scenes/GameState.h"
#include "Kargono/Utility/Operations.h"
#include "Kargono/Utility/Random.h"
#include "AIPlugin/AIService.h"
#include "ScriptingPlugin/ScriptCompilerService.h"
#include "EventsPlugin/EditorEvent.h"
#include "Physics2DPlugin/Physics2DCommon.h"
#include "ParticlesPlugin/ParticleService.h"

#if defined(KG_PLATFORM_WINDOWS)
#include "API/Platform/WindowsBackendAPI.h"
#elif defined(KG_PLATFORM_LINUX)
#include "API/Platform/LinuxBackendAPI.h"
#else
#error "Platform not supported"
#endif

namespace Kargono::Scripting
{
	static Utility::PseudoGenerator s_IDGenerator{ 0xc3bc4ead8efa4c3a };

	struct ScriptingData
	{
#if defined(KG_PLATFORM_WINDOWS)
		HINSTANCE* DLLInstance = nullptr;
#elif defined(KG_PLATFORM_LINUX)
		void* DLLInstance = nullptr;
#endif
	};

	static ScriptingData* s_ScriptingData = nullptr;
}

namespace Kargono::Scripting
{
	// Engine Functions that need to be defined only in this file
	static void Log(const std::string& scriptName, const std::string& scriptLine, const std::string& info)
	{
		KG_WARN("[{}:{}]: {}", scriptName, scriptLine, info);
	}
	static void AddDebugLine(Math::vec3 startPoint, Math::vec3 endPoint)
	{
		Events::DebugLineData lineData{ startPoint, endPoint };
		Events::ManageEditor newEvent = Events::ManageEditor(lineData);
		EngineService::OnEvent(&newEvent);
	}
	static void AddDebugPoint(Math::vec3 point)
	{
		Events::DebugPointData pointData{ point };
		Events::ManageEditor newEvent = Events::ManageEditor(pointData);
		EngineService::OnEvent(&newEvent);
	}
	static void ClearDebugPoints()
	{
		Events::ManageEditor newEvent = Events::ManageEditor(Events::ManageEditorAction::ClearDebugPoints);
		EngineService::OnEvent(&newEvent);
	}
	static void ClearDebugLines()
	{
		Events::ManageEditor newEvent = Events::ManageEditor(Events::ManageEditorAction::ClearDebugLines);
		EngineService::OnEvent(&newEvent);
	}

	static void ApplicationResize(uint16_t resolution)
	{
		static ScreenResolution s_NewResolution;
		s_NewResolution = (ScreenResolution)resolution;

		EngineService::SubmitToMainThread([&]()
		{
			// Send app resize event to either the runtime or the editor
			Math::uvec2 resolution= Utility::ScreenResolutionToVec2(s_NewResolution);
			Events::ApplicationResizeEvent newEvent = Events::ApplicationResizeEvent(resolution.x, resolution.y);
			EngineService::OnEvent(&newEvent);
		});
	}

	std::vector<Ref<Script>> ScriptService::s_AllEngineScripts {};

	static void GenerateEngineScripts(std::vector<Ref<Script>>& engineScripts)
	{
		Utility::PseudoRandomService::ResetState(s_IDGenerator);
		engineScripts.clear();
		// RuntimeUI
		Ref<Script> RuntimeUI_MoveUp = CreateRef<Script>();
		RuntimeUI_MoveUp->m_ScriptName = "MoveUp";
		RuntimeUI_MoveUp->m_ID = Utility::PseudoRandomService::GenerateNumber(s_IDGenerator);
		RuntimeUI_MoveUp->m_ScriptType = ScriptType::Engine;
		RuntimeUI_MoveUp->m_FuncType = WrappedFuncType::Void_None;
		RuntimeUI_MoveUp->m_SectionLabel = "UserInterface";
		RuntimeUI_MoveUp->m_Function = CreateRef<WrappedVoidNone>(RuntimeUI::RuntimeUIService::MoveUp);
		engineScripts.push_back(RuntimeUI_MoveUp);

		Ref<Script> RuntimeUI_MoveDown = CreateRef<Script>();
		RuntimeUI_MoveDown->m_ScriptName = "MoveDown";
		RuntimeUI_MoveDown->m_ID = Utility::PseudoRandomService::GenerateNumber(s_IDGenerator);
		RuntimeUI_MoveDown->m_ScriptType = ScriptType::Engine;
		RuntimeUI_MoveDown->m_FuncType = WrappedFuncType::Void_None;
		RuntimeUI_MoveDown->m_SectionLabel = "UserInterface";
		RuntimeUI_MoveDown->m_Function = CreateRef<WrappedVoidNone>(RuntimeUI::RuntimeUIService::MoveDown);
		engineScripts.push_back(RuntimeUI_MoveDown);

		Ref<Script> RuntimeUI_MoveLeft = CreateRef<Script>();
		RuntimeUI_MoveLeft->m_ScriptName = "MoveLeft";
		RuntimeUI_MoveLeft->m_ID = Utility::PseudoRandomService::GenerateNumber(s_IDGenerator);
		RuntimeUI_MoveLeft->m_ScriptType = ScriptType::Engine;
		RuntimeUI_MoveLeft->m_FuncType = WrappedFuncType::Void_None;
		RuntimeUI_MoveLeft->m_SectionLabel = "UserInterface";
		RuntimeUI_MoveLeft->m_Function = CreateRef<WrappedVoidNone>(RuntimeUI::RuntimeUIService::MoveLeft);
		engineScripts.push_back(RuntimeUI_MoveLeft);

		Ref<Script> RuntimeUI_MoveRight = CreateRef<Script>();
		RuntimeUI_MoveRight->m_ScriptName = "MoveRight";
		RuntimeUI_MoveRight->m_ID = Utility::PseudoRandomService::GenerateNumber(s_IDGenerator);
		RuntimeUI_MoveRight->m_ScriptType = ScriptType::Engine;
		RuntimeUI_MoveRight->m_FuncType = WrappedFuncType::Void_None;
		RuntimeUI_MoveRight->m_SectionLabel = "UserInterface";
		RuntimeUI_MoveRight->m_Function = CreateRef<WrappedVoidNone>(RuntimeUI::RuntimeUIService::MoveRight);
		engineScripts.push_back(RuntimeUI_MoveRight);

		Ref<Script> RuntimeUI_OnPress = CreateRef<Script>();
		RuntimeUI_OnPress->m_ScriptName = "OnPress";
		RuntimeUI_OnPress->m_ID = Utility::PseudoRandomService::GenerateNumber(s_IDGenerator);
		RuntimeUI_OnPress->m_ScriptType = ScriptType::Engine;
		RuntimeUI_OnPress->m_FuncType = WrappedFuncType::Void_None;
		RuntimeUI_OnPress->m_SectionLabel = "UserInterface";
		RuntimeUI_OnPress->m_Function = CreateRef<WrappedVoidNone>(RuntimeUI::RuntimeUIService::OnPress);
		engineScripts.push_back(RuntimeUI_OnPress);

		Ref<Script> EngineCore_CloseApplication = CreateRef<Script>();
		EngineCore_CloseApplication->m_ScriptName = "CloseApplication";
		EngineCore_CloseApplication->m_ID = Utility::PseudoRandomService::GenerateNumber(s_IDGenerator);
		EngineCore_CloseApplication->m_ScriptType = ScriptType::Engine;
		EngineCore_CloseApplication->m_FuncType = WrappedFuncType::Void_None;
		EngineCore_CloseApplication->m_SectionLabel = "Engine";
		EngineCore_CloseApplication->m_Function = CreateRef<WrappedVoidNone>(EngineService::SubmitApplicationCloseEvent);
		engineScripts.push_back(EngineCore_CloseApplication);

		Ref<Script> Client_SessionReadyCheck = CreateRef<Script>();
		Client_SessionReadyCheck->m_ScriptName = "SessionReadyCheck";
		Client_SessionReadyCheck->m_ID = Utility::PseudoRandomService::GenerateNumber(s_IDGenerator);
		Client_SessionReadyCheck->m_ScriptType = ScriptType::Engine;
		Client_SessionReadyCheck->m_FuncType = WrappedFuncType::Void_None;
		Client_SessionReadyCheck->m_SectionLabel = "Network";
		Client_SessionReadyCheck->m_Function = CreateRef<WrappedVoidNone>(Network::ClientService::SessionReadyCheck);
		engineScripts.push_back(Client_SessionReadyCheck);

		Ref<Script> Client_RequestUserCount = CreateRef<Script>();
		Client_RequestUserCount->m_ScriptName = "RequestUserCount";
		Client_RequestUserCount->m_ID = Utility::PseudoRandomService::GenerateNumber(s_IDGenerator);
		Client_RequestUserCount->m_ScriptType = ScriptType::Engine;
		Client_RequestUserCount->m_FuncType = WrappedFuncType::Void_None;
		Client_RequestUserCount->m_SectionLabel = "Network";
		Client_RequestUserCount->m_Function = CreateRef<WrappedVoidNone>(Network::ClientService::RequestUserCount);
		engineScripts.push_back(Client_RequestUserCount);

		Ref<Script> Client_RequestJoinSession = CreateRef<Script>();
		Client_RequestJoinSession->m_ScriptName = "RequestJoinSession";
		Client_RequestJoinSession->m_ID = Utility::PseudoRandomService::GenerateNumber(s_IDGenerator);
		Client_RequestJoinSession->m_ScriptType = ScriptType::Engine;
		Client_RequestJoinSession->m_FuncType = WrappedFuncType::Void_None;
		Client_RequestJoinSession->m_SectionLabel = "Network";
		Client_RequestJoinSession->m_Function = CreateRef<WrappedVoidNone>(Network::ClientService::RequestJoinSession);
		engineScripts.push_back(Client_RequestJoinSession);
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
		// Get the path to the script dll
#if defined(KG_PLATFORM_WINDOWS)
	#if defined(KG_DEBUG) 
			std::filesystem::path dllLocation { Projects::ProjectService::GetActiveIntermediateDirectory() / "Script\\ExportBodyDebug.dll" };
	#else
			std::filesystem::path dllLocation { Projects::ProjectService::GetActiveIntermediateDirectory() / "Script\\ExportBody.dll" };
	#endif
#endif

#if defined(KG_PLATFORM_LINUX)
	#if defined(KG_DEBUG) 
			std::filesystem::path dllLocation { Projects::ProjectService::GetActiveIntermediateDirectory() / "Script/ExportBodyDebug.so" };
	#else
			std::filesystem::path dllLocation { Projects::ProjectService::GetActiveIntermediateDirectory() / "Script/ExportBody.so" };
	#endif
#endif

		// Rebuild shared library if no library exists
		static bool attemptedToRebuild = false;
		if (!Utility::FileSystem::PathExists(dllLocation))
		{
			if (attemptedToRebuild)
			{
				KG_ERROR("Multiple attempts to open dll with no file present! Might be an issue generating shared library.");
				return;
			}
			KG_WARN("Attempt to open scripting dll, however, none exists. Attempting to create new Shared Lib.");
			attemptedToRebuild = true;
			ScriptModuleBuilder::CreateScriptModule();
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
			KG_INFO("Closing existing script module");
			CloseActiveScriptModule();
		}
#if defined(KG_PLATFORM_WINDOWS)
		s_ScriptingData->DLLInstance = new HINSTANCE();
		*(s_ScriptingData->DLLInstance) = LoadLibrary(dllLocation.c_str());
		if (*s_ScriptingData->DLLInstance == NULL)
		{
			KG_CRITICAL("Failed to open dll with path {} with an error code of {}", dllLocation.string(), GetLastError());
			CloseActiveScriptModule();
			return;
	}
#elif defined(KG_PLATFORM_LINUX)
		s_ScriptingData->DLLInstance = dlopen(dllLocation.c_str(), RTLD_LAZY);
		if (s_ScriptingData->DLLInstance == NULL)
		{
			KG_CRITICAL("Failed to open dll with path {}", dllLocation.string());
			CloseActiveScriptModule();
			return;
		}
#endif

		ScriptModuleBuilder::AttachEngineFunctionsToModule();

		KG_VERIFY(s_ScriptingData->DLLInstance, "Scripting Module Opened");

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

		
#if defined(KG_PLATFORM_WINDOWS)
		if (*s_ScriptingData->DLLInstance == NULL)
		{
			KG_WARN("Attempt to close scripting dll, however, DLLInstance value is 0 (NULL)");
			s_ScriptingData->DLLInstance = nullptr;
			return;
		}
		FreeLibrary(*s_ScriptingData->DLLInstance);
		delete s_ScriptingData->DLLInstance;
#elif defined(KG_PLATFORM_LINUX)
		if (s_ScriptingData->DLLInstance == NULL)
		{
			KG_WARN("Attempt to close scripting dll, however, DLLInstance value is 0 (NULL)");
			s_ScriptingData->DLLInstance = nullptr;
			return;
		}
		dlclose(s_ScriptingData->DLLInstance);
#endif

		s_ScriptingData->DLLInstance = nullptr;

		KG_VERIFY(!s_ScriptingData->DLLInstance, "Close Scripting DLL");
	}

	void ScriptService::LoadScriptFunction(Ref<Script> script, WrappedFuncType funcType)
	{
		KG_ASSERT(script);
		if (!s_ScriptingData)
		{
			KG_CRITICAL("Attempt to load a scripting function, however, ScriptEngine is not valid");
			return;
		}
		if (!s_ScriptingData->DLLInstance)
		{
			return;
		}

#if defined(KG_PLATFORM_WINDOWS)
		if (*s_ScriptingData->DLLInstance == NULL)
		{
			return;
		}
#endif

		if (script->m_ScriptType == ScriptType::Engine)
		{
			return;
		}

		if (funcType == WrappedFuncType::ArbitraryFunction)
		{
			script->m_Function = nullptr;
			return;
		}

		switch (funcType)
		{
		case WrappedFuncType::Void_None:
		{
			script->m_Function = CreateRef<WrappedVoidNone>();
#if defined(KG_PLATFORM_WINDOWS)
			((WrappedVoidNone*)script->m_Function.get())->m_Value = reinterpret_cast<void_none>(GetProcAddress(*s_ScriptingData->DLLInstance, script->m_ScriptName.c_str()));
#endif
#if defined(KG_PLATFORM_LINUX)
			((WrappedVoidNone*)script->m_Function.get())->m_Value = reinterpret_cast<void_none>(dlsym(s_ScriptingData->DLLInstance, script->m_ScriptName.c_str()));
#endif
			break;
		}

		case WrappedFuncType::Void_String:
		{
			script->m_Function = CreateRef<WrappedVoidString>();
#if defined(KG_PLATFORM_WINDOWS)
			((WrappedVoidString*)script->m_Function.get())->m_Value = reinterpret_cast<void_string>(GetProcAddress(*s_ScriptingData->DLLInstance, script->m_ScriptName.c_str()));
#elif defined(KG_PLATFORM_LINUX)
			((WrappedVoidString*)script->m_Function.get())->m_Value = reinterpret_cast<void_string>(dlsym(s_ScriptingData->DLLInstance, script->m_ScriptName.c_str()));
#endif
			break;
		}

		case WrappedFuncType::Void_Float:
		{
			script->m_Function = CreateRef<WrappedVoidFloat>();
#if defined(KG_PLATFORM_WINDOWS)
			((WrappedVoidFloat*)script->m_Function.get())->m_Value = reinterpret_cast<void_float>(GetProcAddress(*s_ScriptingData->DLLInstance, script->m_ScriptName.c_str()));
#elif defined(KG_PLATFORM_LINUX)
			((WrappedVoidFloat*)script->m_Function.get())->m_Value = reinterpret_cast<void_float>(dlsym(s_ScriptingData->DLLInstance, script->m_ScriptName.c_str()));
#endif
			break;
		}

		case WrappedFuncType::Void_UInt16:
		{
			script->m_Function = CreateRef<WrappedVoidUInt16>();
#if defined(KG_PLATFORM_WINDOWS)
			((WrappedVoidUInt16*)script->m_Function.get())->m_Value = reinterpret_cast<void_uint16>(GetProcAddress(*s_ScriptingData->DLLInstance, script->m_ScriptName.c_str()));
#elif defined(KG_PLATFORM_LINUX)
			((WrappedVoidUInt16*)script->m_Function.get())->m_Value = reinterpret_cast<void_uint16>(dlsym(s_ScriptingData->DLLInstance, script->m_ScriptName.c_str()));
#endif
			break;
		}
		case WrappedFuncType::Void_UInt32:
		{
			script->m_Function = CreateRef<WrappedVoidUInt32>();
#if defined(KG_PLATFORM_WINDOWS)
			((WrappedVoidUInt32*)script->m_Function.get())->m_Value = reinterpret_cast<void_uint32>(GetProcAddress(*s_ScriptingData->DLLInstance, script->m_ScriptName.c_str()));
#elif defined(KG_PLATFORM_LINUX)
			((WrappedVoidUInt32*)script->m_Function.get())->m_Value = reinterpret_cast<void_uint32>(dlsym(s_ScriptingData->DLLInstance, script->m_ScriptName.c_str()));
#endif
			break;
		}
		case WrappedFuncType::Void_UInt32UInt32:
		{
			script->m_Function = CreateRef<WrappedVoidUInt32UInt32>();
#if defined(KG_PLATFORM_WINDOWS)
			((WrappedVoidUInt32UInt32*)script->m_Function.get())->m_Value = reinterpret_cast<void_uint32uint32>(GetProcAddress(*s_ScriptingData->DLLInstance, script->m_ScriptName.c_str()));
#elif defined(KG_PLATFORM_LINUX)
			((WrappedVoidUInt32UInt32*)script->m_Function.get())->m_Value = reinterpret_cast<void_uint32uint32>(dlsym(s_ScriptingData->DLLInstance, script->m_ScriptName.c_str()));
#endif
			break;
		}

		case WrappedFuncType::Void_Entity:
		{
			script->m_Function = CreateRef<WrappedVoidEntity>();
#if defined(KG_PLATFORM_WINDOWS)
			((WrappedVoidEntity*)script->m_Function.get())->m_Value = reinterpret_cast<void_uint64>(GetProcAddress(*s_ScriptingData->DLLInstance, script->m_ScriptName.c_str()));
#elif defined(KG_PLATFORM_LINUX)
			((WrappedVoidEntity*)script->m_Function.get())->m_Value = reinterpret_cast<void_uint64>(dlsym(s_ScriptingData->DLLInstance, script->m_ScriptName.c_str()));
#endif
			break;
		}

		case WrappedFuncType::Void_Bool:
		{
			script->m_Function = CreateRef<WrappedVoidBool>();
#if defined(KG_PLATFORM_WINDOWS)
			((WrappedVoidBool*)script->m_Function.get())->m_Value = reinterpret_cast<void_bool>(GetProcAddress(*s_ScriptingData->DLLInstance, script->m_ScriptName.c_str()));
#elif defined(KG_PLATFORM_LINUX)
			((WrappedVoidBool*)script->m_Function.get())->m_Value = reinterpret_cast<void_bool>(dlsym(s_ScriptingData->DLLInstance, script->m_ScriptName.c_str()));
#endif
			break;
		}

		case WrappedFuncType::Void_EntityFloat:
		{
			script->m_Function = CreateRef<WrappedVoidEntityFloat>();
#if defined(KG_PLATFORM_WINDOWS)
			((WrappedVoidEntityFloat*)script->m_Function.get())->m_Value = reinterpret_cast<void_uint64float>(GetProcAddress(*s_ScriptingData->DLLInstance, script->m_ScriptName.c_str()));
#elif defined(KG_PLATFORM_LINUX)
			((WrappedVoidEntityFloat*)script->m_Function.get())->m_Value = reinterpret_cast<void_uint64float>(dlsym(s_ScriptingData->DLLInstance, script->m_ScriptName.c_str()));
#endif
			break;
		}

		case WrappedFuncType::Void_UInt32EntityEntityFloat:
		{
			script->m_Function = CreateRef<WrappedVoidUInt32EntityEntityFloat>();
#if defined(KG_PLATFORM_WINDOWS)
			((WrappedVoidUInt32EntityEntityFloat*)script->m_Function.get())->m_Value = reinterpret_cast<void_uint32uint64uint64float>(GetProcAddress(*s_ScriptingData->DLLInstance, script->m_ScriptName.c_str()));
#elif defined(KG_PLATFORM_LINUX)
			((WrappedVoidUInt32EntityEntityFloat*)script->m_Function.get())->m_Value = reinterpret_cast<void_uint32uint64uint64float>(dlsym(s_ScriptingData->DLLInstance, script->m_ScriptName.c_str()));
#endif
			break;
		}
		case WrappedFuncType::Bool_None:
		{
			script->m_Function = CreateRef<WrappedBoolNone>();
#if defined(KG_PLATFORM_WINDOWS)
			((WrappedBoolNone*)script->m_Function.get())->m_Value = reinterpret_cast<bool_none>(GetProcAddress(*s_ScriptingData->DLLInstance, script->m_ScriptName.c_str()));
#elif defined(KG_PLATFORM_LINUX)
			((WrappedBoolNone*)script->m_Function.get())->m_Value = reinterpret_cast<bool_none>(dlsym(s_ScriptingData->DLLInstance, script->m_ScriptName.c_str()));
#endif
			break;
		}

		case WrappedFuncType::Bool_Entity:
		{
			script->m_Function = CreateRef<WrappedBoolEntity>();
#if defined(KG_PLATFORM_WINDOWS)
			((WrappedBoolEntity*)script->m_Function.get())->m_Value = reinterpret_cast<bool_uint64>(GetProcAddress(*s_ScriptingData->DLLInstance, script->m_ScriptName.c_str()));
#elif defined(KG_PLATFORM_LINUX)
			((WrappedBoolEntity*)script->m_Function.get())->m_Value = reinterpret_cast<bool_uint64>(dlsym(s_ScriptingData->DLLInstance, script->m_ScriptName.c_str()));
#endif
			break;
		}
		case WrappedFuncType::Bool_EntityEntity:
		{
			script->m_Function = CreateRef<WrappedBoolEntityEntity>();
#if defined(KG_PLATFORM_WINDOWS)
			((WrappedBoolEntityEntity*)script->m_Function.get())->m_Value = reinterpret_cast<bool_uint64uint64>(GetProcAddress(*s_ScriptingData->DLLInstance, script->m_ScriptName.c_str()));
#elif defined(KG_PLATFORM_LINUX)
			((WrappedBoolEntityEntity*)script->m_Function.get())->m_Value = reinterpret_cast<bool_uint64uint64>(dlsym(s_ScriptingData->DLLInstance, script->m_ScriptName.c_str()));
#endif
			break;
		}
		default:
		{
			KG_ERROR("Invalid WrappedFuncType in LoadScriptFunction!");
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


#if defined(KG_PLATFORM_WINDOWS)
#define ImportInsertFunction(name) \
	s_Add##name = reinterpret_cast<Void_String_Func##name>(GetProcAddress(*s_ScriptingData->DLLInstance, "Add"#name ));\
	if (!s_Add##name)\
	{\
		KG_CRITICAL("Could not load {} function from scripting dll", "" #name);\
		return;\
	}

#elif defined(KG_PLATFORM_LINUX)
#define ImportInsertFunction(name) \
	s_Add##name = reinterpret_cast<Void_String_Func##name>(dlsym(s_ScriptingData->DLLInstance, "Add"#name ));\
	if (!s_Add##name)\
	{\
		KG_CRITICAL("Could not load {} function from scripting dll", "" #name);\
		return;\
	}
#endif
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

#define AddEngineFunctionToCPPFileFourParameters(name, returnType, parameter1, parameter2, parameter3, parameter4)\
	outputStream << "static std::function<" #returnType "(" #parameter1 " a, " #parameter2 " b, " #parameter3 " c,  " #parameter4 " d)> " #name "Ptr {};\n"; \
	outputStream << #returnType " " #name "(" #parameter1 " a, " #parameter2 " b, " #parameter3 " c, " #parameter4 " d)\n"; \
	outputStream << "{\n"; \
	outputStream << "\t"; \
	if (!std::is_same<returnType, void>::value) \
	{\
	outputStream << "return "; \
	}\
	outputStream << #name "Ptr(a, b, c, d);\n"; \
	outputStream << "}\n";

#define AddEngineFunctionToCPPFileEnd(name) \
	outputStream << "if (funcName == \"" #name "\") { " #name "Ptr = funcPtr; return; }\n";

#define AddEngineFunctionPointerToDll(name, funcDef, funcSignatureName) \
	s_Add##funcSignatureName(#name, funcDef);
}

namespace Kargono::Scripting
{
	// Initial definitions and static members for insertion functions (functions that insert engine pointers into the scripting dll)

	// Void return type
	DefineInsertFunction(VoidNone, void)
	DefineInsertFunction(VoidString, void, const std::string&)
	DefineInsertFunction(VoidUInt16, void, uint16_t)
	DefineInsertFunction(VoidUInt64, void, uint64_t)
	DefineInsertFunction(VoidUInt64UInt64, void, uint64_t, uint64_t)
	DefineInsertFunction(VoidVec3, void, Math::vec3)
	DefineInsertFunction(VoidUInt32UInt32, void, uint32_t, uint32_t)
	DefineInsertFunction(VoidVec3Float, void, Math::vec3, float)
	DefineInsertFunction(VoidVec3Vec3, void, Math::vec3, Math::vec3)
	DefineInsertFunction(VoidStringBool, void, const std::string&, bool)
	DefineInsertFunction(VoidUInt16Bool, void, uint16_t, bool)
	DefineInsertFunction(VoidUInt16UInt16, void, uint16_t, uint16_t)
	DefineInsertFunction(VoidUIWidgetString, void, RuntimeUI::WidgetID, const std::string&)
	DefineInsertFunction(VoidUIWindowBool, void, RuntimeUI::WindowID, bool)
	DefineInsertFunction(VoidUIWidgetBool, void, RuntimeUI::WidgetID, bool)
	DefineInsertFunction(VoidUIWidget, void, RuntimeUI::WidgetID)
	DefineInsertFunction(VoidUIWidgetVec4, void, RuntimeUI::WidgetID, Math::vec4)
	DefineInsertFunction(VoidUIWidgetUInt64, void, RuntimeUI::WidgetID, uint64_t)
	DefineInsertFunction(VoidUInt16UInt16String, void, uint16_t, uint16_t, const std::string&)
	DefineInsertFunction(VoidUInt16UInt16Bool, void, uint16_t, uint16_t, bool)
	DefineInsertFunction(VoidStringVoidPtr, void, const std::string&, void*)
	DefineInsertFunction(VoidStringString, void, const std::string&, const std::string&)
	DefineInsertFunction(VoidStringStringBool, void, const std::string&, const std::string&, bool)
	DefineInsertFunction(VoidStringStringString, void, const std::string&, const std::string&, const std::string&)
	DefineInsertFunction(VoidPtrUInt64UInt64UInt64, void*, uint64_t, uint64_t, uint64_t)
	DefineInsertFunction(VoidUInt64UInt64UInt64VoidPtr, void, uint64_t, uint64_t, uint64_t, void*)
	DefineInsertFunction(VoidUInt32UInt64UInt64Float, void, uint32_t, uint64_t, uint64_t, float)
	DefineInsertFunction(VoidStringStringVec4, void, const std::string&, const std::string&, Math::vec4)
	DefineInsertFunction(VoidUInt16UInt16Vec4, void, uint16_t, uint16_t, Math::vec4)
	DefineInsertFunction(VoidUInt64StringVoidPtr, void, uint64_t, const std::string&, void*)
	DefineInsertFunction(VoidPtrString, void*, const std::string&)
	DefineInsertFunction(VoidPtrUInt64String, void*, uint64_t, const std::string&)
	DefineInsertFunction(VoidUInt64Vec2, void, uint64_t, Math::vec2)
	DefineInsertFunction(VoidUInt64Vec3, void, uint64_t, Math::vec3)
	DefineInsertFunction(VoidUInt64Vec3Vec2, void, uint64_t, Math::vec3, Math::vec2)
	// Bool return type
	DefineInsertFunction(BoolUIWidget, bool, RuntimeUI::WidgetID)
	DefineInsertFunction(BoolStringString, bool, const std::string&, const std::string&)
	DefineInsertFunction(BoolUInt64String, bool, uint64_t, const std::string&)
	DefineInsertFunction(BoolUInt64UInt64, bool, uint64_t, uint64_t)
	DefineInsertFunction(BoolUInt16UInt16, bool, uint16_t, uint16_t)
	DefineInsertFunction(BoolUInt16, bool, uint16_t)
	DefineInsertFunction(BoolUInt64, bool, uint64_t)
	DefineInsertFunction(BoolString, bool, const std::string&)
	// Integer return types
	DefineInsertFunction(UInt16None, uint16_t)
	DefineInsertFunction(Int32Int32Int32, int32_t, int32_t, int32_t)
	DefineInsertFunction(UInt64String, uint64_t, const std::string&)
	// Float return type
	DefineInsertFunction(FloatFloatFloat, float, float, float)
	// Vector return types
	DefineInsertFunction(Vec2UInt64, Math::vec2, uint64_t)
	DefineInsertFunction(Vec3UInt64, Math::vec3, uint64_t)
	// String return type
	DefineInsertFunction(StringUInt64, const std::string&, uint64_t)
	DefineInsertFunction(StringUIWidget, const std::string&, RuntimeUI::WidgetID)
	// Other return types
	DefineInsertFunction(RaycastResultVec2Vec2, Physics::RaycastResult, Math::vec2, Math::vec2)

	void ScriptModuleBuilder::CreateScriptModule()
	{
		// Release active script module so it is available to be written to...
		KG_INFO("Closing active script module...");
		ScriptService::CloseActiveScriptModule();

		// Load in ScriptRegistry if not already loaded
		if (Assets::AssetService::GetScriptRegistry().size() == 0)
		{
			KG_WARN("Loading script registry from disk since in-memory registry is empty");
			Assets::AssetService::DeserializeScriptRegistry();
		}

		// Load in project components if not already loaded
		if (Assets::AssetService::GetProjectComponentRegistry().size() == 0)
		{
			KG_WARN("Loading script registry from disk since in-memory registry is empty");
			Assets::AssetService::DeserializeProjectComponentRegistry();
		}

		// Load in ai states if not already loaded
		if (Assets::AssetService::GetAIStateRegistry().size() == 0)
		{
			KG_WARN("Loading script registry from disk since in-memory registry is empty");
			Assets::AssetService::DeserializeAIStateRegistry();
		}

		KG_INFO("Creating Script Module CPP Files...");
		CreateModuleHeaderFile();
		bool generateCPPSuccess = CreateModuleCPPFile();
		if (!generateCPPSuccess)
		{
			KG_WARN("Failure to generate C++ scripts from kgscripts");
			ScriptService::LoadActiveScriptModule();
			Assets::AssetService::DeserializeScriptRegistry();
			return;
		}
		
		KG_INFO("Clearing previous compilation logs...");
		Utility::FileSystem::DeleteSelectedFile("Log/BuildScriptLibraryDebug.log");
		KG_INFO("Compiling debug script module...");
#if defined(KG_PLATFORM_WINDOWS)
		bool buildSuccessful = CompileModuleCodeMSVC(true);
#elif defined(KG_PLATFORM_LINUX)
		bool buildSuccessful = CompileModuleCodeGCC(true);
#endif
		if (!buildSuccessful)
		{
			KG_WARN("Failure to compile script module");
			ScriptService::LoadActiveScriptModule();
			Assets::AssetService::DeserializeScriptRegistry();
			return;
		}
		KG_INFO("Clearing previous compilation logs...");
		Utility::FileSystem::DeleteSelectedFile("Log/BuildScriptLibrary.log");
		KG_INFO("Compiling release script module...");
#if defined(KG_PLATFORM_WINDOWS)
		buildSuccessful = CompileModuleCodeMSVC(false);
#elif defined(KG_PLATFORM_LINUX)
		buildSuccessful = CompileModuleCodeGCC(false);
#endif
		if (!buildSuccessful)
		{
			KG_WARN("Failed to compile release script module");
			ScriptService::LoadActiveScriptModule();
			Assets::AssetService::DeserializeScriptRegistry();
			return;
		}

		KG_INFO("Opening New Scripting Module...");
		ScriptService::LoadActiveScriptModule();

		// Revalidate in-memory script cache
		Assets::AssetService::DeserializeScriptRegistry();
		for (auto& [handle, scriptRef] : Assets::AssetService::GetScriptCache())
		{
			ScriptService::LoadScriptFunction(scriptRef, scriptRef->m_FuncType);
		}
		KG_INFO("Successfully build and loaded new script module");
	}
	void ScriptModuleBuilder::CreateModuleHeaderFile()
	{

		// Write out return value and function name
		std::stringstream outputStream {};
		outputStream << "#pragma once\n";
#if defined(KG_PLATFORM_WINDOWS)
		outputStream << "#ifdef KARGONO_EXPORTS\n";
		outputStream << "#define KARGONO_API __declspec(dllexport)\n";
		outputStream << "#else\n";
		outputStream << "#define KARGONO_API __declspec(dllimport)\n";
		outputStream << "#endif\n";
#elif defined(KG_PLATFORM_LINUX)
		outputStream << "#ifdef KARGONO_EXPORTS\n";
		outputStream << "#define KARGONO_API __attribute__((visibility(\"default\")))\n";
		outputStream << "#else\n";
		outputStream << "#define KARGONO_API\n";
		outputStream << "#endif\n";
#endif

		outputStream << "#include <functional>\n";
		outputStream << "#include <string>\n";
		outputStream << "#include <sstream>\n";
		outputStream << "#include <limits>\n";
		outputStream << "#include \"" << "Kargono/Math/MathAliases.h" << "\"\n"; // Include Math Library
		outputStream << "#include \"" << "Kargono/RuntimeUI/RuntimeUICommon.h" << "\"\n"; // Include Runtime UI Common
		outputStream << "#include \"" << "Kargono/Physics/Physics2DCommon.h" << "\"\n"; // Include Physics Common

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

		// Void return type
		AddImportFunctionToHeaderFile(VoidNone, void)
		AddImportFunctionToHeaderFile(VoidString, void, const std::string&)
		AddImportFunctionToHeaderFile(VoidUInt16, void, uint16_t)
		AddImportFunctionToHeaderFile(VoidUInt64, void, uint64_t)
		AddImportFunctionToHeaderFile(VoidUInt64UInt64, void, uint64_t, uint64_t)
		AddImportFunctionToHeaderFile(VoidVec3, void, Math::vec3)
		AddImportFunctionToHeaderFile(VoidUInt32UInt32, void, uint32_t, uint32_t)
		AddImportFunctionToHeaderFile(VoidVec3Float, void, Math::vec3, float)
		AddImportFunctionToHeaderFile(VoidVec3Vec3, void, Math::vec3, Math::vec3)
		AddImportFunctionToHeaderFile(VoidStringBool, void, const std::string&, bool)
		AddImportFunctionToHeaderFile(VoidUInt16Bool, void, uint16_t, bool)
		AddImportFunctionToHeaderFile(VoidStringVoidPtr, void, const std::string&, void*)
		AddImportFunctionToHeaderFile(VoidStringString, void, const std::string&, const std::string&)
		AddImportFunctionToHeaderFile(VoidStringStringBool, void, const std::string&, const std::string&, bool)
		AddImportFunctionToHeaderFile(VoidStringStringString, void, const std::string&, const std::string&, const std::string&)
		AddImportFunctionToHeaderFile(VoidStringStringVec4, void, const std::string&, const std::string&, Math::vec4)
		AddImportFunctionToHeaderFile(VoidUInt16UInt16Vec4, void, uint16_t, uint16_t, Math::vec4)
		AddImportFunctionToHeaderFile(VoidUInt16UInt16Bool, void, uint16_t, uint16_t, bool)
		AddImportFunctionToHeaderFile(VoidUIWidgetString, void, RuntimeUI::WidgetID, const std::string&)
		AddImportFunctionToHeaderFile(VoidUInt64StringVoidPtr, void, uint64_t, const std::string&, void*)
		AddImportFunctionToHeaderFile(VoidPtrUInt64UInt64UInt64, void*, uint64_t, uint64_t, uint64_t)
		AddImportFunctionToHeaderFile(VoidUInt64UInt64UInt64VoidPtr, void, uint64_t, uint64_t, uint64_t, void*)
		AddImportFunctionToHeaderFile(VoidUInt32UInt64UInt64Float, void, uint32_t, uint64_t, uint64_t, float)
		AddImportFunctionToHeaderFile(VoidUInt16UInt16, void, uint16_t, uint16_t)
		AddImportFunctionToHeaderFile(VoidUInt16UInt16String, void, uint16_t, uint16_t, const std::string&)
		AddImportFunctionToHeaderFile(VoidPtrString, void*, const std::string&)
		AddImportFunctionToHeaderFile(VoidPtrUInt64String, void*, uint64_t, const std::string&)
		AddImportFunctionToHeaderFile(VoidUInt64Vec3, void, uint64_t, Math::vec3)
		AddImportFunctionToHeaderFile(VoidUInt64Vec2, void, uint64_t, Math::vec2)
		AddImportFunctionToHeaderFile(VoidUInt64Vec3Vec2, void, uint64_t, Math::vec3, Math::vec2)
		AddImportFunctionToHeaderFile(VoidUIWidgetString, void, RuntimeUI::WidgetID, const std::string&)
		AddImportFunctionToHeaderFile(VoidUIWindowBool, void, RuntimeUI::WindowID, bool)
		AddImportFunctionToHeaderFile(VoidUIWidgetBool, void, RuntimeUI::WidgetID, bool)
		AddImportFunctionToHeaderFile(VoidUIWidget, void, RuntimeUI::WidgetID)
		AddImportFunctionToHeaderFile(VoidUIWidgetVec4, void, RuntimeUI::WidgetID, Math::vec4)
		AddImportFunctionToHeaderFile(VoidUIWidgetUInt64, void, RuntimeUI::WidgetID, uint64_t)
		// Bool return type
		AddImportFunctionToHeaderFile(BoolUIWidget, bool, RuntimeUI::WidgetID)
		AddImportFunctionToHeaderFile(BoolStringString, bool, const std::string&, const std::string&)
		AddImportFunctionToHeaderFile(BoolUInt64String, bool, uint64_t, const std::string&)
		AddImportFunctionToHeaderFile(BoolUInt64UInt64, bool, uint64_t, uint64_t)
		AddImportFunctionToHeaderFile(BoolUInt16, bool, uint16_t)
		AddImportFunctionToHeaderFile(BoolUInt16UInt16, bool, uint16_t, uint16_t)
		AddImportFunctionToHeaderFile(BoolUInt64, bool, uint64_t)
		AddImportFunctionToHeaderFile(BoolString, bool, const std::string&)
		// Integer return types
		AddImportFunctionToHeaderFile(UInt16None, uint16_t)
		AddImportFunctionToHeaderFile(UInt64String, uint64_t, const std::string&)
		AddImportFunctionToHeaderFile(Int32Int32Int32, int32_t, int32_t, int32_t)
		// Float return type
		AddImportFunctionToHeaderFile(FloatFloatFloat, float, float, float)
		// Vector return types
		AddImportFunctionToHeaderFile(Vec2UInt64, Math::vec2, uint64_t)
		AddImportFunctionToHeaderFile(Vec3UInt64, Math::vec3, uint64_t)
		// String return type
		AddImportFunctionToHeaderFile(StringUInt64, const std::string&, uint64_t)
		AddImportFunctionToHeaderFile(StringUIWidget, const std::string&, RuntimeUI::WidgetID)
		// Other return types
		AddImportFunctionToHeaderFile(RaycastResultVec2Vec2, Physics::RaycastResult, Math::vec2, Math::vec2)

		// Add Script Function Declarations
		for (auto& [handle, asset] : Assets::AssetService::GetScriptRegistry())
		{
			Ref<Script> script = Assets::AssetService::GetScript(handle);
			WrappedVarType returnValue;
			std::vector<WrappedVarType> parameterTypes;
			std::vector<FixedString32> parameterNames;

			// Load return value and parameterTypes differently if using an arbitrary function
			if (script->m_FuncType == WrappedFuncType::ArbitraryFunction)
			{
				returnValue = script->m_ExplicitFuncType.m_ReturnType;
				parameterTypes = script->m_ExplicitFuncType.m_ParameterTypes;
				parameterNames = script->m_ExplicitFuncType.m_ParameterNames;
			}
			else
			{
				returnValue = Utility::WrappedFuncTypeToReturnType(script->m_FuncType);
				parameterTypes = Utility::WrappedFuncTypeToParameterTypes(script->m_FuncType);

				// Provide default names for parameterTypes
				char letterIteration{ '1' };
				for (size_t iteration{ 0 }; iteration < parameterTypes.size(); iteration++)
				{
					parameterNames.emplace_back((std::string("parameter") + letterIteration).c_str());
					letterIteration++;
				}
			}

			outputStream << "\t\tKARGONO_API ";
			outputStream << Utility::WrappedVarTypeToCPPString(returnValue) << " " << script->m_ScriptName << "(";

			// Write out parameterTypes into function signature
			for (size_t iteration{ 0 }; iteration < parameterTypes.size(); iteration++)
			{
				outputStream << Utility::WrappedVarTypeToCPPParameter(parameterTypes.at(iteration)) << " " << parameterNames.at(iteration);
				if (iteration != parameterTypes.size() - 1)
				{
					outputStream << ',';
				}
			}

			outputStream << ");" << "\n";
		}

		outputStream << "\t}" << "\n";
		outputStream << "}" << "\n";

		std::filesystem::path headerFile = { Projects::ProjectService::GetActiveIntermediateDirectory() / "Script/ExportHeader.h" };

		std::string outputString = outputStream.str();
		Utility::Operations::RemoveCharacterFromString(outputString, '\r');

		Utility::FileSystem::WriteFileString(headerFile, outputString);
	}

	bool ScriptModuleBuilder::CreateModuleCPPFile()
	{
		std::stringstream outputStream {};
		outputStream << "#include \"ExportHeader.h\"\n";
		outputStream << "namespace Kargono\n";
		outputStream << "{\n";

		// Insert Callable Function Definitions into CPP file

		// Application
		AddEngineFunctionToCPPFileNoParameters(Application_Close, void)
		AddEngineFunctionToCPPFileOneParameters(Application_Resize, void, uint16_t)
		// Artificial Intelligence
		AddEngineFunctionToCPPFileOneParameters(AI_RevertPreviousState, void, uint64_t)
		AddEngineFunctionToCPPFileOneParameters(AI_ClearGlobalState, void, uint64_t)
		AddEngineFunctionToCPPFileOneParameters(AI_ClearCurrentState, void, uint64_t)
		AddEngineFunctionToCPPFileOneParameters(AI_ClearPreviousState, void, uint64_t)
		AddEngineFunctionToCPPFileOneParameters(AI_ClearAllStates, void, uint64_t)
		AddEngineFunctionToCPPFileTwoParameters(AI_ChangeGlobalState, void, uint64_t, uint64_t)
		AddEngineFunctionToCPPFileTwoParameters(AI_ChangeCurrentState, void, uint64_t, uint64_t)
		AddEngineFunctionToCPPFileTwoParameters(AI_IsGlobalState, bool, uint64_t, uint64_t)
		AddEngineFunctionToCPPFileTwoParameters(AI_IsCurrentState, bool, uint64_t, uint64_t)
		AddEngineFunctionToCPPFileTwoParameters(AI_IsPreviousState, bool, uint64_t, uint64_t)
		AddEngineFunctionToCPPFileFourParameters(AI_SendMessage, void, uint32_t, uint64_t, uint64_t, float)
		// Audio
		AddEngineFunctionToCPPFileOneParameters(PlaySoundFromHandle, void, uint64_t)
		AddEngineFunctionToCPPFileOneParameters(PlayStereoSoundFromHandle, void, uint64_t)
		// Debug
		AddEngineFunctionToCPPFileNoParameters(ClearDebugLines, void)
		AddEngineFunctionToCPPFileNoParameters(ClearDebugPoints, void)
		AddEngineFunctionToCPPFileOneParameters(AddDebugPoint, void, Math::vec3)
		AddEngineFunctionToCPPFileTwoParameters(AddDebugLine, void, Math::vec3, Math::vec3)
		AddEngineFunctionToCPPFileThreeParameters(Log, void, const std::string&, const std::string&, const std::string&)
		// Game State
		AddEngineFunctionToCPPFileOneParameters(GetGameStateField, void*, const std::string&)
		AddEngineFunctionToCPPFileTwoParameters(SetGameStateField, void, const std::string&, void*)
		// Input
		AddEngineFunctionToCPPFileOneParameters(Input_IsKeyPressed, bool, uint16_t)
		AddEngineFunctionToCPPFileOneParameters(InputMap_LoadInputMapFromHandle, void, uint64_t)
		AddEngineFunctionToCPPFileOneParameters(InputMap_IsPollingSlotPressed, bool, uint16_t)
		// Network
		AddEngineFunctionToCPPFileNoParameters(EnableReadyCheck, void)
		AddEngineFunctionToCPPFileNoParameters(RequestUserCount, void)
		AddEngineFunctionToCPPFileNoParameters(RequestJoinSession, void)
		AddEngineFunctionToCPPFileNoParameters(LeaveCurrentSession, void)
		AddEngineFunctionToCPPFileNoParameters(GetActiveSessionSlot, uint16_t)
		AddEngineFunctionToCPPFileOneParameters(SignalAll, void, uint16_t)
		AddEngineFunctionToCPPFileThreeParameters(SendAllEntityPhysics, void, uint64_t, Math::vec3, Math::vec2)
		// Particles
		AddEngineFunctionToCPPFileTwoParameters(Particles_AddEmitterByHandle, void, uint64_t, Math::vec3)
		// Physics
		AddEngineFunctionToCPPFileTwoParameters(Physics_Raycast, Physics::RaycastResult, Math::vec2, Math::vec2)
		// Random
		AddEngineFunctionToCPPFileTwoParameters(GenerateRandomInteger, int32_t, int32_t, int32_t)
		AddEngineFunctionToCPPFileTwoParameters(GenerateRandomFloat, float, float, float)
		// Scenes
		AddEngineFunctionToCPPFileOneParameters(Scenes_IsSceneActive, bool, uint64_t)
		AddEngineFunctionToCPPFileOneParameters(TransitionSceneFromHandle, void, uint64_t)
		AddEngineFunctionToCPPFileOneParameters(TagComponent_GetTag, const std::string&, uint64_t)
		AddEngineFunctionToCPPFileOneParameters(TransformComponent_GetTranslation, Math::vec3, uint64_t)
		AddEngineFunctionToCPPFileOneParameters(Rigidbody2DComponent_GetLinearVelocity, Math::vec2, uint64_t)
		AddEngineFunctionToCPPFileOneParameters(FindEntityHandleByName, uint64_t, const std::string&)
		AddEngineFunctionToCPPFileTwoParameters(CheckHasComponent, bool, uint64_t, const std::string&)
		AddEngineFunctionToCPPFileTwoParameters(SendAllEntityLocation, void, uint64_t, Math::vec3)
		AddEngineFunctionToCPPFileTwoParameters(Rigidbody2DComponent_SetLinearVelocity, void, uint64_t, Math::vec2)
		AddEngineFunctionToCPPFileTwoParameters(TransformComponent_SetTranslation, void, uint64_t, Math::vec3)
		AddEngineFunctionToCPPFileThreeParameters(Scenes_GetProjectComponentField, void*, uint64_t, uint64_t, uint64_t)
		AddEngineFunctionToCPPFileFourParameters(Scenes_SetProjectComponentField, void, uint64_t, uint64_t, uint64_t, void*)
		// User Interface
		AddEngineFunctionToCPPFileNoParameters(RuntimeUI_ClearSelectedWidget, void)
		AddEngineFunctionToCPPFileOneParameters(RuntimeUI_IsUserInterfaceActiveFromHandle, bool, uint64_t)
		AddEngineFunctionToCPPFileOneParameters(RuntimeUI_LoadUserInterfaceFromHandle, void, uint64_t)
		AddEngineFunctionToCPPFileOneParameters(RuntimeUI_GetWidgetText, const std::string&, RuntimeUI::WidgetID
		)
		AddEngineFunctionToCPPFileOneParameters(RuntimeUI_SetSelectedWidget, void, RuntimeUI::WidgetID)
		AddEngineFunctionToCPPFileOneParameters(RuntimeUI_IsWidgetSelected, bool, RuntimeUI::WidgetID)
		AddEngineFunctionToCPPFileTwoParameters(RuntimeUI_SetDisplayWindow, void, RuntimeUI::WindowID, bool)
		AddEngineFunctionToCPPFileTwoParameters(RuntimeUI_SetWidgetSelectable, void, RuntimeUI::WidgetID, bool)
		AddEngineFunctionToCPPFileTwoParameters(RuntimeUI_SetWidgetText, void, RuntimeUI::WidgetID, const std::string&)
		AddEngineFunctionToCPPFileTwoParameters(RuntimeUI_SetWidgetBackgroundColor, void, RuntimeUI::WidgetID, Math::vec4)
		AddEngineFunctionToCPPFileTwoParameters(RuntimeUI_SetWidgetTextColor, void, RuntimeUI::WidgetID, Math::vec4)
		AddEngineFunctionToCPPFileTwoParameters(RuntimeUI_SetWidgetImage, void, RuntimeUI::WidgetID, uint64_t)


		// Insert FuncPointer Importing for DLL processing
		AddImportFunctionToCPPFile(VoidNone, void)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(EnableReadyCheck)
		AddEngineFunctionToCPPFileEnd(RequestUserCount)
		AddEngineFunctionToCPPFileEnd(RequestJoinSession)
		AddEngineFunctionToCPPFileEnd(LeaveCurrentSession)
		AddEngineFunctionToCPPFileEnd(ClearDebugLines)
		AddEngineFunctionToCPPFileEnd(ClearDebugPoints)
		AddEngineFunctionToCPPFileEnd(Application_Close)
		AddEngineFunctionToCPPFileEnd(RuntimeUI_ClearSelectedWidget)
		outputStream << "}\n";
		AddImportFunctionToCPPFile(VoidUInt16, void, uint16_t)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(SignalAll)
		AddEngineFunctionToCPPFileEnd(Application_Resize)
		outputStream << "}\n";
		AddImportFunctionToCPPFile(VoidVec3, void, Math::vec3)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(AddDebugPoint)
		outputStream << "}\n";
		AddImportFunctionToCPPFile(VoidVec3Float, void, Math::vec3, float)
		outputStream << "{\n";
		outputStream << "}\n";
		AddImportFunctionToCPPFile(VoidVec3Vec3, void, Math::vec3, Math::vec3)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(AddDebugLine)
		outputStream << "}\n";
		AddImportFunctionToCPPFile(VoidString, void, const std::string&)
		outputStream << "{\n";
		outputStream << "}\n";
		AddImportFunctionToCPPFile(VoidUInt64, void, uint64_t)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(AI_RevertPreviousState)
		AddEngineFunctionToCPPFileEnd(AI_ClearGlobalState)
		AddEngineFunctionToCPPFileEnd(AI_ClearCurrentState)
		AddEngineFunctionToCPPFileEnd(AI_ClearPreviousState)
		AddEngineFunctionToCPPFileEnd(AI_ClearAllStates)
		AddEngineFunctionToCPPFileEnd(TransitionSceneFromHandle)
		AddEngineFunctionToCPPFileEnd(RuntimeUI_LoadUserInterfaceFromHandle)
		AddEngineFunctionToCPPFileEnd(PlaySoundFromHandle)
		AddEngineFunctionToCPPFileEnd(PlayStereoSoundFromHandle)
		AddEngineFunctionToCPPFileEnd(InputMap_LoadInputMapFromHandle)
		outputStream << "}\n";
		AddImportFunctionToCPPFile(VoidUInt64UInt64, void, uint64_t, uint64_t)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(AI_ChangeGlobalState)
		AddEngineFunctionToCPPFileEnd(AI_ChangeCurrentState)
		outputStream << "}\n";
		AddImportFunctionToCPPFile(VoidStringBool, void, const std::string&, bool)
		outputStream << "{\n";
		outputStream << "}\n";
		AddImportFunctionToCPPFile(VoidUInt16Bool, void, uint16_t, bool)
		outputStream << "{\n";
		outputStream << "}\n";
		AddImportFunctionToCPPFile(VoidUIWindowBool, void, RuntimeUI::WindowID, bool)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(RuntimeUI_SetDisplayWindow)
		outputStream << "}\n";
		AddImportFunctionToCPPFile(VoidUInt16UInt16, void, uint16_t, uint16_t)
		outputStream << "{\n";
		outputStream << "}\n";
		AddImportFunctionToCPPFile(VoidUIWidget, void, RuntimeUI::WidgetID)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(RuntimeUI_SetSelectedWidget)
		outputStream << "}\n";
		AddImportFunctionToCPPFile(VoidUInt16UInt16Bool, void, uint16_t, uint16_t, bool)
		outputStream << "{\n";
		outputStream << "}\n";
		AddImportFunctionToCPPFile(VoidUIWidgetBool, void, RuntimeUI::WidgetID, bool)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(RuntimeUI_SetWidgetSelectable)
		outputStream << "}\n";
		AddImportFunctionToCPPFile(VoidStringVoidPtr, void, const std::string&, void*)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(SetGameStateField)
		outputStream << "}\n";
		AddImportFunctionToCPPFile(VoidStringString, void, const std::string&, const std::string&)
		outputStream << "{\n";
		outputStream << "}\n";
		AddImportFunctionToCPPFile(VoidStringStringBool, void, const std::string&, const std::string&, bool)
		outputStream << "{\n";
		outputStream << "}\n";
		AddImportFunctionToCPPFile(VoidStringStringString, void, const std::string&, const std::string&, const std::string&)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(Log)
		outputStream << "}\n";
		AddImportFunctionToCPPFile(VoidUInt16UInt16String, void, uint16_t, uint16_t, const std::string&)
		outputStream << "{\n";
		outputStream << "}\n";
		AddImportFunctionToCPPFile(VoidUIWidgetString, void, RuntimeUI::WidgetID, const std::string&)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(RuntimeUI_SetWidgetText)
		outputStream << "}\n";
		AddImportFunctionToCPPFile(VoidUIWidgetUInt64, void, RuntimeUI::WidgetID, uint64_t)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(RuntimeUI_SetWidgetImage)
		outputStream << "}\n";
		AddImportFunctionToCPPFile(VoidStringStringVec4, void, const std::string&, const std::string&, Math::vec4)
		outputStream << "{\n";
		outputStream << "}\n";
		AddImportFunctionToCPPFile(VoidUInt64Vec3, void, uint64_t, Math::vec3)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(SendAllEntityLocation)
		AddEngineFunctionToCPPFileEnd(TransformComponent_SetTranslation)
		AddEngineFunctionToCPPFileEnd(Particles_AddEmitterByHandle)
		outputStream << "}\n";
		AddImportFunctionToCPPFile(VoidUInt64Vec2, void, uint64_t, Math::vec2)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(Rigidbody2DComponent_SetLinearVelocity)
		outputStream << "}\n";
		AddImportFunctionToCPPFile(BoolUInt64String, bool, uint64_t, const std::string&)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(CheckHasComponent)
		outputStream << "}\n";
		AddImportFunctionToCPPFile(BoolStringString, bool, const std::string&, const std::string&)
		outputStream << "{\n";
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
		outputStream << "}\n";
		AddImportFunctionToCPPFile(VoidUInt64Vec3Vec2, void, uint64_t, Math::vec3, Math::vec2)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(SendAllEntityPhysics)
		outputStream << "}\n";
		AddImportFunctionToCPPFile(VoidUInt16UInt16Vec4, void, uint16_t, uint16_t, Math::vec4)
		outputStream << "{\n";
		outputStream << "}\n";
		AddImportFunctionToCPPFile(VoidUIWidgetVec4, void, RuntimeUI::WidgetID, Math::vec4)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(RuntimeUI_SetWidgetTextColor);
		AddEngineFunctionToCPPFileEnd(RuntimeUI_SetWidgetBackgroundColor);
		outputStream << "}\n";
		AddImportFunctionToCPPFile(VoidPtrUInt64String, void*, uint64_t, const std::string&)
		outputStream << "{\n";
		outputStream << "}\n";
		AddImportFunctionToCPPFile(VoidUInt32UInt32, void, uint32_t, uint32_t)
		outputStream << "{\n";
		outputStream << "}\n";
		AddImportFunctionToCPPFile(VoidPtrString, void*, const std::string&)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(GetGameStateField)
		outputStream << "}\n";
		AddImportFunctionToCPPFile(BoolUInt64UInt64, bool, uint64_t, uint64_t)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(AI_IsGlobalState)
		AddEngineFunctionToCPPFileEnd(AI_IsCurrentState)
		AddEngineFunctionToCPPFileEnd(AI_IsPreviousState)
		outputStream << "}\n";
		AddImportFunctionToCPPFile(Int32Int32Int32, int32_t, int32_t, int32_t)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(GenerateRandomInteger)
		outputStream << "}\n";
		AddImportFunctionToCPPFile(FloatFloatFloat, float, float, float)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(GenerateRandomFloat)
		outputStream << "}\n";
		AddImportFunctionToCPPFile(BoolUInt16, bool, uint16_t)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(Input_IsKeyPressed)
		AddEngineFunctionToCPPFileEnd(InputMap_IsPollingSlotPressed)
		outputStream << "}\n";
		AddImportFunctionToCPPFile(BoolString, bool, const std::string&)
		outputStream << "{\n";
		outputStream << "}\n";
		AddImportFunctionToCPPFile(BoolUInt64, bool, uint64_t)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(Scenes_IsSceneActive)
		AddEngineFunctionToCPPFileEnd(RuntimeUI_IsUserInterfaceActiveFromHandle)
		outputStream << "}\n";
		AddImportFunctionToCPPFile(BoolUInt16UInt16, bool, uint16_t, uint16_t)
		outputStream << "{\n";
		outputStream << "}\n";
		AddImportFunctionToCPPFile(BoolUIWidget, bool, RuntimeUI::WidgetID)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(RuntimeUI_IsWidgetSelected)
		outputStream << "}\n";
		AddImportFunctionToCPPFile(VoidPtrUInt64UInt64UInt64, void*, uint64_t, uint64_t, uint64_t)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(Scenes_GetProjectComponentField)
		outputStream << "}\n";
		AddImportFunctionToCPPFile(VoidUInt64UInt64UInt64VoidPtr, void, uint64_t, uint64_t, uint64_t, void*)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(Scenes_SetProjectComponentField)
		outputStream << "}\n";
		AddImportFunctionToCPPFile(VoidUInt32UInt64UInt64Float, void, uint32_t, uint64_t, uint64_t, float)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(AI_SendMessage)
		outputStream << "}\n";
		AddImportFunctionToCPPFile(RaycastResultVec2Vec2, Physics::RaycastResult, Math::vec2, Math::vec2)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(Physics_Raycast)
		outputStream << "}\n";
		AddImportFunctionToCPPFile(StringUInt64, const std::string&, uint64_t)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(TagComponent_GetTag)
		outputStream << "}\n";
		AddImportFunctionToCPPFile(StringUIWidget, const std::string&, RuntimeUI::WidgetID)
		outputStream << "{\n";
		AddEngineFunctionToCPPFileEnd(RuntimeUI_GetWidgetText)
		outputStream << "}\n";

		// Write scripts into a single cpp file
		bool compilationSuccess{ true };
		for (auto& [handle, asset] : Assets::AssetService::GetScriptRegistry())
		{
			if (asset.Data.GetSpecificMetaData<Assets::ScriptMetaData>()->m_ScriptType == ScriptType::Engine)
			{
				continue;
			}
			std::string compiledScript = ScriptCompilerService::CompileScriptFile(Projects::ProjectService::GetActiveAssetDirectory() / asset.Data.FileLocation);
			if (compiledScript.empty())
			{
				KG_WARN("Failed to compile the script at: {}", asset.Data.FileLocation.string());
				compilationSuccess = false;
			}
			outputStream << compiledScript;
			outputStream << '\n';
		}
		outputStream << "}\n";

		if (!compilationSuccess)
		{
			return false;
		}

		std::filesystem::path file = { Projects::ProjectService::GetActiveIntermediateDirectory() / "Script/ExportBody.cpp" };

		std::string outputString = outputStream.str();
		Utility::Operations::RemoveCharacterFromString(outputString, '\r');

		Utility::FileSystem::WriteFileString(file, outputString);
		return true;
	}

	bool ScriptModuleBuilder::CompileModuleCodeMSVC(bool createDebug)
	{
		Utility::FileSystem::CreateNewDirectory(Projects::ProjectService::GetActiveIntermediateDirectory() / "Script/");
		std::filesystem::path binaryPath { Projects::ProjectService::GetActiveIntermediateDirectory() / "Script/" };
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
		std::filesystem::path sourcePath { Projects::ProjectService::GetActiveIntermediateDirectory() / "Script/ExportBody.cpp" };

		std::stringstream outputStream {};
		outputStream << "("; // Parentheses to group all function calls together
		// Access visual studio toolset console
		outputStream << "\"C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Auxiliary\\Build\\vcvars64.bat\"";

		// Start Next Command
		//outputStream << " > Log\\ScriptCompilation.log 2>&1 "; // Sends error/info to log file
		outputStream << " && "; // Combine commands

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
		outputStream << "/Fo" << "\"" << binaryPath.string() << "\"" << ' '; // Define Intermediate Location
		outputStream << "\"" << sourcePath.string() << "\"" << " "; // Compile scripting source file (ExportBody.cpp)

		// Start Next Command
		//outputStream << " > Log\\ScriptCompilation.log 2>&1 "; // Sends error/info to log file
		outputStream << " && "; // Combine commands

		// Start Linking Stage
		outputStream << "link "; // Start link command
		outputStream << "/DLL "; // Specify output as a shared library
		outputStream << "/ignore:4099 "; // Ignores warning about missing pbd files for .obj files (I generate the symbols inside of the .obj file with /Z7 flag) 
		if (createDebug)
		{
			outputStream << "/DEBUG "; // Specifies output as debug files
			outputStream << "/PDB:" << "\"" << debugSymbolsPath.string() << "\"" << " "; // Specify .pdb file location/name
		}
		outputStream << "/OUT:" << "\"" << binaryFile.string() << "\"" << " "; // Specify output directory
		outputStream << "\"" << objectPath.string() << "\""; // Object File to Link

		outputStream << ")"; // Parentheses to group all function calls together

		// Sends all three calls (open dev console, compiler, and linker) error/info to log file
		if (createDebug)
		{
			outputStream << " >> Log\\BuildScriptLibraryDebug.log 2>&1 ";
		}
		else
		{
			outputStream << " >> Log\\BuildScriptLibrary.log 2>&1 ";
		}

		// Call Command
		return system(outputStream.str().c_str()) == 0;
	}

	bool ScriptModuleBuilder::CompileModuleCodeGCC(bool createDebug)
	{
		// Set up paths and files
		Utility::FileSystem::CreateNewDirectory(Projects::ProjectService::GetActiveIntermediateDirectory() / "Script/");
		std::filesystem::path binaryPath = Projects::ProjectService::GetActiveIntermediateDirectory() / "Script/";
		std::filesystem::path binaryFile;
		std::filesystem::path objectPath;
		
		if (createDebug) 
		{
			binaryFile = binaryPath / "ExportBodyDebug.so";  // Using .so for shared libraries on Linux
			objectPath = binaryPath / "ExportBody.o";       // Object file with .o extension
		} 
		else 
		{
			binaryFile = binaryPath / "ExportBody.so";
			objectPath = binaryPath / "ExportBody.o";
		}

		std::filesystem::path sourcePath = Projects::ProjectService::GetActiveIntermediateDirectory() / "Script/ExportBody.cpp";

		// Set up the output stream for the commands
		std::stringstream outputStream;
		outputStream << "("; // Parentheses to group all function calls together

		// Start GCC Command
		outputStream << "g++ ";  // GCC compiler command
		outputStream << "-c ";   // Tell GCC to only compile the code (not link)
		outputStream << "-fPIC "; // Ensure position independent code for 64bit arch
		if (createDebug) 
		{
			outputStream << "-g ";  // Enable debug symbols in the object file
		}
		outputStream << "-std=c++20 "; // Specify language version (C++20)
		outputStream << "-I../Dependencies/glm "; // Include GLM headers
		outputStream << "-I../Engine/Source ";  // Include Kargono headers
		outputStream << "-fexceptions ";  // Handle exceptions
		outputStream << "-D KARGONO_EXPORTS ";  // Define macros for exporting DLL functions

		// Output the object file path
		outputStream << "-o \"" << objectPath.string() << "\" ";  // Object file path
		outputStream << "\"" << sourcePath.string() << "\" "; // Compile the source file

		// Start Linking Stage
		outputStream << "&& ";  // Combine commands

		outputStream << "g++ ";  // GCC linker command
		outputStream << "-shared ";  // Create a shared library (.so)
		outputStream << "-fPIC "; // Ensure position independent code for 64bit arch
		if (createDebug) 
		{
			outputStream << "-g ";  // Debug information for shared library
			outputStream << "-Wl,-Map=" << binaryPath / "ExportBody.map" << " "; // Map file for debugging
		}
		outputStream << "-o \"" << binaryFile.string() << "\" "; // Output shared library file path
		outputStream << "\"" << objectPath.string() << "\" "; // Object file to link

		outputStream << ")";  // Parentheses to group the commands

		// Redirect logs if debugging
		if (createDebug) 
		{
			outputStream << " >> Log/BuildScriptLibraryDebug.log 2>&1 ";
		} 
		else 
		{
			outputStream << " >> Log/BuildScriptLibrary.log 2>&1 ";
		}

		// Call system command
		return system(outputStream.str().c_str()) == 0;
	}


	void ScriptModuleBuilder::AttachEngineFunctionsToModule()
	{
		// Void return type
		ImportInsertFunction(VoidNone)
		ImportInsertFunction(VoidUInt16)
		ImportInsertFunction(VoidUInt64)
		ImportInsertFunction(VoidString)
		ImportInsertFunction(VoidVec3)
		ImportInsertFunction(VoidUInt32UInt32)
		ImportInsertFunction(VoidVec3Float)
		ImportInsertFunction(VoidVec3Vec3)
		ImportInsertFunction(VoidPtrString)
		ImportInsertFunction(VoidStringBool)
		ImportInsertFunction(VoidUInt16Bool)
		ImportInsertFunction(VoidUInt16UInt16)
		ImportInsertFunction(VoidUInt16UInt16String)
		ImportInsertFunction(VoidUInt16UInt16Bool)
		ImportInsertFunction(VoidStringVoidPtr)
		ImportInsertFunction(VoidStringString)
		ImportInsertFunction(VoidStringStringBool)
		ImportInsertFunction(VoidStringStringString)
		ImportInsertFunction(VoidStringStringVec4)
		ImportInsertFunction(VoidUInt64StringVoidPtr)
		ImportInsertFunction(VoidPtrUInt64UInt64UInt64)
		ImportInsertFunction(VoidUInt64UInt64UInt64VoidPtr)
		ImportInsertFunction(VoidUInt32UInt64UInt64Float)
		ImportInsertFunction(VoidUInt16UInt16Vec4)
		ImportInsertFunction(VoidPtrUInt64String)
		ImportInsertFunction(VoidUInt64Vec3)
		ImportInsertFunction(VoidUInt64Vec2)
		ImportInsertFunction(VoidUInt64UInt64)
		ImportInsertFunction(VoidUInt64Vec3Vec2)
		ImportInsertFunction(VoidUIWidgetString)
		ImportInsertFunction(VoidUIWindowBool)
		ImportInsertFunction(VoidUIWidgetBool)
		ImportInsertFunction(VoidUIWidget)
		ImportInsertFunction(VoidUIWidgetVec4)
		ImportInsertFunction(VoidUIWidgetUInt64)
		// Bool return type
		ImportInsertFunction(BoolUIWidget)
		ImportInsertFunction(BoolStringString)
		ImportInsertFunction(BoolUInt64String)
		ImportInsertFunction(BoolUInt64UInt64)
		ImportInsertFunction(BoolUInt16)
		ImportInsertFunction(BoolUInt16UInt16)
		ImportInsertFunction(BoolUInt64)
		ImportInsertFunction(BoolString)
		// Integer return types
		ImportInsertFunction(UInt16None)
		ImportInsertFunction(UInt64String)
		ImportInsertFunction(Int32Int32Int32)
		// Float return type
		ImportInsertFunction(FloatFloatFloat)
		// Vector return types
		ImportInsertFunction(Vec2UInt64)
		ImportInsertFunction(Vec3UInt64)
		// String return types
		ImportInsertFunction(StringUInt64)
		ImportInsertFunction(StringUIWidget)
		// Other return types
		ImportInsertFunction(RaycastResultVec2Vec2)

		// Application
		AddEngineFunctionPointerToDll(Application_Resize, ApplicationResize, VoidUInt16)
		AddEngineFunctionPointerToDll(Application_Close, EngineService::SubmitApplicationCloseEvent, VoidNone)
		// Artificial Intelligence
		AddEngineFunctionPointerToDll(AI_ChangeGlobalState, AI::AIService::ChangeGlobalState, VoidUInt64UInt64)
		AddEngineFunctionPointerToDll(AI_ChangeCurrentState, AI::AIService::ChangeCurrentState, VoidUInt64UInt64)
		AddEngineFunctionPointerToDll(AI_RevertPreviousState, AI::AIService::RevertPreviousState, VoidUInt64)
		AddEngineFunctionPointerToDll(AI_SendMessage, AI::AIService::SendAIMessage, VoidUInt32UInt64UInt64Float)
		AddEngineFunctionPointerToDll(AI_ClearGlobalState, AI::AIService::ClearGlobalState, VoidUInt64)
		AddEngineFunctionPointerToDll(AI_ClearCurrentState, AI::AIService::ClearCurrentState, VoidUInt64)
		AddEngineFunctionPointerToDll(AI_ClearPreviousState, AI::AIService::ClearPreviousState, VoidUInt64)
		AddEngineFunctionPointerToDll(AI_ClearAllStates, AI::AIService::ClearAllStates, VoidUInt64)
		AddEngineFunctionPointerToDll(AI_IsGlobalState, AI::AIService::IsGlobalState, BoolUInt64UInt64)
		AddEngineFunctionPointerToDll(AI_IsCurrentState, AI::AIService::IsCurrentState, BoolUInt64UInt64)
		AddEngineFunctionPointerToDll(AI_IsPreviousState, AI::AIService::IsPreviousState, BoolUInt64UInt64)
		// Audio
		AddEngineFunctionPointerToDll(PlaySoundFromHandle, Audio::AudioService::PlaySoundFromHandle, VoidUInt64)
		AddEngineFunctionPointerToDll(PlayStereoSoundFromHandle, Audio::AudioService::PlayStereoSoundFromHandle, VoidUInt64)
		AddEngineFunctionPointerToDll(SignalAll, Network::ClientService::SignalAll, VoidUInt16)
		// Debug
		AddEngineFunctionPointerToDll(Log, Scripting::Log, VoidStringStringString)
		AddEngineFunctionPointerToDll(ClearDebugLines, Scripting::ClearDebugLines, VoidNone)
		AddEngineFunctionPointerToDll(ClearDebugPoints, Scripting::ClearDebugPoints, VoidNone)
		AddEngineFunctionPointerToDll(AddDebugPoint, Scripting::AddDebugPoint, VoidVec3)
		AddEngineFunctionPointerToDll(AddDebugLine, Scripting::AddDebugLine, VoidVec3Vec3)
		// Game State
		AddEngineFunctionPointerToDll(SetGameStateField, Scenes::GameStateService::SetActiveGameStateField, VoidStringVoidPtr)
		AddEngineFunctionPointerToDll(GetGameStateField, Scenes::GameStateService::GetActiveGameStateField, VoidPtrString)
		// Input
		AddEngineFunctionPointerToDll(Input_IsKeyPressed, Input::InputService::IsKeyPressed, BoolUInt16)
		AddEngineFunctionPointerToDll(InputMap_LoadInputMapFromHandle, Input::InputMapService::SetActiveInputMapFromHandle, VoidUInt64)
		AddEngineFunctionPointerToDll(InputMap_IsPollingSlotPressed, Input::InputMapService::IsPollingSlotPressed, BoolUInt16)
		// Networking
		AddEngineFunctionPointerToDll(LeaveCurrentSession, Network::ClientService::LeaveCurrentSession, VoidNone)
		AddEngineFunctionPointerToDll(EnableReadyCheck, Network::ClientService::EnableReadyCheck, VoidNone)
		AddEngineFunctionPointerToDll(RequestJoinSession, Network::ClientService::RequestJoinSession, VoidNone)
		AddEngineFunctionPointerToDll(SendAllEntityPhysics, Network::ClientService::SendAllEntityPhysics, VoidUInt64Vec3Vec2)
		AddEngineFunctionPointerToDll(RequestUserCount, Network::ClientService::RequestUserCount, VoidNone)
		AddEngineFunctionPointerToDll(GetActiveSessionSlot, Network::ClientService::GetActiveSessionSlot, UInt16None)
		AddEngineFunctionPointerToDll(SendAllEntityLocation, Network::ClientService::SendAllEntityLocation, VoidUInt64Vec3)
		// Particles
		AddEngineFunctionPointerToDll(Particles_AddEmitterByHandle, Particles::ParticleService::AddEmitterByHandle, VoidUInt64Vec3)
		// Physics 2D
		AddEngineFunctionPointerToDll(Physics_Raycast, Physics::Physics2DService::Raycast, RaycastResultVec2Vec2)
		// Random
		AddEngineFunctionPointerToDll(GenerateRandomInteger, Utility::RandomService::GenerateRandomInteger, Int32Int32Int32)
		AddEngineFunctionPointerToDll(GenerateRandomFloat, Utility::RandomService::GenerateRandomFloat, FloatFloatFloat)
		// Runtime User Interface
		AddEngineFunctionPointerToDll(RuntimeUI_SetWidgetText, RuntimeUI::RuntimeUIService::SetActiveWidgetTextByIndex, VoidUIWidgetString)
		AddEngineFunctionPointerToDll(RuntimeUI_IsUserInterfaceActiveFromHandle, RuntimeUI::RuntimeUIService::IsUIActiveFromHandle, BoolUInt64)
		AddEngineFunctionPointerToDll(RuntimeUI_LoadUserInterfaceFromHandle, RuntimeUI::RuntimeUIService::SetActiveUIFromHandle, VoidUInt64)
		AddEngineFunctionPointerToDll(RuntimeUI_SetDisplayWindow, RuntimeUI::RuntimeUIService::SetDisplayWindowByIndex, VoidUIWindowBool)
		AddEngineFunctionPointerToDll(RuntimeUI_SetSelectedWidget, RuntimeUI::RuntimeUIService::SetSelectedWidgetByIndex, VoidUIWidget)
		AddEngineFunctionPointerToDll(RuntimeUI_ClearSelectedWidget, RuntimeUI::RuntimeUIService::ClearSelectedWidget, VoidNone)
		AddEngineFunctionPointerToDll(RuntimeUI_SetWidgetTextColor, RuntimeUI::RuntimeUIService::SetWidgetTextColorByIndex, VoidUIWidgetVec4)
		AddEngineFunctionPointerToDll(RuntimeUI_SetWidgetBackgroundColor, RuntimeUI::RuntimeUIService::SetWidgetBackgroundColorByIndex, VoidUIWidgetVec4)
		AddEngineFunctionPointerToDll(RuntimeUI_SetWidgetSelectable, RuntimeUI::RuntimeUIService::SetWidgetSelectableByIndex, VoidUIWidgetBool)
		AddEngineFunctionPointerToDll(RuntimeUI_IsWidgetSelected, RuntimeUI::RuntimeUIService::IsWidgetSelectedByIndex, BoolUIWidget)
		AddEngineFunctionPointerToDll(RuntimeUI_SetWidgetImage, RuntimeUI::RuntimeUIService::SetWidgetImageByIndex, VoidUIWidgetUInt64)
		AddEngineFunctionPointerToDll(RuntimeUI_GetWidgetText, RuntimeUI::RuntimeUIService::GetWidgetTextByIndex, StringUIWidget)
		// Scenes
		AddEngineFunctionPointerToDll(TransitionSceneFromHandle, Scenes::SceneService::TransitionSceneFromHandle, VoidUInt64)
		AddEngineFunctionPointerToDll(CheckHasComponent, Scenes::SceneService::CheckActiveHasComponent, BoolUInt64String)
		AddEngineFunctionPointerToDll(FindEntityHandleByName, Scenes::SceneService::FindEntityHandleByName, UInt64String)
		AddEngineFunctionPointerToDll(Scenes_IsSceneActive, Scenes::SceneService::IsSceneActive, BoolUInt64)
		AddEngineFunctionPointerToDll(TransformComponent_GetTranslation, Scenes::SceneService::TransformComponentGetTranslation, Vec3UInt64)
		AddEngineFunctionPointerToDll(TransformComponent_SetTranslation, Scenes::SceneService::TransformComponentSetTranslation, VoidUInt64Vec3)
		AddEngineFunctionPointerToDll(Rigidbody2DComponent_SetLinearVelocity, Scenes::SceneService::Rigidbody2DComponent_SetLinearVelocity, VoidUInt64Vec2)
		AddEngineFunctionPointerToDll(Rigidbody2DComponent_GetLinearVelocity, Scenes::SceneService::Rigidbody2DComponent_GetLinearVelocity, Vec2UInt64)
		AddEngineFunctionPointerToDll(Scenes_GetProjectComponentField, Scenes::SceneService::GetProjectComponentField, VoidPtrUInt64UInt64UInt64)
		AddEngineFunctionPointerToDll(Scenes_SetProjectComponentField, Scenes::SceneService::SetProjectComponentField, VoidUInt64UInt64UInt64VoidPtr)
		AddEngineFunctionPointerToDll(TagComponent_GetTag, Scenes::SceneService::TagComponentGetTag, StringUInt64)
		
	}
}
