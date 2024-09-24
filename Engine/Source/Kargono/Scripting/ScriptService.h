#pragma once


#include "Kargono/Assets/Asset.h"
#include "Kargono/Core/Base.h"
#include "Kargono/Core/UUID.h"
#include "Kargono/Core/WrappedData.h"
#include "Kargono/Scripting/ScriptingCommon.h"

#include <filesystem>
#include <functional>
#include <string>
#include <sstream>
#include <vector>

namespace Kargono::Assets { class AssetManager; }

namespace Kargono::Scripting
{
	class Script;
	//==============================
	// Script Service Class 
	//==============================
	class ScriptService
	{
	public:
		//==============================
		// Lifecycle Functions
		//==============================
		static void Init();
		static void Terminate();
	public:
		//==============================
		// Manage Active Script Module
		//==============================
		static void LoadActiveScriptModule();
		static void CloseActiveScriptModule();

		//==============================
		// Manage Individual Scripts
		//==============================
		static void LoadScriptFunction(Ref<Script> script, WrappedFuncType funcType);

		//==============================
		// Getters/Setters
		//==============================
		static std::vector<Ref<Script>>& GetAllEngineScripts()
		{
			return s_AllEngineScripts;
		}
	private:
		static std::vector<Ref<Script>> s_AllEngineScripts;
	private:
		friend Assets::AssetManager;
	};
	//==============================
	// Script Class
	//==============================
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
	inline std::string ScriptToString(Ref<Scripting::Script> script)
	{
		return Utility::ScriptTypeToString(script->m_ScriptType) + "::" + script->m_SectionLabel + "::" + script->m_ScriptName;
	}

	//==============================
	// Utility for Creating Function Definition/Signatures
	//==============================
	inline std::string GenerateFunctionSignature(WrappedFuncType funcType, const std::string& name)
	{
		WrappedVarType returnType = Utility::WrappedFuncTypeToReturnType(funcType);
		std::vector<WrappedVarType> parameterTypes = Utility::WrappedFuncTypeToParameterTypes(funcType);
		// Write out return value and function name
		std::stringstream outputStream {};
		outputStream << Utility::WrappedVarTypeToKGScript(returnType) << " " << name<< "(";

		// Write out parameters into function signature
		char letterIteration{ 'a' };
		for (uint32_t iteration{ 0 }; static_cast<size_t>(iteration) < parameterTypes.size(); iteration++)
		{
			outputStream << Utility::WrappedVarTypeToKGScript(parameterTypes.at(iteration)) << " " << letterIteration;
			if (iteration != parameterTypes.size() - 1)
			{
				outputStream << ',';
			}
			letterIteration++;
		}
		outputStream << ")";
		return outputStream.str();
	}

	inline std::string GenerateFunctionStub(WrappedFuncType funcType, const std::string& name)
	{
		std::stringstream outputStream {};
		outputStream << GenerateFunctionSignature(funcType, name);
		outputStream << '\n';
		outputStream << "{" << "\n";
		WrappedVarType returnType = Utility::WrappedFuncTypeToReturnType(funcType);
		if (returnType != WrappedVarType::Void)
		{
			outputStream << "\treturn ";
			switch (returnType)
			{
				case WrappedVarType::Bool:
				{
					outputStream << "false\n";
					break;
				}
				case WrappedVarType::String:
				{
					outputStream << "\"\"\n";
					break;
				}
				case WrappedVarType::Vector3:
				{
					outputStream << "Math::vec3(0.0f, 0.0f, 0.0f)\n";
					break;
				}
				case WrappedVarType::Float:
				case WrappedVarType::UInteger16:
				case WrappedVarType::UInteger32:
				case WrappedVarType::Integer32:
				case WrappedVarType::UInteger64:
				{
					outputStream << "0\n";
					break;
				}
				case WrappedVarType::Void:
				case WrappedVarType::None:
				default:
				{
					KG_ERROR("Unsupported return type provided {}", Utility::WrappedVarTypeToString(returnType));
					break;
				}
			}
		}
		outputStream << "}" << "\n";

		return outputStream.str();
	}
}
