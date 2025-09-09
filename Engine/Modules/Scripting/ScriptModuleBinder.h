#pragma once

#include "Modules/Assets/Asset.h"
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
		void CreateModuleHeaderFile();
		bool CreateModuleCPPFile();
	private:
		//==============================
		// Compile Module
		//==============================
		bool CompileModuleCodeMSVC(bool createDebug);
		bool CompileModuleCodeGCC(bool createDebug);
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
		void LoadActiveScriptModule();
		void CloseActiveScriptModule();
		//==============================
		// Manage Individual Scripts
		//==============================
		void LoadScriptFunction(Ref<Script> script, WrappedFuncType funcType);
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

	class Script
	{
	public:
		std::string m_ScriptName{};
		ScriptType m_ScriptType{ ScriptType::None };
		std::string m_SectionLabel {"None"};
		WrappedFuncType m_FuncType{ WrappedFuncType::None };
		UUID m_ID{ k_EmptyUUID };
		Ref<WrappedFunction> m_Function {nullptr};
		ExplicitFuncType m_ExplicitFuncType{};
	};
}

namespace Kargono::Utility
{
	inline std::string ScriptToString(Ref<Scripting::Script> script)
	{
		std::string returnString = Utility::ScriptTypeToString(script->m_ScriptType);
		returnString.append("::");
		returnString.append(script->m_SectionLabel);
		returnString.append("::");
		returnString.append(script->m_ScriptName);
		return returnString;
	}


	inline std::string ScriptToEditorUIGroup(Ref<Scripting::Script> script)
	{
		std::string returnString = Utility::ScriptTypeToString(script->m_ScriptType);
		returnString.append("::");
		returnString.append(script->m_SectionLabel);
		return returnString;
	}

	//==============================
	// Utility for Creating Function Definition/Signatures
	//==============================
	inline std::string GenerateFunctionSignature(WrappedFuncType funcType, std::string_view name, const Scripting::ExplicitFuncType& explicitType)
	{
		// Initialize return type and parameter list depending on provided types
		WrappedVarType returnType;
		std::vector<WrappedVarType> parameterTypes;
		std::vector<FixedBufStr32> parameterNames;
		if (funcType == WrappedFuncType::ArbitraryFunction)
		{
			returnType = explicitType.m_ReturnType;
			parameterTypes = explicitType.m_ParameterTypes;
			parameterNames = explicitType.m_ParameterNames;
		}
		else
		{
			returnType = Utility::WrappedFuncTypeToReturnType(funcType);
			parameterTypes = Utility::WrappedFuncTypeToParameterTypes(funcType);

			// Check if a list of parameter names is available and is the correct size
			if (explicitType.m_ParameterNames.size() == parameterTypes.size())
			{
				// Simply use the existing names if available
				parameterNames = explicitType.m_ParameterNames;
			}
			else
			{
				// Fill parameterNames with default names
				char letterIteration{ 'a' };
				for (size_t iteration{ 0 }; iteration < parameterTypes.size(); iteration++)
				{
					parameterNames.emplace_back(letterIteration);
					letterIteration++;
				}
			}
		}

		// Write out return value and function name
		std::stringstream outputStream {};
		outputStream << Utility::WrappedVarTypeToKGScript(returnType) << " " << name<< "(";

		// Write out parameters into function signature
		for (std::size_t iteration{ 0 }; static_cast<size_t>(iteration) < parameterTypes.size(); iteration++)
		{
			outputStream << Utility::WrappedVarTypeToKGScript(parameterTypes.at(iteration)) << " " << parameterNames.at(iteration);
			if (iteration != parameterTypes.size() - 1)
			{
				outputStream << ',';
			}
		}
		outputStream << ")";
		return outputStream.str();
	}
	

	inline std::string GenerateFunctionStub(WrappedFuncType funcType, std::string_view name, const Scripting::ExplicitFuncType& explicitType)
	{
		std::stringstream outputStream {};
		outputStream << GenerateFunctionSignature(funcType, name, explicitType);
		outputStream << '\n';
		outputStream << "{" << "\n";

		// Get current return type
		WrappedVarType returnType;
		if (funcType == WrappedFuncType::ArbitraryFunction)
		{
			returnType = explicitType.m_ReturnType;
		}
		else
		{
			returnType = Utility::WrappedFuncTypeToReturnType(funcType);
		}

		// Write return statement if necessary
		if (returnType != WrappedVarType::Void)
		{
			outputStream << "\treturn ";
			switch (returnType)
			{
				case WrappedVarType::Bool:
				{
					outputStream << "false;\n";
					break;
				}
				case WrappedVarType::String:
				{
					outputStream << "\"\";\n";
					break;
				}
				case WrappedVarType::Vector2:
				{
					outputStream << "Math::vec2(0.0f, 0.0f);\n";
					break;
				}
				case WrappedVarType::Vector3:
				{
					outputStream << "Math::vec3(0.0f, 0.0f, 0.0f);\n";
					break;
				}
				case WrappedVarType::Vector4:
				{
					outputStream << "Math::vec4(0.0f, 0.0f, 0.0f, 0.0f);\n";
					break;
				}
				case WrappedVarType::IVector2:
				{
					outputStream << "Math::ivec2(0.0f, 0.0f);\n";
					break;
				}
				case WrappedVarType::IVector3:
				{
					outputStream << "Math::ivec3(0.0f, 0.0f, 0.0f);\n";
					break;
				}
				case WrappedVarType::IVector4:
				{
					outputStream << "Math::ivec4(0.0f, 0.0f, 0.0f, 0.0f);\n";
					break;
				}
				case WrappedVarType::Float:
				{
					outputStream << "0.0f;\n";
					break;
				}
				case WrappedVarType::Integer16:
				case WrappedVarType::Integer32:
				case WrappedVarType::Integer64:
				case WrappedVarType::UInteger16:
				case WrappedVarType::UInteger32:
				case WrappedVarType::UInteger64:
				{
					outputStream << "0;\n";
					break;
				}

				case WrappedVarType::Entity:
				{
					outputStream << "0;\n";
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
