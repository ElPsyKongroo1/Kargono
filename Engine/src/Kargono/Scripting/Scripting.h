#pragma once


#include "Kargono/Core/Base.h"
#include "Kargono/Core/UUID.h"
#include "Kargono/Core/WrappedData.h"

#include <filesystem>
#include <functional>
#include <string>
#include <vector>

#include "Kargono/Script/ScriptEngine.h"

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
		std::vector<WrappedVarType> m_Parameters{};
		WrappedVarType m_ReturnValue{WrappedVarType::None};
		UUID m_ID{};
		Ref<WrappedFunction> m_Function {nullptr};
	private:
		friend class Assets::AssetManager;
	};
}
