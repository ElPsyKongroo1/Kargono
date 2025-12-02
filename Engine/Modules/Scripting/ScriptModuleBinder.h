#pragma once

#include "Modules/Assets/AssetsCommon.h"
#include "Kargono/Core/Base.h"
#include "Kargono/Core/UUID.h"
#include "Kargono/Core/WrappedData.h"
#include "Modules/Scripting/ScriptingCommon.h"
#include "Kargono/Utility/Random.h"

#include "Modules/Scripting/Platform/SharedLibrary.h"

#include <filesystem>
#include <functional>
#include <string>
#include <sstream>
#include <vector>

namespace Kargono::Scripting
{
	class Script;
	class ScriptModuleBinder;

	class ScriptModuleBuilder
	{
	private:
		//==============================
		// Constructor/Destructor
		//==============================
		ScriptModuleBuilder() = default;
		~ScriptModuleBuilder() = default;
	public:
		//==============================
		// Lifecycle Functions
		//==============================
		void Init(SharedLib* libReference);
	public:
		//==============================
		// Create Script Module
		//==============================
		void CreateScriptModule();
	private:
		//==============================
		// Generate Module Code
		//==============================
		void CreateModuleHeaderFile(const std::filesystem::path& outputDir);
		bool CreateModuleCPPFile(const std::filesystem::path& outputDir);
	private:
		//==============================
		// Compile Module
		//==============================
		bool CompileModuleCodeMSVC(const std::filesystem::path& outputDir, bool createDebug);
		bool CompileModuleCodeGCC(const std::filesystem::path& outputDir, bool createDebug);
	private:
		//==============================
		// Connect Engine To Module
		//==============================
		void AttachEngineFunctionsToModule();
	private:
		//==============================
		// Injected Dependencies
		//==============================
		SharedLib* i_LibReference;
	private:
		//==============================
		// Owning Class
		//==============================
		friend class ScriptModuleBinder;
	};

	class ScriptModuleBinder
	{
	public:
		//==============================
		// Constructor/Destructor
		//==============================
		ScriptModuleBinder() = default;
		~ScriptModuleBinder() = default;
	public:
		//==============================
		// Lifecycle Functions
		//==============================
		void Init();
		void Terminate();
	private:
		// Helper functions
		void GenerateEngineScripts(std::vector<Ref<Script>>& engineScripts);
	public:
		//==============================
		// Manage Active Script Module
		//==============================
		void LoadActiveScriptModule(const std::filesystem::path& outputDir);
		void CloseActiveScriptModule();
		//==============================
		// Manage Individual Scripts
		//==============================
		void LoadScriptFunction(Script* script, WrappedFuncType funcType);
		//==============================
		// Getters/Setters
		//==============================
		std::vector<Ref<Script>>& GetAllEngineScripts()
		{
			return m_AllEngineScripts;
		}
	public:
		//==============================
		// Public Fields
		//==============================
		ScriptModuleBuilder m_ScriptBuilder;
	private:
		//==============================
		// Internal Fields
		//==============================
		std::vector<Ref<Script>> m_AllEngineScripts;
		Utility::PseudoGenerator m_IDGenerator{ 0xc3bc4ead8efa4c3a };
		SharedLib m_ScriptLibrary{};
	};

	class ScriptBinderService // TODO: EWWWWW
	{
	public:
		//==============================
		// Getters/Setters
		//==============================
		static ScriptModuleBinder& GetActiveContext() { return s_AppScriptBinder;}
	private:
		//==============================
		// Internal Fields
		//==============================
		static inline ScriptModuleBinder s_AppScriptBinder{};
	};
}


