#include "kgpch.h"

#include "Kargono/Scripting/ScriptCompilerService.h"

#include "Kargono/Utility/FileSystem.h"
#include "Kargono/Utility/Regex.h"
#include "Kargono/EditorUI/EditorUI.h"
#include "Kargono/Scripting/ScriptTokenizer.h"
#include "Kargono/Scripting/ScriptTokenParser.h"
#include "Kargono/Scripting/ScriptOutputGenerator.h"
#include "Kargono/Assets/AssetService.h"
#include "Kargono/ECS/ProjectComponent.h"


namespace Kargono::Utility
{
	inline Scripting::ScriptToken WrappedVarTypeToPrimitiveType(WrappedVarType type)
	{
		switch (type)
		{
		case WrappedVarType::Integer32: return { Scripting::ScriptTokenType::PrimitiveType, "int32" };
		case WrappedVarType::UInteger16: return { Scripting::ScriptTokenType::PrimitiveType, "uint16" };
		case WrappedVarType::UInteger32: return { Scripting::ScriptTokenType::PrimitiveType, "uint32" };
		case WrappedVarType::UInteger64: return { Scripting::ScriptTokenType::PrimitiveType, "uint64" };
		case WrappedVarType::Vector3: return { Scripting::ScriptTokenType::PrimitiveType, "vector3" };
		case WrappedVarType::String: return { Scripting::ScriptTokenType::PrimitiveType, "string" };
		case WrappedVarType::Bool: return { Scripting::ScriptTokenType::PrimitiveType, "bool" };
		case WrappedVarType::Float: return { Scripting::ScriptTokenType::PrimitiveType, "float" };
		case WrappedVarType::Void: return { Scripting::ScriptTokenType::None, "" };
		case WrappedVarType::None: return { Scripting::ScriptTokenType::None, "" };
		}
		KG_ERROR("Unknown Type of WrappedVariableType.");
		return { Scripting::ScriptTokenType::None, "" };
	}
}

namespace Kargono::Scripting
{
	LanguageDefinition ScriptCompilerService::s_ActiveLanguageDefinition {};

	static std::vector<ScriptToken> s_AllLiterals
	{
		{ ScriptTokenType::PrimitiveType, "bool" },
		{ ScriptTokenType::PrimitiveType, "int32" },
		{ ScriptTokenType::PrimitiveType, "uint16" },
		{ ScriptTokenType::PrimitiveType, "uint64" },
		{ ScriptTokenType::PrimitiveType, "uint32" },
		{ ScriptTokenType::PrimitiveType, "float" },
		{ ScriptTokenType::PrimitiveType, "string" }
	};

	static std::vector<ScriptToken> s_AllIntegerTypes
	{
		{ ScriptTokenType::PrimitiveType, "int32" },
		{ ScriptTokenType::PrimitiveType, "uint16" },
		{ ScriptTokenType::PrimitiveType, "uint32" },
		{ ScriptTokenType::PrimitiveType, "uint64" }
	};
	static std::vector<ScriptToken> s_AllLiteralsWithoutString
	{
		{ ScriptTokenType::PrimitiveType, "bool" },
		{ ScriptTokenType::PrimitiveType, "int32" },
		{ ScriptTokenType::PrimitiveType, "uint16" },
		{ ScriptTokenType::PrimitiveType, "uint32" },
		{ ScriptTokenType::PrimitiveType, "uint64" },
		{ ScriptTokenType::PrimitiveType, "float" }
	};

	std::string ScriptCompilerService::CompileScriptFile(const std::filesystem::path& scriptLocation)
	{
		// Lazy loading KGScript language def
		if (!s_ActiveLanguageDefinition)
		{
			CreateKGScriptLanguageDefinition();
		}

		// Check for invalid input
		if (!std::filesystem::exists(scriptLocation))
		{
			KG_WARN("Failed to compile .kgscript. File does not exist at specified location!");
			return {};
		}
		if (scriptLocation.extension() != ".kgscript")
		{
			KG_WARN("Failed to compile .kgscript. File uses incorrect extension!");
			return {};
		}

		// Load in script file from disk
		std::string scriptFile = Utility::FileSystem::ReadFileString(scriptLocation);

		// Get tokens from text
		ScriptTokenizer scriptTokenizer{};
		std::vector<ScriptToken> tokens = scriptTokenizer.TokenizeString(std::move(scriptFile));

		ScriptTokenParser tokenParser{};
		auto [parseSuccess, newAST] = tokenParser.ParseTokens(std::move(tokens));
		//tokenParser.PrintTokens();

		if (!parseSuccess)
		{
			KG_WARN("Token parsing failed");
			// Print out error messages
			tokenParser.PrintErrors();
			tokenParser.PrintTokens();
			tokenParser.PrintAST();
			return "Token parsing failed";
		}

		//tokenParser.PrintAST();

		// Generate output text
		ScriptOutputGenerator outputGenerator{};
		auto [outputSuccess, outputText] = outputGenerator.GenerateOutput(std::move(newAST));

		if (!outputSuccess)
		{
			KG_WARN("Output text generation failed");
			return "Failed to generate output text";
		}
		
		//KG_WARN(outputText);
		return outputText;
	}

	std::vector<ParserError> ScriptCompilerService::CheckForErrors(const std::string& text)
	{
		// Lazy loading KGScript language def
		if (!s_ActiveLanguageDefinition)
		{
			CreateKGScriptLanguageDefinition();
		}

		// Get tokens from text
		ScriptTokenizer scriptTokenizer{};
		std::vector<ScriptToken> tokens = scriptTokenizer.TokenizeString(text);

		ScriptTokenParser tokenParser{};
		auto [parseSuccess, newAST] = tokenParser.ParseTokens(std::move(tokens));
		if (!parseSuccess)
		{
			return tokenParser.GetErrors();
		}
		
		return {};
	}

	CursorContext ScriptCompilerService::FindCursorContext(const std::string& text)
	{
		// Lazy loading KGScript language def
		if (!s_ActiveLanguageDefinition)
		{
			CreateKGScriptLanguageDefinition();
		}

		// Get tokens from text
		ScriptTokenizer scriptTokenizer{}; 
		std::vector<ScriptToken> tokens = scriptTokenizer.TokenizeString(text);

		// Parse tokens and check for generated cursor context
		ScriptTokenParser tokenParser{};
		tokenParser.ParseTokens(std::move(tokens));
		auto [success, context] = tokenParser.GetCursorContext();
		if (success)
		{
			return context;
		}

		return {};
	}

	std::vector<SuggestionSpec> Scripting::ScriptCompilerService::GetSuggestions(const std::string& scriptText, const std::string& queryText)
	{
		// Lazy loading KGScript language def
		if (!s_ActiveLanguageDefinition)
		{
			CreateKGScriptLanguageDefinition();
		}

		// Get tokens from text
		ScriptTokenizer scriptTokenizer{};
		std::vector<ScriptToken> tokens = scriptTokenizer.TokenizeString(scriptText);

		// Parse tokens and check for generated cursor context
		ScriptTokenParser tokenParser{};
		tokenParser.ParseTokens(std::move(tokens));
		auto [success, context] = tokenParser.GetCursorContext();

		// Exit gracefully if no context was found
		if (!success)
		{
			return {};
		}

		// Generate suggestions using based on context flags
		std::vector<SuggestionSpec> allSuggestions;
		if (context.m_Flags.IsFlagSet((uint8_t)CursorFlags::AfterNamespaceResolution))
		{
			GetSuggestionsForAfterNamespace(allSuggestions, context, queryText);
		}
		else if (context.m_Flags.IsFlagSet((uint8_t)CursorFlags::IsDataMember))
		{
			GetSuggestionsForIsDataMember(allSuggestions, context, queryText);
		}
		else if (context.m_Flags.IsFlagSet((uint8_t)CursorFlags::IsFunctionParameter))
		{
			GetSuggestionsForIsParameter(allSuggestions, context, queryText);
		}
		else
		{
			GetSuggestionsDefault(allSuggestions, context, queryText);
		}
		

		return allSuggestions;
	}

	void Scripting::ScriptCompilerService::GetSuggestionsForAfterNamespace(std::vector<SuggestionSpec>& allSuggestions, const CursorContext& context, const std::string& queryText)
	{
		// Generate suggestions for function identifiers
		for (auto& [funcName, funcNode] : ScriptCompilerService::s_ActiveLanguageDefinition.FunctionDefinitions)
		{
			// Ensure namespaces match
			if (context.CurrentNamespace.Value != funcNode.Namespace.Value)
			{
				continue;
			}

			// Decide whether to insert function
			std::string label = funcNode.Namespace ? funcNode.Namespace.Value + "::" + funcNode.Name.Value : funcNode.Name.Value;
			if (Utility::Regex::GetMatchSuccess(label, queryText, false))
			{
				SuggestionSpec newSuggestion;
				newSuggestion.m_Label = label;
				newSuggestion.m_ReplacementText = funcNode.Name.Value + "()";
				newSuggestion.m_Icon = EditorUI::EditorUIService::s_IconFunction;
				allSuggestions.push_back(newSuggestion);
			}
		}
	}

