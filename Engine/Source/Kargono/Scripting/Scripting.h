#pragma once


#include "Kargono/Assets/Asset.h"
#include "Kargono/Core/Base.h"
#include "Kargono/Core/UUID.h"
#include "Kargono/Core/WrappedData.h"
#include "Kargono/Scripting/ScriptingResources.h"
#include "Kargono/Script/ScriptEngine.h"

#include <filesystem>
#include <functional>
#include <string>
#include <sstream>
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

namespace Kargono::Utility
{
	inline std::string GenerateFunctionSignature(WrappedFuncType funcType, Assets::AssetHandle handle)
	{
		WrappedVarType returnType = Utility::WrappedFuncTypeToReturnType(funcType);
		std::vector<WrappedVarType> parameterTypes = Utility::WrappedFuncTypeToParameterTypes(funcType);
		// Write out return value and function name
		std::stringstream outputStream {};
		outputStream << Utility::WrappedVarTypeToCPPString(returnType) << " KG_FUNC_" << handle << "(";

		// Write out parameters into function signature
		char letterIteration{ 'a' };
		for (uint32_t iteration{ 0 }; static_cast<size_t>(iteration) < parameterTypes.size(); iteration++)
		{
			outputStream << Utility::WrappedVarTypeToCPPString(parameterTypes.at(iteration)) << " " << letterIteration;
			if (iteration != parameterTypes.size() - 1)
			{
				outputStream << ',';
			}
			letterIteration++;
		}
		outputStream << ")";
		return outputStream.str();
	}

	inline std::string GenerateFunctionStub(WrappedFuncType funcType, Assets::AssetHandle handle)
	{
		std::stringstream outputStream {};
		outputStream << GenerateFunctionSignature(funcType, handle);
		outputStream << '\n';
		outputStream << "{" << "\n";
		outputStream << "}" << "\n";

		return outputStream.str();
	}
}