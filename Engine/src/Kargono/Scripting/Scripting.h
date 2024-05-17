#pragma once


#include "Kargono/Core/Base.h"
#include "Kargono/Core/UUID.h"
#include "Kargono/Core/WrappedData.h"
#include "Kargono/Scripting/ScriptingResources.h"
#include "Kargono/Script/ScriptEngine.h"

#include <filesystem>
#include <functional>
#include <string>
#include <vector>


namespace Kargono::Assets { class AssetManager; }

namespace Kargono::Scripting
{
	class Script;

	class ScriptCore
	{
	public:
		static void Init();
		static void Terminate();
	public:
		static void OpenDll();
		static void CloseDll();
		
		static void LoadScriptFunction(Ref<Script> script, WrappedFuncType funcType);
	};

	class Script
	{
	public:
		std::string m_ScriptName{};
		ScriptType m_ScriptType{ ScriptType::None };
		std::string m_SectionLabel {"None"};
		WrappedFuncType m_FuncType{ WrappedFuncType::None };
		UUID m_ID{};
		Ref<WrappedFunction> m_Function {nullptr};
	private:
		friend class Assets::AssetManager;
	};
}