	void Scripting::ScriptCompilerService::GetSuggestionsForIsParameter(std::vector<SuggestionSpec>& allSuggestions, const CursorContext& context, const std::string& queryText)
	{
		// Generate suggestions for primitive types
		for (auto& [name, primitiveType] : ScriptCompilerService::s_ActiveLanguageDefinition.PrimitiveTypes)
		{
			if (Utility::Regex::GetMatchSuccess(primitiveType.Name, queryText, false))
			{
				SuggestionSpec newSuggestion;
				newSuggestion.m_Label = primitiveType.Name;
				newSuggestion.m_ReplacementText = primitiveType.Name;
				newSuggestion.m_Icon = Kargono::EditorUI::EditorUIService::s_IconEntity;
				allSuggestions.push_back(newSuggestion);
			}
		}
	}

	void Scripting::ScriptCompilerService::GetSuggestionsForIsDataMember(std::vector<SuggestionSpec>& allSuggestions, const CursorContext& context, const std::string& queryText)
	{
		// Generate suggestions for all member fields
		for (auto& [name, member] : context.DataMembers)
		{
			if (DataMember* dataMember = std::get_if<DataMember>(&member->Value))
			{
				if (Utility::Regex::GetMatchSuccess(dataMember->Name, queryText, false))
				{
					SuggestionSpec newSuggestion;
					newSuggestion.m_Label = dataMember->Name;
					newSuggestion.m_ReplacementText = dataMember->Name;
					newSuggestion.m_Icon = Kargono::EditorUI::EditorUIService::s_IconEntity;
					allSuggestions.push_back(newSuggestion);
				}	
			}
			else if (FunctionNode* funcNode = std::get_if<FunctionNode>(&member->Value))
			{
				if (Utility::Regex::GetMatchSuccess(funcNode->Name.Value, queryText, false))
				{
					SuggestionSpec newSuggestion;
					newSuggestion.m_Label = funcNode->Name.Value;
					newSuggestion.m_ReplacementText = funcNode->Name.Value + "()";
					newSuggestion.m_Icon = Kargono::EditorUI::EditorUIService::s_IconFunction;
					allSuggestions.push_back(newSuggestion);
				}
			}
		}
	}

	void Scripting::ScriptCompilerService::GetSuggestionsDefault(std::vector<SuggestionSpec>& allSuggestions, const CursorContext& context, const std::string& queryText)
	{
		// Generate suggestions for primitive types
		if (context.m_Flags.IsFlagSet((uint8_t)CursorFlags::AllowAllVariableTypes))
		{
			for (auto& [name, primitiveType] : ScriptCompilerService::s_ActiveLanguageDefinition.PrimitiveTypes)
			{
				if (Utility::Regex::GetMatchSuccess(primitiveType.Name, queryText, false))
				{
					SuggestionSpec newSuggestion;
					newSuggestion.m_Label = primitiveType.Name;
					newSuggestion.m_ReplacementText = primitiveType.Name;
					newSuggestion.m_Icon = Kargono::EditorUI::EditorUIService::s_IconEntity;
					allSuggestions.push_back(newSuggestion);
				}
			}
		}

		// Generate suggestions for stack variables
		for (auto& stackFrame : context.StackVariables)
		{
			for (auto& variable : stackFrame)
			{
				bool returnTypesMatch = false;
				for (auto& type : context.AllReturnTypes)
				{
					if (variable.Type.Value == type.Value)
					{
						returnTypesMatch = true;
						break;
					}
				}

				if (context.m_Flags.IsFlagSet((uint8_t)CursorFlags::AllowAllVariableTypes) || returnTypesMatch)
				{
					if (Utility::Regex::GetMatchSuccess(variable.Identifier.Value, queryText, false))
					{
						SuggestionSpec newSuggestion;
						newSuggestion.m_Label = variable.Identifier.Value;
						newSuggestion.m_ReplacementText = variable.Identifier.Value;
						newSuggestion.m_Icon = Kargono::EditorUI::EditorUIService::s_IconEntity;
						allSuggestions.push_back(newSuggestion);
					}
				}
			}
		}
		
		// Handle Functions Identifiers
		for (auto& [funcName, funcNode] : ScriptCompilerService::s_ActiveLanguageDefinition.FunctionDefinitions)
		{
			// Determine if the return types indeed match
			bool returnTypesMatch = false;
			for (auto& type : context.AllReturnTypes)
			{
				if (funcNode.ReturnType.Value == type.Value)
				{
					returnTypesMatch = true;
					break;
				}
			}

			if (context.m_Flags.IsFlagSet((uint8_t)CursorFlags::AllowAllVariableTypes) || returnTypesMatch)
			{
				// Decide whether to insert function
				std::string label = funcNode.Namespace ? funcNode.Namespace.Value + "::" + funcNode.Name.Value : funcNode.Name.Value;
				if (Utility::Regex::GetMatchSuccess(label, queryText, false))
				{
					SuggestionSpec newSuggestion;
					newSuggestion.m_Label = label;
					newSuggestion.m_ReplacementText = funcNode.Namespace ? funcNode.Namespace.Value + "::" + funcNode.Name.Value + "()" : funcNode.Name.Value + "()";
					newSuggestion.m_Icon = EditorUI::EditorUIService::s_IconFunction;
					allSuggestions.push_back(newSuggestion);
				}
			}
		}
		
	}



	void ScriptCompilerService::CreateKGScriptLanguageDefinition()
	{
		s_ActiveLanguageDefinition = {};

		// Load in AI Message Types from current project
		KG_ASSERT(Projects::ProjectService::GetActive());
		for (const std::string& messageType : Projects::ProjectService::GetAllMessageTypes())
		{
			s_ActiveLanguageDefinition.AllMessageTypes.insert(messageType);
		}

		CreateKGScriptKeywords();

		CreateKGScriptInitializationPrototypes();

		CreateKGScriptPrimitiveTypes();

		CreateKGScriptNamespaces();

		CreateKGScriptFunctionDefinitions();

	}

	void ScriptCompilerService::CreateKGScriptKeywords()
	{
		s_ActiveLanguageDefinition.Keywords =
		{
			"return",
			"void",
			"if",
			"else"
		};
	}

	void ScriptCompilerService::CreateKGScriptInitializationPrototypes()
	{
		// Add initialization list constructor prototypes
		InitializationListType newInitListType{};
		ScriptToken newInitListValue{};

		newInitListValue.Type = ScriptTokenType::PrimitiveType;
		newInitListValue.Value = "vector2";
		newInitListType.ReturnType = newInitListValue;
		newInitListValue.Type = ScriptTokenType::PrimitiveType;
		newInitListValue.Value = "float";
		newInitListType.ParameterTypes.push_back(newInitListValue);
		newInitListType.ParameterTypes.push_back(newInitListValue);
		s_ActiveLanguageDefinition.InitListTypes.push_back(newInitListType);

		newInitListType = {};
		newInitListValue = {};

		newInitListValue.Type = ScriptTokenType::PrimitiveType;
		newInitListValue.Value = "vector3";
		newInitListType.ReturnType = newInitListValue;
		newInitListValue.Type = ScriptTokenType::PrimitiveType;
		newInitListValue.Value = "float";
		newInitListType.ParameterTypes.push_back(newInitListValue);
		newInitListType.ParameterTypes.push_back(newInitListValue);
		newInitListType.ParameterTypes.push_back(newInitListValue);
		s_ActiveLanguageDefinition.InitListTypes.push_back(newInitListType);

		newInitListType = {};
		newInitListValue = {};

		newInitListValue.Type = ScriptTokenType::PrimitiveType;
		newInitListValue.Value = "vector4";
		newInitListType.ReturnType = newInitListValue;
		newInitListValue.Type = ScriptTokenType::PrimitiveType;
		newInitListValue.Value = "float";
		newInitListType.ParameterTypes.push_back(newInitListValue);
		newInitListType.ParameterTypes.push_back(newInitListValue);
		newInitListType.ParameterTypes.push_back(newInitListValue);
		newInitListType.ParameterTypes.push_back(newInitListValue);
		s_ActiveLanguageDefinition.InitListTypes.push_back(newInitListType);

		newInitListType = {};
		newInitListValue = {};
	}

