#pragma once

#include "Kargono/Core/Base.h"
#include "Modules/Scripting/ScriptingCommon.h"
#include "Kargono/Core/UUID.h"
#include "Modules/Scripting/Module/ScriptingModule.h"
#include "Modules/Assets/AssetsCommon.h"
#include "Modules/Assets/Module/AssetTag.h"

#include <unordered_set>
#include <string>

namespace Kargono::Scripting
{
	struct ScriptSpec
	{
	public:
		Scripting::ScriptType m_Type{ Scripting::ScriptType::None };
		FixedBufStr32 m_SectionLabel{};
		WrappedFuncType m_FunctionType{ WrappedFuncType::None };
		Scripting::ExplicitFuncType m_ExplicitFuncType{};
	};

	struct ScriptRegistryExtension
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		ScriptRegistryExtension() = default;
		~ScriptRegistryExtension() = default;
	public:
		//==============================
		// Manage Section Labels
		//==============================
		bool AddScriptSectionLabel(std::string_view newLabel);
		bool DeleteScriptSectionLabel(std::string_view label);
		bool EditScriptSectionLabel(std::string_view oldLabel, std::string_view newLabel);
	public:
		//==============================
		// Serialization
		//==============================
		void Serialize(void* context);
		void Deserialize(void* context);
	public:
		//==============================
		// Public Fields
		//==============================
		std::unordered_set<FixedBufStr32> m_ScriptSectionLabels{};
	};

	struct ScriptMetaData
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		ScriptMetaData() = default;
		~ScriptMetaData() = default;
	public:
		//==============================
		// Serialization
		//==============================
		void Serialize(void* context);
		void Deserialize(void* context);
	public:
		//==============================
		// Public Fields
		//==============================
		Scripting::ScriptType m_ScriptType{ Scripting::ScriptType::None };
		FixedBufStr32 m_SectionLabel{};
		WrappedFuncType m_FunctionType{};
		Scripting::ExplicitFuncType m_ExplicitFuncType{};
	};

	class Script
	{
	public:
		//==============================
		// Metaprogramming Info
		//==============================
		using Spec = ScriptSpec;
		using RegistryExtension = ScriptRegistryExtension;
		using MetadataExtension = ScriptMetaData;
	public:
		//==============================
		// Asset Config Info
		//==============================
		constexpr static FixedBufStr32 GetDisplayName()
		{
			return "Kargono Script";
		}

		constexpr static Assets::AssetFlags GetAssetFlags()
		{
			Assets::AssetFlags flags{};
			flags.SetFlag(Assets::AssetFlag::HasAssetCache);
			flags.SetFlag(Assets::AssetFlag::RequireUniqueName);
			return flags;
		}

		constexpr static FixedBufStr16 GetFileExtension()
		{
			return ".kgscript";
		}

		void CreateFromSpec(Assets::Metadata<Script>& metadata, const ScriptSpec& spec);
		static bool ValidateCreateFromSpec(const Assets::AssetCreationData& creationData, const ScriptSpec& spec);
		void ValidateDelete(Assets::Metadata<Script>& metadata);
		void UpdateFromSpec(Assets::Metadata<Script>& metadata, const ScriptSpec& spec);
		
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		Script() = default;
		~Script() = default;
	public:
		//==============================
		// Serialization
		//==============================
		void Serialize(void* context);
		void Deserialize(void* context);
	public:
		//==============================
		// Public Fields
		//==============================
		FixedBufStr32 m_ScriptName{};
		ScriptType m_ScriptType{ ScriptType::None };
		FixedBufStr32 m_SectionLabel{ "None" };
		UUID m_ID{ k_EmptyUUID };
		WrappedFuncType m_FuncType{ WrappedFuncType::None };
		Ref<WrappedFunction> m_Function{ nullptr };
		ExplicitFuncType m_ExplicitFuncType{};
	};

	Register_Module_Type(Script, Assets::AssetTag)
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
		std::stringstream outputStream{};
		outputStream << Utility::WrappedVarTypeToKGScript(returnType) << " " << name << "(";

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
		std::stringstream outputStream{};
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