	void ScriptCompilerService::CreateKGScriptPrimitiveTypes()
	{
		// Add basic/primitive data types
		PrimitiveType newPrimitiveType{};
		DataMember newDataMember{};
		FunctionNode newFunctionMember{};
		FunctionParameter newMemberParameter{};
		ScriptToken dataMemberPrimitiveType{};

		newPrimitiveType.Name = "bool";
		newPrimitiveType.Description = "Basic 8 bit type representing either true or false. Ex: false";
		newPrimitiveType.AcceptableLiteral = ScriptTokenType::BooleanLiteral;
		newPrimitiveType.EmittedDeclaration = "bool";
		newPrimitiveType.EmittedParameter = "bool";
		newPrimitiveType.Icon = EditorUI::EditorUIService::s_IconBoolean;
		s_ActiveLanguageDefinition.PrimitiveTypes.insert_or_assign(newPrimitiveType.Name, newPrimitiveType);

		newPrimitiveType.Name = "keycode";
		newPrimitiveType.Description = "A predefined type that contains all available input keys. Ex: Key::RightShift or Key::A";
		newPrimitiveType.AcceptableLiteral = ScriptTokenType::InputKeyLiteral;
		newPrimitiveType.EmittedDeclaration = "KeyCode";
		newPrimitiveType.EmittedParameter = "KeyCode";
		newPrimitiveType.Icon = EditorUI::EditorUIService::s_IconInput;
		s_ActiveLanguageDefinition.PrimitiveTypes.insert_or_assign(newPrimitiveType.Name, newPrimitiveType);

		newPrimitiveType.Name = "message_type";
		newPrimitiveType.Description = "A type that enumerates the different messages that can be sent between entities directly";
		newPrimitiveType.AcceptableLiteral = ScriptTokenType::MessageTypeLiteral;
		newPrimitiveType.EmittedDeclaration = "uint32_t";
		newPrimitiveType.EmittedParameter = "uint32_t";
		newPrimitiveType.Icon = EditorUI::EditorUIService::s_IconRight;
		s_ActiveLanguageDefinition.PrimitiveTypes.insert_or_assign(newPrimitiveType.Name, newPrimitiveType);

		newPrimitiveType = {};
		newPrimitiveType.Name = "string";
		newPrimitiveType.Description = "Basic type representing a list of ASCII characters. Ex: \"Hello World\", \"This is a sample sentence\"";
		newPrimitiveType.AcceptableLiteral = ScriptTokenType::StringLiteral;
		newPrimitiveType.EmittedDeclaration = "std::string";
		newPrimitiveType.EmittedParameter = "const std::string&";
		newPrimitiveType.Icon = EditorUI::EditorUIService::s_IconTextWidget;
		s_ActiveLanguageDefinition.PrimitiveTypes.insert_or_assign(newPrimitiveType.Name, newPrimitiveType);

		newPrimitiveType = {};
		newPrimitiveType.Name = "int32";
		newPrimitiveType.Description = "Basic signed integer type that is 32 bits wide. This variable can only hold discrete values between -2,147,483,648 to 2,147,483,647";
		newPrimitiveType.AcceptableLiteral = ScriptTokenType::IntegerLiteral;
		newPrimitiveType.EmittedDeclaration = "int32_t";
		newPrimitiveType.EmittedParameter = "int32_t";
		newPrimitiveType.Icon = EditorUI::EditorUIService::s_IconNumber;
		s_ActiveLanguageDefinition.PrimitiveTypes.insert_or_assign(newPrimitiveType.Name, newPrimitiveType);

		newPrimitiveType = {};
		newPrimitiveType.Name = "uint16";
		newPrimitiveType.Description = "Basic unsigned integer type that is 16 bits wide. This variable can only hold discrete values between 0 to 65,535";
		newPrimitiveType.AcceptableLiteral = ScriptTokenType::IntegerLiteral;
		newPrimitiveType.EmittedDeclaration = "uint16_t";
		newPrimitiveType.EmittedParameter = "uint16_t";
		newPrimitiveType.Icon = EditorUI::EditorUIService::s_IconNumber;
		s_ActiveLanguageDefinition.PrimitiveTypes.insert_or_assign(newPrimitiveType.Name, newPrimitiveType);

		newPrimitiveType = {};
		newPrimitiveType.Name = "uint32";
		newPrimitiveType.Description = "Basic unsigned integer type that is 32 bits wide. This variable can only hold discrete values between 0 to 4,294,967,295";
		newPrimitiveType.AcceptableLiteral = ScriptTokenType::IntegerLiteral;
		newPrimitiveType.EmittedDeclaration = "uint32_t";
		newPrimitiveType.EmittedParameter = "uint32_t";
		newPrimitiveType.Icon = EditorUI::EditorUIService::s_IconNumber;
		s_ActiveLanguageDefinition.PrimitiveTypes.insert_or_assign(newPrimitiveType.Name, newPrimitiveType);

		newPrimitiveType = {};
		newPrimitiveType.Name = "uint64";
		newPrimitiveType.Description = "Basic unsigned integer type that is 64 bits wide. This variable can only hold discrete values between 0 to 18,446,744,073,709,551,615";
		newPrimitiveType.AcceptableLiteral = ScriptTokenType::IntegerLiteral;
		newPrimitiveType.EmittedDeclaration = "uint64_t";
		newPrimitiveType.EmittedParameter = "uint64_t";
		newPrimitiveType.Icon = EditorUI::EditorUIService::s_IconNumber;
		s_ActiveLanguageDefinition.PrimitiveTypes.insert_or_assign(newPrimitiveType.Name, newPrimitiveType);

		newPrimitiveType = {};
		newPrimitiveType.Name = "entity";
		newPrimitiveType.Description = "This type represents a specific entity in the active scene. An entity is the basic unit that exists inside of a scene. Entities are composed of multiple components. Note that this type can represent entities in other scenes, however, this can lead to undefined behavior.";
		newPrimitiveType.AcceptableLiteral = ScriptTokenType::None;
		newPrimitiveType.EmittedDeclaration = "uint64_t";
		newPrimitiveType.EmittedParameter = "uint64_t";
		newPrimitiveType.Icon = EditorUI::EditorUIService::s_IconEntity;

		newFunctionMember.Name = { ScriptTokenType::Identifier, "HasComponent" };
		newFunctionMember.Namespace = {};
		newFunctionMember.ReturnType = { ScriptTokenType::PrimitiveType, "bool" };
		newFunctionMember.Description = "This member function checks whether the selected entity has the provided component. This function takes the name of the component as an argument and returns a boolean indicating whether the entity has indicated the component.";
		newMemberParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "string" });
		newMemberParameter.Identifier = { ScriptTokenType::Identifier, "componentName" };
		newFunctionMember.Parameters.push_back(newMemberParameter);
		newFunctionMember.OnGenerateGetter = [](ScriptOutputGenerator& generator, MemberNode& member)
		{
			FunctionCallNode* funcCall = std::get_if<FunctionCallNode>(&member.ChildMemberNode->CurrentNodeExpression->Value);

			generator.m_OutputText << "CheckHasComponent(";
			generator.GenerateExpression(member.CurrentNodeExpression);
			generator.m_OutputText << ", ";
			generator.GenerateExpression(funcCall->Arguments.at(0));
			generator.m_OutputText << ")";
		};
		newMemberParameter = {};
		newPrimitiveType.Members.insert_or_assign(newFunctionMember.Name.Value, CreateRef<MemberType>(newFunctionMember));
		newFunctionMember = {};

		newFunctionMember.Name = { ScriptTokenType::Identifier, "GetFieldFloat" };
		newFunctionMember.Namespace = {};
		newFunctionMember.ReturnType = { ScriptTokenType::PrimitiveType, "float" };
		newFunctionMember.Description = "This member function gets the field denoted by its name and returns it. This function only returns floats. The argument is the name of the field.";
		newMemberParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "string" });
		newMemberParameter.Identifier = { ScriptTokenType::Identifier, "fieldName" };
		newFunctionMember.Parameters.push_back(newMemberParameter);
		newFunctionMember.OnGenerateGetter = [](ScriptOutputGenerator& generator, MemberNode& member)
		{
			FunctionCallNode* funcCall = std::get_if<FunctionCallNode>(&member.ChildMemberNode->CurrentNodeExpression->Value);

			generator.m_OutputText << "*(float*)GetEntityFieldByName(";
			generator.GenerateExpression(member.CurrentNodeExpression);
			generator.m_OutputText << ", ";
			generator.GenerateExpression(funcCall->Arguments.at(0));
			generator.m_OutputText << ")";
		};
		newMemberParameter = {};
		newPrimitiveType.Members.insert_or_assign(newFunctionMember.Name.Value, CreateRef<MemberType>(newFunctionMember));
		newFunctionMember = {};

		newFunctionMember.Name = { ScriptTokenType::Identifier, "GetFieldVector3" };
		newFunctionMember.Namespace = {};
		newFunctionMember.ReturnType = { ScriptTokenType::PrimitiveType, "vector3" };
		newFunctionMember.Description = "This member function gets the field denoted by its name and returns it. This function only returns vector3 fields. The argument is the name of the field.";
		newMemberParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "string" });
		newMemberParameter.Identifier = { ScriptTokenType::Identifier, "fieldName" };
		newFunctionMember.Parameters.push_back(newMemberParameter);
		newFunctionMember.OnGenerateGetter = [](ScriptOutputGenerator& generator, MemberNode& member)
		{
			FunctionCallNode* funcCall = std::get_if<FunctionCallNode>(&member.ChildMemberNode->CurrentNodeExpression->Value);

			generator.m_OutputText << "*(Math::vec3*)GetEntityFieldByName(";
			generator.GenerateExpression(member.CurrentNodeExpression);
			generator.m_OutputText << ", ";
			generator.GenerateExpression(funcCall->Arguments.at(0));
			generator.m_OutputText << ")";
		};
		newMemberParameter = {};
		newPrimitiveType.Members.insert_or_assign(newFunctionMember.Name.Value, CreateRef<MemberType>(newFunctionMember));
		newFunctionMember = {};

		newFunctionMember.Name = { ScriptTokenType::Identifier, "GetFieldUInt32" };
		newFunctionMember.Namespace = {};
		newFunctionMember.ReturnType = { ScriptTokenType::PrimitiveType, "uint32" };
		newFunctionMember.Description = "This member function gets the field denoted by its name and returns it. This function only returns uint32 fields. The argument is the name of the field.";
		newMemberParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "string" });
		newMemberParameter.Identifier = { ScriptTokenType::Identifier, "fieldName" };
		newFunctionMember.Parameters.push_back(newMemberParameter);
		newFunctionMember.OnGenerateGetter = [](ScriptOutputGenerator& generator, MemberNode& member)
		{
			FunctionCallNode* funcCall = std::get_if<FunctionCallNode>(&member.ChildMemberNode->CurrentNodeExpression->Value);

			generator.m_OutputText << "*(uint32_t*)GetEntityFieldByName(";
			generator.GenerateExpression(member.CurrentNodeExpression);
			generator.m_OutputText << ", ";
			generator.GenerateExpression(funcCall->Arguments.at(0));
			generator.m_OutputText << ")";
		};
		newMemberParameter = {};
		newPrimitiveType.Members.insert_or_assign(newFunctionMember.Name.Value, CreateRef<MemberType>(newFunctionMember));
		newFunctionMember = {};


		newFunctionMember.Name = { ScriptTokenType::Identifier, "SetFieldUInt32" };
		newFunctionMember.Namespace = {};
		newFunctionMember.ReturnType = {};
		newFunctionMember.Description = "This member function sets the field denoted by its name to the value provided. This function takes any available primitive type.";
		newMemberParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "string" });
		newMemberParameter.Identifier = { ScriptTokenType::Identifier, "fieldName" };
		newFunctionMember.Parameters.push_back(newMemberParameter);
		newMemberParameter = {};
		newMemberParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "uint32" });
		newMemberParameter.Identifier = { ScriptTokenType::Identifier, "newValue" };
		newFunctionMember.Parameters.push_back(newMemberParameter);
		newMemberParameter = {};
		newFunctionMember.OnGenerateGetter = [](ScriptOutputGenerator& generator, MemberNode& member)
		{
			FunctionCallNode* funcCall = std::get_if<FunctionCallNode>(&member.ChildMemberNode->CurrentNodeExpression->Value);

			generator.m_OutputText << "SetEntityFieldByName(";
			generator.GenerateExpression(member.CurrentNodeExpression);
			generator.m_OutputText << ", ";
			generator.GenerateExpression(funcCall->Arguments.at(0));
			generator.m_OutputText << ", ";
			generator.m_OutputText << "(void*)&RValueToLValue(";
			generator.GenerateExpression(funcCall->Arguments.at(1));
			generator.m_OutputText << "))";
		};
		newPrimitiveType.Members.insert_or_assign(newFunctionMember.Name.Value, CreateRef<MemberType>(newFunctionMember));
		newFunctionMember = {};

		newFunctionMember.Name = { ScriptTokenType::Identifier, "SetFieldVector3" };
		newFunctionMember.Namespace = {};
		newFunctionMember.ReturnType = {};
		newFunctionMember.Description = "This member function sets the field denoted by its name to the value provided. This function takes a vector3 as the newValue.";
		newMemberParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "string" });
		newMemberParameter.Identifier = { ScriptTokenType::Identifier, "fieldName" };
		newFunctionMember.Parameters.push_back(newMemberParameter);
		newMemberParameter = {};
		newMemberParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "vector3" });
		newMemberParameter.Identifier = { ScriptTokenType::Identifier, "newValue" };
		newFunctionMember.Parameters.push_back(newMemberParameter);
		newMemberParameter = {};
		newFunctionMember.OnGenerateGetter = [](ScriptOutputGenerator& generator, MemberNode& member)
		{
			FunctionCallNode* funcCall = std::get_if<FunctionCallNode>(&member.ChildMemberNode->CurrentNodeExpression->Value);

			generator.m_OutputText << "SetEntityFieldByName(";
			generator.GenerateExpression(member.CurrentNodeExpression);
			generator.m_OutputText << ", ";
			generator.GenerateExpression(funcCall->Arguments.at(0));
			generator.m_OutputText << ", ";
			generator.m_OutputText << "(void*)&RValueToLValue(";
			generator.GenerateExpression(funcCall->Arguments.at(1));
			generator.m_OutputText << "))";
		};
		newPrimitiveType.Members.insert_or_assign(newFunctionMember.Name.Value, CreateRef<MemberType>(newFunctionMember));
		newFunctionMember = {};

		// Provide all project components as member data for the entity type
		for (auto& [handle, asset] : Assets::AssetService::GetProjectComponentRegistry())
		{
			Ref<ECS::ProjectComponent> projectComp = Assets::AssetService::GetProjectComponent(handle);
			KG_ASSERT(projectComp);

			// Initialize project component data
			DataMember projectComponentMember{};
			projectComponentMember.Name = projectComp->m_Name;
			projectComponentMember.Description = "This is a custom project component. This component provides fields specific to this component.";
			projectComponentMember.PrimitiveType.Type = ScriptTokenType::None;
			projectComponentMember.PrimitiveType.Value = "None";

			// Load each field from project component into projectComponentMember's member list
			for (size_t iteration{0}; iteration < projectComp->m_DataNames.size(); iteration++)
			{
				DataMember projectComponentFieldMember{};
				projectComponentFieldMember.Name = projectComp->m_DataNames.at(iteration);
				projectComponentFieldMember.Description = "This is a custom project component. This component provides fields specific to this component.";
				projectComponentFieldMember.PrimitiveType = Utility::WrappedVarTypeToPrimitiveType(projectComp->m_DataTypes.at(iteration));
				projectComponentFieldMember.OnGenerateGetter = [](ScriptOutputGenerator& generator, MemberNode& member)
				{
					
					generator.m_OutputText << "*(";
					generator.m_OutputText << Utility::WrappedVarTypeToCPPString(Utility::KGScriptToWrappedVarType(member.ReturnType.Value));
					generator.m_OutputText << "*)";
					generator.m_OutputText << "Scenes_GetProjectComponentField(";

					// Output entity ID
					generator.GenerateExpression(member.CurrentNodeExpression);
					generator.m_OutputText << ", ";

					// Output project component ID
					TokenExpressionNode* projectComponentExpression = std::get_if<TokenExpressionNode>(&member.ChildMemberNode->CurrentNodeExpression->Value);
					KG_ASSERT(projectComponentExpression);
					Ref<ECS::ProjectComponent> component = nullptr;
					for (auto& [handle, asset] : Assets::AssetService::GetProjectComponentRegistry())
					{
						if (asset.Data.GetSpecificMetaData<Assets::ProjectComponentMetaData>()->Name == projectComponentExpression->Value.Value)
						{
							component = Assets::AssetService::GetProjectComponent(handle);
							generator.m_OutputText << std::to_string(handle);
							break;
						}
					}
					KG_ASSERT(component);
					generator.m_OutputText << ", ";

					// Output component field ID
					TokenExpressionNode* fieldNameExpression = std::get_if<TokenExpressionNode>(&member.ChildMemberNode->ChildMemberNode->CurrentNodeExpression->Value);
					KG_ASSERT(fieldNameExpression);
					size_t iteration{ 0 };
					for (const std::string& fieldName : component->m_DataNames)
					{
						if (fieldName == fieldNameExpression->Value.Value)
						{
							generator.m_OutputText << std::to_string(iteration);
							break;
						}
						iteration++;
					}

					generator.m_OutputText << ")";
				};

				projectComponentFieldMember.OnGenerateSetter = [](ScriptOutputGenerator& generator, StatementAssignment& assignmentStatement)
				{
					MemberNode* memberNode = std::get_if<MemberNode>(&assignmentStatement.Name->Value);
					KG_ASSERT(memberNode);

					generator.m_OutputText << "Scenes_SetProjectComponentField(";

					// Generate entityID
					generator.GenerateExpression(memberNode->CurrentNodeExpression);
					generator.m_OutputText << ", ";

					// Output project component ID
					TokenExpressionNode* projectComponentExpression = std::get_if<TokenExpressionNode>(&memberNode->ChildMemberNode->CurrentNodeExpression->Value);
					KG_ASSERT(projectComponentExpression);
					Ref<ECS::ProjectComponent> component = nullptr;
					for (auto& [handle, asset] : Assets::AssetService::GetProjectComponentRegistry())
					{
						if (asset.Data.GetSpecificMetaData<Assets::ProjectComponentMetaData>()->Name == projectComponentExpression->Value.Value)
						{
							component = Assets::AssetService::GetProjectComponent(handle);
							generator.m_OutputText << std::to_string(handle);
							break;
						}
					}
					KG_ASSERT(component);
					generator.m_OutputText << ", ";

					// Output component field ID
					TokenExpressionNode* fieldNameExpression = std::get_if<TokenExpressionNode>(&memberNode->ChildMemberNode->ChildMemberNode->CurrentNodeExpression->Value);
					KG_ASSERT(fieldNameExpression);
					size_t iteration{ 0 };
					for (const std::string& fieldName : component->m_DataNames)
					{
						if (fieldName == fieldNameExpression->Value.Value)
						{
							generator.m_OutputText << std::to_string(iteration);
							break;
						}
						iteration++;
					}
					generator.m_OutputText << ", ";

					// Output expression
					if (TokenExpressionNode* tokenExpression = std::get_if<TokenExpressionNode>(&assignmentStatement.Value->Value))
					{
						if (tokenExpression->Value.Type == ScriptTokenType::Identifier)
						{
							generator.m_OutputText << "&" << tokenExpression->Value.Value;
						}
						else if (IsLiteral(tokenExpression->Value))
						{
							generator.m_OutputText << "(void*)&RValueToLValue(" << tokenExpression->Value.Value << ")";
						}
						else
						{
							KG_WARN("Invalid argument type provided to GameState::SetField");
							return;
						}
					}
					else
					{
						assignmentStatement.Value->GenerationAffixes = CreateRef<ExpressionGenerationAffixes>("(void*)&RValueToLValue(", ")");
						generator.GenerateExpression(assignmentStatement.Value);
					}

					generator.m_OutputText << ")";
				};
				projectComponentMember.Members.insert_or_assign(projectComponentFieldMember.Name, CreateRef<MemberType>(projectComponentFieldMember));
			}

			// Insert project component into the entity type's member list
			newPrimitiveType.Members.insert_or_assign(projectComponentMember.Name, CreateRef<MemberType>(projectComponentMember));
		}

		newDataMember.Name = "Transform";
		newDataMember.Description = "This entity member is a transform component. This component stores the location, size, and rotation of the provided entity.";
		newDataMember.PrimitiveType.Type = ScriptTokenType::None;
		newDataMember.PrimitiveType.Value = "None";

		newFunctionMember.Name = { ScriptTokenType::Identifier, "SetTranslation" };
		newFunctionMember.Namespace = {};
		newFunctionMember.ReturnType = { ScriptTokenType::None, "None" };
		newFunctionMember.Description = "This function sets the position for the selected entity. This function takes a vector3 to denote the entity's new x, y, and z position.";
		newMemberParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "vector3" });
		newMemberParameter.Identifier = { ScriptTokenType::Identifier, "newPosition" };
		newFunctionMember.Parameters.push_back(newMemberParameter);
		newFunctionMember.OnGenerateGetter = [](ScriptOutputGenerator& generator, MemberNode& member)
		{
			FunctionCallNode* funcCall = std::get_if<FunctionCallNode>(&member.ChildMemberNode->ChildMemberNode->CurrentNodeExpression->Value);
			
			generator.m_OutputText << "TransformComponent_SetTranslation(";
			generator.GenerateExpression(member.CurrentNodeExpression);
			generator.m_OutputText << ", ";
			generator.GenerateExpression(funcCall->Arguments.at(0));
			generator.m_OutputText << ")";
		};
		newMemberParameter = {};
		newDataMember.Members.insert_or_assign(newFunctionMember.Name.Value, CreateRef<MemberType>(newFunctionMember));
		newFunctionMember = {};

		newFunctionMember.Name = { ScriptTokenType::Identifier, "GetTranslation" };
		newFunctionMember.Namespace = {};
		newFunctionMember.ReturnType = { ScriptTokenType::PrimitiveType, "vector3" };
		newFunctionMember.Description = "This function gets the position for the selected entity. This function takes no parameters and returns a vector3.";
		newFunctionMember.OnGenerateGetter = [](ScriptOutputGenerator& generator, MemberNode& member)
		{
			generator.m_OutputText << "TransformComponent_GetTranslation(";
			generator.GenerateExpression(member.CurrentNodeExpression);
			generator.m_OutputText << ")";
		};
		newDataMember.Members.insert_or_assign(newFunctionMember.Name.Value, CreateRef<MemberType>(newFunctionMember));
		newFunctionMember = {};

		newPrimitiveType.Members.insert_or_assign(newDataMember.Name, CreateRef<MemberType>(newDataMember));
		newDataMember = {};

		newDataMember.Name = "RigidBody";
		newDataMember.Description = "This entity member is a rigid body component. This component provides an interface to interact with the 2D physics body associated with this entity.";
		newDataMember.PrimitiveType.Type = ScriptTokenType::None;
		newDataMember.PrimitiveType.Value = "None";

		newFunctionMember.Name = { ScriptTokenType::Identifier, "SetLinearVelocity" };
		newFunctionMember.Namespace = {};
		newFunctionMember.ReturnType = { ScriptTokenType::None, "None" };
		newFunctionMember.Description = "This function sets the current linear velocity of the 2D physics object associated with this entity. This function takes a vector2 to denote the entity's new x and y velocity.";
		newMemberParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "vector2" });
		newMemberParameter.Identifier = { ScriptTokenType::Identifier, "newVelocity" };
		newFunctionMember.Parameters.push_back(newMemberParameter);
		newFunctionMember.OnGenerateGetter = [](ScriptOutputGenerator& generator, MemberNode& member)
		{
			FunctionCallNode* funcCall = std::get_if<FunctionCallNode>(&member.ChildMemberNode->ChildMemberNode->CurrentNodeExpression->Value);

			generator.m_OutputText << "Rigidbody2DComponent_SetLinearVelocity(";
			generator.GenerateExpression(member.CurrentNodeExpression);
			generator.m_OutputText << ", ";
			generator.GenerateExpression(funcCall->Arguments.at(0));
			generator.m_OutputText << ")";
		};
		newMemberParameter = {};
		newDataMember.Members.insert_or_assign(newFunctionMember.Name.Value, CreateRef<MemberType>(newFunctionMember));
		newFunctionMember = {};

		newFunctionMember.Name = { ScriptTokenType::Identifier, "GetLinearVelocity" };
		newFunctionMember.Namespace = {};
		newFunctionMember.ReturnType = { ScriptTokenType::PrimitiveType, "vector2" };
		newFunctionMember.Description = "This function gets the current linear velocity of the 2D physics object associated with this entity.";
		newFunctionMember.OnGenerateGetter = [](ScriptOutputGenerator& generator, MemberNode& member)
		{
			FunctionCallNode* funcCall = std::get_if<FunctionCallNode>(&member.ChildMemberNode->ChildMemberNode->CurrentNodeExpression->Value);

			generator.m_OutputText << "Rigidbody2DComponent_GetLinearVelocity(";
			generator.GenerateExpression(member.CurrentNodeExpression);
			generator.m_OutputText << ")";
		};
		newDataMember.Members.insert_or_assign(newFunctionMember.Name.Value, CreateRef<MemberType>(newFunctionMember));
		newFunctionMember = {};

		newPrimitiveType.Members.insert_or_assign(newDataMember.Name, CreateRef<MemberType>(newDataMember));
		newDataMember = {};

		newDataMember.Name = "Tag";
		newDataMember.Description = "This entity member is a tag component. This component provides a name that can be used to identify this entity. Note that tags are not necessarily unique!";
		newDataMember.PrimitiveType.Type = ScriptTokenType::None;
		newDataMember.PrimitiveType.Value = "None";
		newFunctionMember.Name = { ScriptTokenType::Identifier, "GetTag" };
		newFunctionMember.Namespace = {};
		newFunctionMember.ReturnType = { ScriptTokenType::PrimitiveType, "string" };
		newFunctionMember.Description = "This functions gets the current tag of the entity as a string.";
		newFunctionMember.OnGenerateGetter = [](ScriptOutputGenerator& generator, MemberNode& member)
		{
			FunctionCallNode* funcCall = std::get_if<FunctionCallNode>(&member.ChildMemberNode->ChildMemberNode->CurrentNodeExpression->Value);

			generator.m_OutputText << "TagComponent_GetTag(";
			generator.GenerateExpression(member.CurrentNodeExpression);
			generator.m_OutputText << ")";
		};
		newMemberParameter = {};
		newDataMember.Members.insert_or_assign(newFunctionMember.Name.Value, CreateRef<MemberType>(newFunctionMember));
		newFunctionMember = {};
		newPrimitiveType.Members.insert_or_assign(newDataMember.Name, CreateRef<MemberType>(newDataMember));
		newDataMember = {};

		s_ActiveLanguageDefinition.PrimitiveTypes.insert_or_assign(newPrimitiveType.Name, newPrimitiveType);

		newPrimitiveType = {};
		newPrimitiveType.Name = "float";
		newPrimitiveType.Description = "A decimal number that is 32 bits wide";
		newPrimitiveType.AcceptableLiteral = ScriptTokenType::FloatLiteral;
		newPrimitiveType.EmittedDeclaration = "float";
		newPrimitiveType.EmittedParameter = "float";
		newPrimitiveType.Icon = EditorUI::EditorUIService::s_IconDecimal;
		s_ActiveLanguageDefinition.PrimitiveTypes.insert_or_assign(newPrimitiveType.Name, newPrimitiveType);

		newPrimitiveType = {};
		newPrimitiveType.Name = "vector2";
		newPrimitiveType.Description = "A series of two contiguous floats (32 bit decimal numbers)";
		newPrimitiveType.AcceptableLiteral = ScriptTokenType::None;
		newPrimitiveType.EmittedDeclaration = "Math::vec2";
		newPrimitiveType.EmittedParameter = "Math::vec2";
		newPrimitiveType.Icon = EditorUI::EditorUIService::s_IconDecimal;
		newDataMember.Name = "x";
		dataMemberPrimitiveType.Type = ScriptTokenType::PrimitiveType;
		dataMemberPrimitiveType.Value = "float";
		newDataMember.PrimitiveType = dataMemberPrimitiveType;
		newPrimitiveType.Members.insert_or_assign(newDataMember.Name, CreateRef<MemberType>(newDataMember));
		newDataMember = {};
		dataMemberPrimitiveType = {};
		newDataMember.Name = "y";
		dataMemberPrimitiveType.Type = ScriptTokenType::PrimitiveType;
		dataMemberPrimitiveType.Value = "float";
		newDataMember.PrimitiveType = dataMemberPrimitiveType;
		newPrimitiveType.Members.insert_or_assign(newDataMember.Name, CreateRef<MemberType>(newDataMember));
		newDataMember = {};
		dataMemberPrimitiveType = {};
		newPrimitiveType.AcceptableArithmetic.insert("float");
		s_ActiveLanguageDefinition.PrimitiveTypes.insert_or_assign(newPrimitiveType.Name, newPrimitiveType);

		newPrimitiveType = {};
		newDataMember = {};
		dataMemberPrimitiveType = {};
		newPrimitiveType.Name = "vector3";
		newPrimitiveType.Description = "A series of three contiguous floats (32 bit decimal numbers)";
		newPrimitiveType.AcceptableLiteral = ScriptTokenType::None;
		newPrimitiveType.EmittedDeclaration = "Math::vec3";
		newPrimitiveType.EmittedParameter = "Math::vec3";
		newPrimitiveType.Icon = EditorUI::EditorUIService::s_IconDecimal;
		newDataMember.Name = "x";
		dataMemberPrimitiveType.Type = ScriptTokenType::PrimitiveType;
		dataMemberPrimitiveType.Value = "float";
		newDataMember.PrimitiveType = dataMemberPrimitiveType;
		newPrimitiveType.Members.insert_or_assign(newDataMember.Name, CreateRef<MemberType>(newDataMember));
		newDataMember = {};
		dataMemberPrimitiveType = {};
		newDataMember.Name = "y";
		dataMemberPrimitiveType.Type = ScriptTokenType::PrimitiveType;
		dataMemberPrimitiveType.Value = "float";
		newDataMember.PrimitiveType = dataMemberPrimitiveType;
		newPrimitiveType.Members.insert_or_assign(newDataMember.Name, CreateRef<MemberType>(newDataMember));
		newDataMember = {};
		dataMemberPrimitiveType = {};
		newDataMember.Name = "z";
		dataMemberPrimitiveType.Type = ScriptTokenType::PrimitiveType;
		dataMemberPrimitiveType.Value = "float";
		newDataMember.PrimitiveType = dataMemberPrimitiveType;
		newPrimitiveType.Members.insert_or_assign(newDataMember.Name, CreateRef<MemberType>(newDataMember));
		newDataMember = {};
		dataMemberPrimitiveType = {};
		newPrimitiveType.AcceptableArithmetic.insert("float");
		s_ActiveLanguageDefinition.PrimitiveTypes.insert_or_assign(newPrimitiveType.Name, newPrimitiveType);

		newPrimitiveType = {};
		newDataMember = {};
		dataMemberPrimitiveType = {};
		newPrimitiveType.Name = "vector4";
		newPrimitiveType.Description = "A series of four contiguous floats (32 bit decimal numbers)";
		newPrimitiveType.AcceptableLiteral = ScriptTokenType::None;
		newPrimitiveType.EmittedDeclaration = "Math::vec4";
		newPrimitiveType.EmittedParameter = "Math::vec4";
		newPrimitiveType.Icon = EditorUI::EditorUIService::s_IconDecimal;
		newPrimitiveType.AcceptableArithmetic.insert("float");
		s_ActiveLanguageDefinition.PrimitiveTypes.insert_or_assign(newPrimitiveType.Name, newPrimitiveType);
	}

	void ScriptCompilerService::CreateKGScriptNamespaces()
	{
		// Add namespace descriptions
		s_ActiveLanguageDefinition.NamespaceDescriptions.insert_or_assign("UI", "This namespace provides functions that can manage and interact with the active user interface.");
		s_ActiveLanguageDefinition.NamespaceDescriptions.insert_or_assign("GameState", "This namespace provides functions that can manage and interact with the active game state");
		s_ActiveLanguageDefinition.NamespaceDescriptions.insert_or_assign("Scenes", "This namespace provides functions that can manage the active scene.");
		s_ActiveLanguageDefinition.NamespaceDescriptions.insert_or_assign("Input", "This namespace provides functions allow access to the current input state and manage the current input mode/mapping");
		s_ActiveLanguageDefinition.NamespaceDescriptions.insert_or_assign("Audio", "This namespace provides functions that can manage audio files and play audio");
		s_ActiveLanguageDefinition.NamespaceDescriptions.insert_or_assign("Math", "This namespace provides various math functions to be used.");
		s_ActiveLanguageDefinition.NamespaceDescriptions.insert_or_assign("Network", "This namespace provides function that interact with the active network connection between the current client and the server.");
		s_ActiveLanguageDefinition.NamespaceDescriptions.insert_or_assign("Scripts", "This namespace provides access to all available scripts in the current project.");
	}

	void ScriptCompilerService::CreateKGScriptFunctionDefinitions()
	{
		// Add function declarations
		FunctionNode newFunctionNode{};
		FunctionParameter newParameter{};

		newFunctionNode.Name = { ScriptTokenType::Identifier, "str" };
		newFunctionNode.ReturnType = { ScriptTokenType::PrimitiveType, "string" };
		newParameter.AllTypes = s_AllLiteralsWithoutString;
		newParameter.Identifier = { ScriptTokenType::Identifier, "text" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newFunctionNode.Description = "Convert basic variable types into a string. Ex: 23 -> \"23\", false -> \"false\"";
		newFunctionNode.OnGenerateFunction = [](FunctionCallNode& node)
		{
			node.Identifier.Value = "std::to_string";
		};

		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);

		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Name = { ScriptTokenType::Identifier, "log" };
		newFunctionNode.ReturnType = { ScriptTokenType::None, "None" };
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "string" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "logText" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newFunctionNode.Description = "Debug logger function. This function prints the provided text to the engine's console output.";
		newFunctionNode.OnGenerateFunction = [](FunctionCallNode& node)
		{
			node.Identifier.Value = "Log";
		};

		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);

		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "UI" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "SetWidgetText" };
		newFunctionNode.ReturnType = { ScriptTokenType::None, "None" };
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "string" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "windowName" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "string" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "widgetName" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "string" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "text" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newFunctionNode.Description = "Change the displayed text of a TextWidget in the active user interface. This function takes the window tag, widget tag, and new text as arguments.";
		newFunctionNode.OnGenerateFunction = [](FunctionCallNode& node)
		{
			node.Namespace = {};
		};

		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);

		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "UI" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "LoadUI" };
		newFunctionNode.ReturnType = { ScriptTokenType::None, "None" };
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "string" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "uiName" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newFunctionNode.Description = "Change the currently open user interface. This function takes the name of the new user inteface as an argument.";
		newFunctionNode.OnGenerateFunction = [](FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "LoadUserInterfaceFromName";
		};

		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);

		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "UI" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "DisplayWindow" };
		newFunctionNode.ReturnType = { ScriptTokenType::None, "None" };
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "string" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "windowName" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "bool" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "setDisplay" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newFunctionNode.Description = "Set whether a specified window is displayed in the active user interface. This function takes the name of the window to modify as a string and a boolean representing the display option as arguments.";
		newFunctionNode.OnGenerateFunction = [](FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "SetDisplayWindow";
		};

		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);

		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "UI" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "WidgetSelectable" };
		newFunctionNode.ReturnType = { ScriptTokenType::None, "None" };
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "string" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "windowName" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "string" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "widgetName" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "bool" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "isSelectable" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newFunctionNode.Description = "Set whether a specified widget is selectable using the keyboard/mouse/etc navigation. This function takes the name of the window the widget exists inside, the name of the widget to modify, and a boolean which sets the selectable state of the widget as arguments.";
		newFunctionNode.OnGenerateFunction = [](FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "SetWidgetSelectable";
		};
		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "UI" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "IsWidgetSelected" };
		newFunctionNode.ReturnType = { ScriptTokenType::PrimitiveType, "bool" };
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "string" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "windowName" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "string" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "widgetName" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newFunctionNode.Description = "Check if the indicated widget is currently selected in the active UI. This function takes the name of the window the widget exists inside and the name of the widget itself.";
		newFunctionNode.OnGenerateFunction = [](FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "RuntimeUI_IsWidgetSelected";
		};
		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "UI" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "WidgetTextColor" };
		newFunctionNode.ReturnType = { ScriptTokenType::None, "None" };
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "string" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "windowName" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "string" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "widgetName" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "vector4" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "newColor" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newFunctionNode.Description = "Set the color of the text in a TextWidget. This function takes the name of the window the widget exists inside, the name of the widget to modify, and a vector4 to denote the new color using RGBA floats.";
		newFunctionNode.OnGenerateFunction = [](FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "SetWidgetTextColor";
		};

		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);

		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "UI" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "WidgetBackgroundColor" };
		newFunctionNode.ReturnType = { ScriptTokenType::None, "None" };
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "string" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "windowName" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "string" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "widgetName" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "vector4" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "newColor" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newFunctionNode.Description = "Set the background color of the provided widget. This function takes the name of the window the widget exists inside, the name of the widget to modify, and a vector4 to denote the new color using RGBA floats.";
		newFunctionNode.OnGenerateFunction = [](FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "SetWidgetBackgroundColor";
		};
		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "UI" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "SetSelectedWidget" };
		newFunctionNode.ReturnType = { ScriptTokenType::None, "None" };
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "string" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "windowName" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "string" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "widgetName" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newFunctionNode.Description = "Set the provided widget as selected in the current in-game user interface. This function takes the name of the window and the name of the widget as parameters.";
		newFunctionNode.OnGenerateFunction = [](FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "SetSelectedWidget";
		};
		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "GameState" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "SetField" };
		newFunctionNode.ReturnType = { ScriptTokenType::None, "None" };
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "string" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "fieldName" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newParameter.AllTypes = s_AllLiterals;
		newParameter.Identifier = { ScriptTokenType::Identifier, "fieldValue" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newFunctionNode.Description = "Set the specified field in the active Game State. This function requires the name of the field and the value it will be set to.";
		newFunctionNode.OnGenerateFunction = [](FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "SetGameStateField";

			if (TokenExpressionNode* tokenExpression = std::get_if<TokenExpressionNode>(&node.Arguments.at(1)->Value))
			{
				if (tokenExpression->Value.Type == ScriptTokenType::Identifier)
				{
					tokenExpression->Value.Value = "&" + tokenExpression->Value.Value;
				}
				else if (IsLiteral(tokenExpression->Value))
				{
					tokenExpression->Value.Value = "(void*)&RValueToLValue(" + tokenExpression->Value.Value + ")";
				}
				else
				{
					KG_WARN("Invalid argument type provided to GameState::SetField");
					return;
				}
			}
			else
			{
				node.Arguments.at(1)->GenerationAffixes = CreateRef<ExpressionGenerationAffixes>("(void*)&RValueToLValue(", ")");
			}
		};
		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "GameState" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "GetFieldUInt16" };
		newFunctionNode.ReturnType = { ScriptTokenType::PrimitiveType, "uint16" };
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "string" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "fieldName" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newFunctionNode.Description = "Get the specified field in the active Game State and returns it as a uint16. This function requires the name of the field as a parameter.";
		newFunctionNode.OnGenerateFunction = [](FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "*(uint16_t*)GetGameStateField";
		};
		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "Scenes" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "LoadScene" };
		newFunctionNode.ReturnType = { ScriptTokenType::None, "None" };
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "string" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "sceneName" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newFunctionNode.Description = "Change the active scene to the scene specified. This function takes the name of the scene that should be transitioned towards as an argument.";
		newFunctionNode.OnGenerateFunction = [](FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "TransitionSceneFromName";

		};

		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);

		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "Scenes" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "GetEntity" };
		newFunctionNode.ReturnType = { ScriptTokenType::PrimitiveType, "entity" };
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "string" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "entityName" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newFunctionNode.Description = "Get a reference to the entity with provided tag within the current scene. Note that if multiple entities exist inside the current scene with the same tag, there is no guarentee which entity will be returned. This function takes the name inside the entity's tag component as an argument.";
		newFunctionNode.OnGenerateFunction = [](FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "FindEntityHandleByName";

		};
		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "Scenes" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "IsSceneActive" };
		newFunctionNode.ReturnType = { ScriptTokenType::PrimitiveType, "bool" };
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "string" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "sceneName" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newFunctionNode.Description = "This function indicates whether the provided scene is currently active. This function takes a string which is the local filepath to the scene and returns a bool.";
		newFunctionNode.OnGenerateFunction = [](FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "Scenes_IsSceneActive";

		};
		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "Input" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "LoadInputMode" };
		newFunctionNode.ReturnType = { ScriptTokenType::None, "None" };
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "string" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "inputModeName" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newFunctionNode.Description = "Change the active input mapping/mode. The input mode maps user input to functionality/scripts. This function takes the name of the new input mode as an argument.";
		newFunctionNode.OnGenerateFunction = [](FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "LoadInputModeByName";

		};
		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "Input" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "IsKeyPressed" };
		newFunctionNode.ReturnType = { ScriptTokenType::PrimitiveType, "bool" };
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "keycode" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "queryKey" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newFunctionNode.Description = "Check if the provided key is current pressed on the keyboard. This function takes a keycode as a parameter.";
		newFunctionNode.OnGenerateFunction = [](FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "IsKeyPressed";
		};
		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "Audio" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "PlaySound" };
		newFunctionNode.ReturnType = { ScriptTokenType::None, "None" };
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "string" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "soundName" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newFunctionNode.Description = "Play a sound file. This function call is intended for short sound segments that play as mono. This function takes the name of the sound file as an argument.";
		newFunctionNode.OnGenerateFunction = [](FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "PlaySoundFromName";

		};

		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);

		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "Audio" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "PlayMusic" };
		newFunctionNode.ReturnType = { ScriptTokenType::None, "None" };
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "string" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "musicName" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newFunctionNode.Description = "Play a music file. This function call is intended to play a single song file, preferebly in stereo. This function takes the name of the sound file as an argument.";
		newFunctionNode.OnGenerateFunction = [](FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "PlayStereoSoundFromName";

		};

		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);

		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "Random" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "GetInteger" };
		newFunctionNode.ReturnType = { ScriptTokenType::PrimitiveType, "int32" };
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "int32" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "lowerBound" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "int32" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "upperBound" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newFunctionNode.Description = "Generate a random integer between the provided lower and upper bounds. Note that this function uses a fairly expensive algorithm for generating numbers compared to pseudorandom number generators. This function takes two integers to denote the lower and upper bounds respectively.";
		newFunctionNode.OnGenerateFunction = [](FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "GenerateRandomNumber";

		};
		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "Math" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "NormalizeVector2" };
		newFunctionNode.ReturnType = { ScriptTokenType::PrimitiveType, "vector2" };
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "vector2" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "vectorToNormalize" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newFunctionNode.Description = "Normalize a provided vector. This function ensures that the magnitude (length) of the vector is 1. The ratios between component vectors (x,y,z) remain unchanged. This function takes in a single vector2.";
		newFunctionNode.OnGenerateFunction = [](FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "glm::normalize";

		};
		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "Math" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "MaxUInt16" };
		newFunctionNode.ReturnType = { ScriptTokenType::PrimitiveType, "uint16" };
		newFunctionNode.Description = "This function returns the maximum value for a uint16.";
		newFunctionNode.OnGenerateFunction = [](FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "std::numeric_limits<uint16_t>().max";
		};
		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "Network" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "GetActiveSessionSlot" };
		newFunctionNode.ReturnType = { ScriptTokenType::PrimitiveType, "uint16" };
		newFunctionNode.Description = "This function retreives the slot of the current client inside the active application session. This function takes no parameters.";
		newFunctionNode.OnGenerateFunction = [](FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "GetActiveSessionSlot";

		};
		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "Network" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "PushAllEntityLocation" };
		newFunctionNode.ReturnType = { ScriptTokenType::None, "" };
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "entity" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "providedEntity" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "vector3" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "providedLocation" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newFunctionNode.Description = "This function sends the provided location to the server and all other clients in the current session for the provided entity. This function takes an entity and a vector3 as parameters.";
		newFunctionNode.OnGenerateFunction = [](FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "SendAllEntityLocation";

		};
		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "Network" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "RequestJoinSession" };
		newFunctionNode.ReturnType = { ScriptTokenType::None, "" };
		newFunctionNode.Description = "This function sends a request message to the server to join a session. This function takes no parameters.";
		newFunctionNode.OnGenerateFunction = [](FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "RequestJoinSession";

		};
		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "Network" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "LeaveCurrentSession" };
		newFunctionNode.ReturnType = { ScriptTokenType::None, "" };
		newFunctionNode.Description = "This function sends a message to the server to leave the active session. This function takes no parameters.";
		newFunctionNode.OnGenerateFunction = [](FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "LeaveCurrentSession";
		};
		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "Network" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "SendAllEntityPhysics" };
		newFunctionNode.ReturnType = { ScriptTokenType::None, "" };
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "entity" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "providedEntity" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "vector3" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "newLocation" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "vector2" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "newLinearVelocity" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newFunctionNode.Description = "This functions sends all other clients on the network new translation and linear velocity for the provided entity. This function takes an entity to update, a new translation as a vector3, and a new linear velocity as a vector2.";
		newFunctionNode.OnGenerateFunction = [](FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "SendAllEntityPhysics";

		};
		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "Network" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "SignalAll" };
		newFunctionNode.ReturnType = { ScriptTokenType::None, "" };
		newParameter.AllTypes = s_AllIntegerTypes;
		newParameter.Identifier = { ScriptTokenType::Identifier, "signal" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newFunctionNode.Description = "This function sends a signal to the server, which is then sent to all other clients. This signal can be interpretted in different ways depending on your application's needs.";
		newFunctionNode.OnGenerateFunction = [](FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "SignalAll";

		};
		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "Network" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "RequestUserCount" };
		newFunctionNode.ReturnType = { ScriptTokenType::None, "" };
		newFunctionNode.Description = "This function sends a request to the server to find out the number of users currently online. Note that a function needs to be set up to receive the user count request.";
		newFunctionNode.OnGenerateFunction = [](FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "RequestUserCount";
		};
		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "Network" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "EnableReadyCheck" };
		newFunctionNode.ReturnType = { ScriptTokenType::None, "" };
		newFunctionNode.Description = "This function sends a signal to the server indicating this client is ready. This can be intepretted differently depending on your application.";
		newFunctionNode.OnGenerateFunction = [](FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "EnableReadyCheck";

		};
		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
		newFunctionNode = {};
		newParameter = {};

		for (auto& [handle, script] : Assets::AssetService::GetScriptCache())
		{
			if (script->m_ScriptType == ScriptType::Engine)
			{
				continue;
			}
			newFunctionNode.Namespace = { ScriptTokenType::Identifier, "Scripts" };
			newFunctionNode.Name = { ScriptTokenType::Identifier, script->m_ScriptName };
			newFunctionNode.ReturnType = Utility::WrappedVarTypeToPrimitiveType(Utility::WrappedFuncTypeToReturnType(script->m_FuncType));

			for (auto parameter : Utility::WrappedFuncTypeToParameterTypes(script->m_FuncType))
			{
				ScriptToken currentToken = Utility::WrappedVarTypeToPrimitiveType(parameter);
				if (currentToken.Value == "uint64")
				{
					newParameter.AllTypes.push_back(currentToken);
					newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "entity" });
				}
				else
				{
					newParameter.AllTypes.push_back(currentToken);
				}
				newParameter.Identifier = { ScriptTokenType::Identifier, "emptyName" };
				newFunctionNode.Parameters.push_back(newParameter);
				newParameter = {};
			}

			newFunctionNode.Description = "";
			newFunctionNode.OnGenerateFunction = [](FunctionCallNode& node)
			{
				node.Namespace = {};
			};
			s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
			newFunctionNode = {};
			newParameter = {};
		}
	}

	bool ScriptCompilerService::IsLiteralOrIdentifier(ScriptToken token)
	{
		if (IsLiteral(token) || token.Type == ScriptTokenType::Identifier)
		{
			return true;
		}
		return false;
	}

	bool ScriptCompilerService::IsLiteral(ScriptToken token)
	{
		if (token.Type == ScriptTokenType::IntegerLiteral ||
			token.Type == ScriptTokenType::StringLiteral ||
			token.Type == ScriptTokenType::BooleanLiteral ||
			token.Type == ScriptTokenType::FloatLiteral ||
			token.Type == ScriptTokenType::InputKeyLiteral ||
			token.Type == ScriptTokenType::MessageTypeLiteral)
		{
			return true;
		}
		return false;
	}

	bool ScriptCompilerService::IsUnaryOperator(ScriptToken token)
	{
		if (token.Type == ScriptTokenType::SubtractionOperator ||
			token.Type == ScriptTokenType::NegationOperator)
		{
			return true;
		}
		return false;
	}

	bool ScriptCompilerService::IsBinaryOperator(ScriptToken token)
	{
		switch (token.Type)
		{
		case ScriptTokenType::AdditionOperator:
		case ScriptTokenType::SubtractionOperator:
		case ScriptTokenType::MultiplicationOperator:
		case ScriptTokenType::DivisionOperator:
		case ScriptTokenType::EqualToOperator:
		case ScriptTokenType::NotEqualToOperator:
		case ScriptTokenType::GreaterThan:
		case ScriptTokenType::GreaterThanOrEqual:
		case ScriptTokenType::LessThan:
		case ScriptTokenType::LessThanOrEqual:
			return true;
		default:
			return false;
		}
	}

	bool ScriptCompilerService::IsAdditionOrSubtractionOperator(ScriptToken token)
	{
		if (token.Type == ScriptTokenType::AdditionOperator ||
			token.Type == ScriptTokenType::SubtractionOperator)
		{
			return true;
		}
		return false;
	}

	bool ScriptCompilerService::IsMultiplicationOrDivisionOperator(ScriptToken token)
	{
		if (token.Type == ScriptTokenType::MultiplicationOperator ||
			token.Type == ScriptTokenType::DivisionOperator)
		{
			return true;
		}
		return false;
	}

	bool ScriptCompilerService::IsComparisonOperator(ScriptToken token)
	{
		switch (token.Type)
		{
		case ScriptTokenType::EqualToOperator:
		case ScriptTokenType::NotEqualToOperator:
		case ScriptTokenType::GreaterThan:
		case ScriptTokenType::GreaterThanOrEqual:
		case ScriptTokenType::LessThan:
		case ScriptTokenType::LessThanOrEqual:
			return true;
		default:
			return false;
		}
	}
	bool ScriptCompilerService::IsBooleanOperator(ScriptToken token)
	{
		switch (token.Type)
		{
		case ScriptTokenType::AndOperator:
		case ScriptTokenType::OrOperator:
			return true;
		default:
			return false;
		}
	}
}