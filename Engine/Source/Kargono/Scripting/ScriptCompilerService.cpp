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
#include "Kargono/Utility/Operations.h"
#include "Kargono/Core/KeyCodes.h"
#include "Kargono/RuntimeUI/RuntimeUI.h"
#include "Kargono/Scenes/Scene.h"
#include "Kargono/ECS/Entity.h"

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
		case WrappedVarType::Entity: return { Scripting::ScriptTokenType::PrimitiveType, "entity" };
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

	void ScriptCompilerService::Terminate()
	{
		s_ActiveLanguageDefinition.Clear();
	}

	std::string ScriptCompilerService::CompileScriptFile(const std::filesystem::path& scriptLocation)
	{
		// Lazy loading KGScript language def
		if (!s_ActiveLanguageDefinition)
		{
			CreateKGScriptLanguageDefinition();
		}

		// Check for invalid input
		if (!Utility::FileSystem::PathExists(scriptLocation))
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
			//tokenParser.PrintTokens();
			//tokenParser.PrintAST();
			return {};
		}

		//tokenParser.PrintAST();

		// Generate output text
		ScriptOutputGenerator outputGenerator{};
		auto [outputSuccess, outputText] = outputGenerator.GenerateOutput(std::move(newAST));

		if (!outputSuccess)
		{
			KG_WARN("Output text generation failed");
			return {};
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
		else if (context.m_Flags.IsFlagSet((uint8_t)CursorFlags::IsLiteralMember))
		{
			GetSuggestionsForLiteralMember(allSuggestions, context, queryText);
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
				newSuggestion.m_ShiftValue = -1;
				allSuggestions.push_back(newSuggestion);
			}
		}


		// Generate suggestions for all assets
		for (auto& [assetType, assetTypeInfo] : ScriptCompilerService::s_ActiveLanguageDefinition.AllLiteralTypes)
		{
			// Ensure asset type matches namespace
			if (context.CurrentNamespace.Value != assetType)
			{
				continue;
			}

			// Generate suggestions for emitter configs
			for (auto& [assetName, handle] : assetTypeInfo.m_CustomLiteralNameToID)
			{
				if (Utility::Regex::GetMatchSuccess(assetName, queryText, false))
				{
					SuggestionSpec newSuggestion;
					newSuggestion.m_Label = assetName;
					newSuggestion.m_ReplacementText = assetName;
					newSuggestion.m_Icon = assetTypeInfo.m_LiteralIcon;
					allSuggestions.push_back(newSuggestion);
				}
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
					newSuggestion.m_ShiftValue = -1;
					allSuggestions.push_back(newSuggestion);
				}
			}
		}
	}

	void Scripting::ScriptCompilerService::GetSuggestionsDefault(std::vector<SuggestionSpec>& allSuggestions, const CursorContext& context, const std::string& queryText)
	{
		// Store the return types in a set for easy checking
		std::unordered_set<std::string> returnTypes;

		// Fill return types set
		for (const ScriptToken& type : context.AllReturnTypes)
		{
			returnTypes.insert(type.Value);
		}

		// Generate suggestions for all namespaces
		for (auto& [name, primitiveType] : ScriptCompilerService::s_ActiveLanguageDefinition.NamespaceDescriptions)
		{
			if (Utility::Regex::GetMatchSuccess(name, queryText, false))
			{
				SuggestionSpec newSuggestion;
				newSuggestion.m_Label = name;
				newSuggestion.m_ReplacementText = name + "::";
				newSuggestion.m_Icon = Kargono::EditorUI::EditorUIService::s_IconDirectory;
				allSuggestions.push_back(newSuggestion);
			}
		}

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
				bool returnTypesMatch = returnTypes.contains(variable.Type.Value);
				
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
			bool returnTypesMatch = returnTypes.contains(funcNode.ReturnType.Value);

			if (funcNode.Namespace)
			{
				continue;
			}

			if (context.m_Flags.IsFlagSet((uint8_t)CursorFlags::AllowAllVariableTypes) || returnTypesMatch)
			{
				// Decide whether to insert function
				std::string label = funcNode.Name.Value;
				if (Utility::Regex::GetMatchSuccess(label, queryText, false))
				{
					SuggestionSpec newSuggestion;
					newSuggestion.m_Label = label;
					newSuggestion.m_ReplacementText = funcNode.Name.Value + "()";
					newSuggestion.m_Icon = EditorUI::EditorUIService::s_IconFunction;
					newSuggestion.m_ShiftValue = -1;
					allSuggestions.push_back(newSuggestion);
				}
			}
		}

	}

	void ScriptCompilerService::GetSuggestionsForLiteralMember(std::vector<SuggestionSpec>& allSuggestions, const CursorContext& context, const std::string& queryText)
	{
		// Generate suggestions for all member fields
		for (auto& [name, icon] : context.LiteralMembers)
		{
			if (Utility::Regex::GetMatchSuccess(name, queryText, false))
			{
				SuggestionSpec newSuggestion;
				newSuggestion.m_Label = name;
				newSuggestion.m_ReplacementText = name;
				newSuggestion.m_Icon = icon;
				allSuggestions.push_back(newSuggestion);
			}
		}
	}



	void ScriptCompilerService::CreateKGScriptLanguageDefinition()
	{
		s_ActiveLanguageDefinition = {};

		// Load in AI Message Types from current project
		KG_ASSERT(Projects::ProjectService::GetActive());
		s_ActiveLanguageDefinition.AllMessageTypes.clear();
		for (const std::string& messageType : Projects::ProjectService::GetAllMessageTypes())
		{
			s_ActiveLanguageDefinition.AllMessageTypes.insert(messageType);
		}

		CreateKGScriptKeywords();

		CreateKGScriptInitializationPrototypes();

		CreateKGScriptPrimitiveTypes();

		CreateKGScriptCustomLiterals();

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
			"else",
			"while",
			"break",
			"continue"
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
		newPrimitiveType.AcceptableLiteral = ScriptTokenType::CustomLiteral;
		newPrimitiveType.EmittedDeclaration = "uint16_t";
		newPrimitiveType.EmittedParameter = "uint16_t";
		newPrimitiveType.Icon = EditorUI::EditorUIService::s_IconInput;
		s_ActiveLanguageDefinition.PrimitiveTypes.insert_or_assign(newPrimitiveType.Name, newPrimitiveType);

		newPrimitiveType.Name = "screen_resolution";
		newPrimitiveType.Description = "A predefined type taht contains all available screen resolutions. Ex: ScreenResolution::1920x1080 or ScreenResolution::512x512";
		newPrimitiveType.AcceptableLiteral = ScriptTokenType::CustomLiteral;
		newPrimitiveType.EmittedDeclaration = "uint16_t";
		newPrimitiveType.EmittedParameter = "uint16_t";
		newPrimitiveType.Icon = EditorUI::EditorUIService::s_IconGrid;
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
		newPrimitiveType.Name = "ai_state";
		newPrimitiveType.Description = "Reference to an AI state asset. The ai state is used to describe an individual state that an entity can be in, inside a large finite state machine context. This state describes its OnEvent() actions and how it enters/exits the state.";
		newPrimitiveType.AcceptableLiteral = ScriptTokenType::CustomLiteral;
		newPrimitiveType.EmittedDeclaration = "uint64_t";
		newPrimitiveType.EmittedParameter = "uint64_t";
		newPrimitiveType.Icon = EditorUI::EditorUIService::s_IconAI;
		s_ActiveLanguageDefinition.PrimitiveTypes.insert_or_assign(newPrimitiveType.Name, newPrimitiveType);

		newPrimitiveType = {};
		newPrimitiveType.Name = "audio_buffer";
		newPrimitiveType.Description = "Reference to an audio buffer asset. The audio buffer is typicaly loaded into the engine and referenced when it should be played by the audio system.";
		newPrimitiveType.AcceptableLiteral = ScriptTokenType::CustomLiteral;
		newPrimitiveType.EmittedDeclaration = "uint64_t";
		newPrimitiveType.EmittedParameter = "uint64_t";
		newPrimitiveType.Icon = EditorUI::EditorUIService::s_IconAudio;
		s_ActiveLanguageDefinition.PrimitiveTypes.insert_or_assign(newPrimitiveType.Name, newPrimitiveType);

		newPrimitiveType = {};
		newPrimitiveType.Name = "emitter_config";
		newPrimitiveType.Description = "Reference to an emitter config asset. The emitter config is used to describe the behavior of a particle emitter instance. The config can be used to generate a new emitter instance.";
		newPrimitiveType.AcceptableLiteral = ScriptTokenType::CustomLiteral;
		newPrimitiveType.EmittedDeclaration = "uint64_t";
		newPrimitiveType.EmittedParameter = "uint64_t";
		newPrimitiveType.Icon = EditorUI::EditorUIService::s_IconEmitterConfig;
		s_ActiveLanguageDefinition.PrimitiveTypes.insert_or_assign(newPrimitiveType.Name, newPrimitiveType);

		newPrimitiveType = {};
		newPrimitiveType.Name = "font";
		newPrimitiveType.Description = "Reference to an font asset. A font is typically loaded into the engine and referenced by in-game user interfaces.";
		newPrimitiveType.AcceptableLiteral = ScriptTokenType::CustomLiteral;
		newPrimitiveType.EmittedDeclaration = "uint64_t";
		newPrimitiveType.EmittedParameter = "uint64_t";
		newPrimitiveType.Icon = EditorUI::EditorUIService::s_IconFont;
		s_ActiveLanguageDefinition.PrimitiveTypes.insert_or_assign(newPrimitiveType.Name, newPrimitiveType);

		newPrimitiveType = {};
		newPrimitiveType.Name = "game_state";
		newPrimitiveType.Description = "Reference to a game state asset. A game state asset is a global accessible chunk of data that described the game's current state.";
		newPrimitiveType.AcceptableLiteral = ScriptTokenType::CustomLiteral;
		newPrimitiveType.EmittedDeclaration = "uint64_t";
		newPrimitiveType.EmittedParameter = "uint64_t";
		newPrimitiveType.Icon = EditorUI::EditorUIService::s_IconGameState;
		s_ActiveLanguageDefinition.PrimitiveTypes.insert_or_assign(newPrimitiveType.Name, newPrimitiveType);

		newPrimitiveType = {};
		newPrimitiveType.Name = "input_map";
		newPrimitiveType.Description = "Reference to an input map asset. An input map asset is a list of input->script bindings that describe how the user can interact with the game when it is loaded.";
		newPrimitiveType.AcceptableLiteral = ScriptTokenType::CustomLiteral;
		newPrimitiveType.EmittedDeclaration = "uint64_t";
		newPrimitiveType.EmittedParameter = "uint64_t";
		newPrimitiveType.Icon = EditorUI::EditorUIService::s_IconInput;
		s_ActiveLanguageDefinition.PrimitiveTypes.insert_or_assign(newPrimitiveType.Name, newPrimitiveType);

		newPrimitiveType = {};
		newPrimitiveType.Name = "project_component";
		newPrimitiveType.Description = "Reference to a project component asset. A project component is a chunk of data that can be associated with any entity and is specific to a particular game project.";
		newPrimitiveType.AcceptableLiteral = ScriptTokenType::CustomLiteral;
		newPrimitiveType.EmittedDeclaration = "uint64_t";
		newPrimitiveType.EmittedParameter = "uint64_t";
		newPrimitiveType.Icon = EditorUI::EditorUIService::s_IconEntity;
		s_ActiveLanguageDefinition.PrimitiveTypes.insert_or_assign(newPrimitiveType.Name, newPrimitiveType);

		newPrimitiveType = {};
		newPrimitiveType.Name = "scene";
		newPrimitiveType.Description = "Reference to a scene asset. A scene is a world that can be loaded into that contains a list of entities and other world defining data.";
		newPrimitiveType.AcceptableLiteral = ScriptTokenType::CustomLiteral;
		newPrimitiveType.EmittedDeclaration = "uint64_t";
		newPrimitiveType.EmittedParameter = "uint64_t";
		newPrimitiveType.Icon = EditorUI::EditorUIService::s_IconScene;
		s_ActiveLanguageDefinition.PrimitiveTypes.insert_or_assign(newPrimitiveType.Name, newPrimitiveType);

		newPrimitiveType = {};
		newPrimitiveType.Name = "texture_2d";
		newPrimitiveType.Description = "Reference to a 2D texture asset. A texture_2d is a 2 dimensional (width and height) image that is managed by the engine.";
		newPrimitiveType.AcceptableLiteral = ScriptTokenType::CustomLiteral;
		newPrimitiveType.EmittedDeclaration = "uint64_t";
		newPrimitiveType.EmittedParameter = "uint64_t";
		newPrimitiveType.Icon = EditorUI::EditorUIService::s_IconTexture;
		s_ActiveLanguageDefinition.PrimitiveTypes.insert_or_assign(newPrimitiveType.Name, newPrimitiveType);

		newPrimitiveType = {};
		newPrimitiveType.Name = "user_interface";
		newPrimitiveType.Description = "Reference to a user interface. A user interface is a structure holding a series of windows and widgets that can be rendered if attached to the engine's active context.";
		newPrimitiveType.AcceptableLiteral = ScriptTokenType::CustomLiteral;
		newPrimitiveType.EmittedDeclaration = "uint64_t";
		newPrimitiveType.EmittedParameter = "uint64_t";
		newPrimitiveType.Icon = EditorUI::EditorUIService::s_IconUserInterface;
		s_ActiveLanguageDefinition.PrimitiveTypes.insert_or_assign(newPrimitiveType.Name, newPrimitiveType);

		newPrimitiveType = {};
		newPrimitiveType.Name = "user_interface_window";
		newPrimitiveType.Description = "Reference to a user interface window. This object is a reference to a window that exists inside the context of a user_interface asset. You can typically obtain one of these with this syntax: UserInterfaces::userInterfaceName.window1.";
		newPrimitiveType.EmittedDeclaration = "RuntimeUI::WindowID";
		newPrimitiveType.EmittedParameter = "RuntimeUI::WindowID";
		newPrimitiveType.Icon = EditorUI::EditorUIService::s_IconWindow;
		s_ActiveLanguageDefinition.PrimitiveTypes.insert_or_assign(newPrimitiveType.Name, newPrimitiveType);

		newPrimitiveType = {};
		newPrimitiveType.Name = "text_widget";
		newPrimitiveType.Description = "Reference to a user interface text widget. This object is a reference to a text widget that exists inside the context of a user_interface asset. You can typically obtain one of these with this syntax: UserInterfaces::userInterfaceName.window1.widget1.";
		newPrimitiveType.EmittedDeclaration = "RuntimeUI::WidgetID";
		newPrimitiveType.EmittedParameter = "RuntimeUI::WidgetID";
		newPrimitiveType.Icon = EditorUI::EditorUIService::s_IconTextWidget;
		s_ActiveLanguageDefinition.PrimitiveTypes.insert_or_assign(newPrimitiveType.Name, newPrimitiveType);

		newPrimitiveType = {};
		newPrimitiveType.Name = "button_widget";
		newPrimitiveType.Description = "Reference to a user interface button widget. This object is a reference to a button widget that exists inside the context of a user_interface asset. You can typically obtain one of these with this syntax: UserInterfaces::userInterfaceName.window1.widget1.";
		newPrimitiveType.EmittedDeclaration = "RuntimeUI::WidgetID";
		newPrimitiveType.EmittedParameter = "RuntimeUI::WidgetID";
		newPrimitiveType.Icon = EditorUI::EditorUIService::s_IconButtonWidget;
		s_ActiveLanguageDefinition.PrimitiveTypes.insert_or_assign(newPrimitiveType.Name, newPrimitiveType);

		newPrimitiveType = {};
		newPrimitiveType.Name = "image_widget";
		newPrimitiveType.Description = "Reference to a user interface image widget. This object is a reference to a image widget that exists inside the context of a user_interface asset. You can typically obtain one of these with this syntax: UserInterfaces::userInterfaceName.window1.widget1.";
		newPrimitiveType.EmittedDeclaration = "RuntimeUI::WidgetID";
		newPrimitiveType.EmittedParameter = "RuntimeUI::WidgetID";
		newPrimitiveType.Icon = EditorUI::EditorUIService::s_IconTexture;
		s_ActiveLanguageDefinition.PrimitiveTypes.insert_or_assign(newPrimitiveType.Name, newPrimitiveType);

		newPrimitiveType = {};
		newPrimitiveType.Name = "image_button_widget";
		newPrimitiveType.Description = "Reference to a user interface image button widget. You can typically obtain one of these with this syntax: UserInterfaces::userInterfaceName.window1.widget1.";
		newPrimitiveType.EmittedDeclaration = "RuntimeUI::WidgetID";
		newPrimitiveType.EmittedParameter = "RuntimeUI::WidgetID";
		newPrimitiveType.Icon = EditorUI::EditorUIService::s_IconTexture;
		s_ActiveLanguageDefinition.PrimitiveTypes.insert_or_assign(newPrimitiveType.Name, newPrimitiveType);

		newPrimitiveType = {};
		newPrimitiveType.Name = "checkbox_widget";
		newPrimitiveType.Description = "Reference to a user interface checkbox widget. You can typically obtain one of these with this syntax: UserInterfaces::userInterfaceName.window1.widget1.";
		newPrimitiveType.EmittedDeclaration = "RuntimeUI::WidgetID";
		newPrimitiveType.EmittedParameter = "RuntimeUI::WidgetID";
		newPrimitiveType.Icon = EditorUI::EditorUIService::s_IconCheckbox_Enabled;
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

		newDataMember.Name = "AIState";
		newDataMember.Description = "This entity member is a AI state component. This component stores the AI's current global, current, and previous states and uses them to provide a managed state machine in the engine.";
		newDataMember.PrimitiveType.Type = ScriptTokenType::None;
		newDataMember.PrimitiveType.Value = "None";

		newFunctionMember.Name = { ScriptTokenType::Identifier, "ChangeGlobalState" };
		newFunctionMember.Namespace = {};
		newFunctionMember.ReturnType = { ScriptTokenType::None, "None" };
		newFunctionMember.Description = "This function changes this entities's global state to the indicated state. This function takes in the location of an AI state component in the current project as a parameter.";
		newMemberParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "ai_state" });
		newMemberParameter.Identifier = { ScriptTokenType::Identifier, "newAIState" };
		newFunctionMember.Parameters.push_back(newMemberParameter);
		newFunctionMember.OnGenerateGetter = [](ScriptOutputGenerator& generator, MemberNode& member)
		{
			FunctionCallNode* funcCall = std::get_if<FunctionCallNode>(&member.ChildMemberNode->ChildMemberNode->CurrentNodeExpression->Value);

			generator.m_OutputText << "AI_ChangeGlobalState(";
			generator.GenerateExpression(member.CurrentNodeExpression);
			generator.m_OutputText << ", ";
			generator.GenerateExpression(funcCall->Arguments.at(0));
			generator.m_OutputText << ")";
		};
		newMemberParameter = {};
		newDataMember.Members.insert_or_assign(newFunctionMember.Name.Value, CreateRef<MemberType>(newFunctionMember));
		newFunctionMember = {};

		newFunctionMember.Name = { ScriptTokenType::Identifier, "ChangeCurrentState" };
		newFunctionMember.Namespace = {};
		newFunctionMember.ReturnType = { ScriptTokenType::None, "None" };
		newFunctionMember.Description = "This function changes this entities's current state to the indicated state. This function takes in the location of an AI state component in the current project as a parameter.";
		newMemberParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "ai_state" });
		newMemberParameter.Identifier = { ScriptTokenType::Identifier, "newAIState" };
		newFunctionMember.Parameters.push_back(newMemberParameter);
		newFunctionMember.OnGenerateGetter = [](ScriptOutputGenerator& generator, MemberNode& member)
		{
			FunctionCallNode* funcCall = std::get_if<FunctionCallNode>(&member.ChildMemberNode->ChildMemberNode->CurrentNodeExpression->Value);

			generator.m_OutputText << "AI_ChangeCurrentState(";
			generator.GenerateExpression(member.CurrentNodeExpression);
			generator.m_OutputText << ", ";
			generator.GenerateExpression(funcCall->Arguments.at(0));
			generator.m_OutputText << ")";
		};
		newMemberParameter = {};
		newDataMember.Members.insert_or_assign(newFunctionMember.Name.Value, CreateRef<MemberType>(newFunctionMember));
		newFunctionMember = {};

		newFunctionMember.Name = { ScriptTokenType::Identifier, "IsGlobalState" };
		newFunctionMember.Namespace = {};
		newFunctionMember.ReturnType = { ScriptTokenType::PrimitiveType, "bool" };
		newFunctionMember.Description = "This function check whether the global state of this entity is identical to the indicated aistate. This function takes in the location of an AI state component in the current project as a parameter.";
		newMemberParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "ai_state" });
		newMemberParameter.Identifier = { ScriptTokenType::Identifier, "queryAIState" };
		newFunctionMember.Parameters.push_back(newMemberParameter);
		newFunctionMember.OnGenerateGetter = [](ScriptOutputGenerator& generator, MemberNode& member)
			{
				FunctionCallNode* funcCall = std::get_if<FunctionCallNode>(&member.ChildMemberNode->ChildMemberNode->CurrentNodeExpression->Value);

				generator.m_OutputText << "AI_IsGlobalState(";
				generator.GenerateExpression(member.CurrentNodeExpression);
				generator.m_OutputText << ", ";
				generator.GenerateExpression(funcCall->Arguments.at(0));
				generator.m_OutputText << ")";
			};
		newMemberParameter = {};
		newDataMember.Members.insert_or_assign(newFunctionMember.Name.Value, CreateRef<MemberType>(newFunctionMember));
		newFunctionMember = {};

		newFunctionMember.Name = { ScriptTokenType::Identifier, "IsCurrentState" };
		newFunctionMember.Namespace = {};
		newFunctionMember.ReturnType = { ScriptTokenType::PrimitiveType, "bool" };
		newFunctionMember.Description = "This function check whether the current state of this entity is identical to the indicated aistate. This function takes in the location of an AI state component in the current project as a parameter.";
		newMemberParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "ai_state" });
		newMemberParameter.Identifier = { ScriptTokenType::Identifier, "newAIState" };
		newFunctionMember.Parameters.push_back(newMemberParameter);
		newFunctionMember.OnGenerateGetter = [](ScriptOutputGenerator& generator, MemberNode& member)
		{
			FunctionCallNode* funcCall = std::get_if<FunctionCallNode>(&member.ChildMemberNode->ChildMemberNode->CurrentNodeExpression->Value);

			generator.m_OutputText << "AI_IsCurrentState(";
			generator.GenerateExpression(member.CurrentNodeExpression);
			generator.m_OutputText << ", ";
			generator.GenerateExpression(funcCall->Arguments.at(0));
			generator.m_OutputText << ")";
		};
		newMemberParameter = {};
		newDataMember.Members.insert_or_assign(newFunctionMember.Name.Value, CreateRef<MemberType>(newFunctionMember));
		newFunctionMember = {};

		newFunctionMember.Name = { ScriptTokenType::Identifier, "IsPreviousState" };
		newFunctionMember.Namespace = {};
		newFunctionMember.ReturnType = { ScriptTokenType::PrimitiveType, "bool" };
		newFunctionMember.Description = "This function check whether the previous state of this entity is identical to the indicated aistate. This function takes in the location of an AI state component in the current project as a parameter.";
		newMemberParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "ai_state" });
		newMemberParameter.Identifier = { ScriptTokenType::Identifier, "newAIState" };
		newFunctionMember.Parameters.push_back(newMemberParameter);
		newFunctionMember.OnGenerateGetter = [](ScriptOutputGenerator& generator, MemberNode& member)
			{
				FunctionCallNode* funcCall = std::get_if<FunctionCallNode>(&member.ChildMemberNode->ChildMemberNode->CurrentNodeExpression->Value);

				generator.m_OutputText << "AI_IsPreviousState(";
				generator.GenerateExpression(member.CurrentNodeExpression);
				generator.m_OutputText << ", ";
				generator.GenerateExpression(funcCall->Arguments.at(0));
				generator.m_OutputText << ")";
			};
		newMemberParameter = {};
		newDataMember.Members.insert_or_assign(newFunctionMember.Name.Value, CreateRef<MemberType>(newFunctionMember));
		newFunctionMember = {};

		newFunctionMember.Name = { ScriptTokenType::Identifier, "RevertPreviousState" };
		newFunctionMember.Namespace = {};
		newFunctionMember.ReturnType = { ScriptTokenType::None, "None" };
		newFunctionMember.Description = "This function changes this entities's current state its previous state if it exists. This function takes no parameters";
		newFunctionMember.OnGenerateGetter = [](ScriptOutputGenerator& generator, MemberNode& member)
		{
			FunctionCallNode* funcCall = std::get_if<FunctionCallNode>(&member.ChildMemberNode->ChildMemberNode->CurrentNodeExpression->Value);

			generator.m_OutputText << "AI_RevertPreviousState(";
			generator.GenerateExpression(member.CurrentNodeExpression);
			generator.m_OutputText << ")";
		};
		newMemberParameter = {};
		newDataMember.Members.insert_or_assign(newFunctionMember.Name.Value, CreateRef<MemberType>(newFunctionMember));
		newFunctionMember = {};

		newFunctionMember.Name = { ScriptTokenType::Identifier, "ClearGlobalState" };
		newFunctionMember.Namespace = {};
		newFunctionMember.ReturnType = { ScriptTokenType::None, "None" };
		newFunctionMember.Description = "This function clears the current global state of the entity's AIStateComponent. This function takes no parameters";
		newFunctionMember.OnGenerateGetter = [](ScriptOutputGenerator& generator, MemberNode& member)
		{
			FunctionCallNode* funcCall = std::get_if<FunctionCallNode>(&member.ChildMemberNode->ChildMemberNode->CurrentNodeExpression->Value);

			generator.m_OutputText << "AI_ClearGlobalState(";
			generator.GenerateExpression(member.CurrentNodeExpression);
			generator.m_OutputText << ")";
		};
		newMemberParameter = {};
		newDataMember.Members.insert_or_assign(newFunctionMember.Name.Value, CreateRef<MemberType>(newFunctionMember));
		newFunctionMember = {};

		newFunctionMember.Name = { ScriptTokenType::Identifier, "ClearCurrentState" };
		newFunctionMember.Namespace = {};
		newFunctionMember.ReturnType = { ScriptTokenType::None, "None" };
		newFunctionMember.Description = "This function clears the current current state of the entity's AIStateComponent. This function takes no parameters";
		newFunctionMember.OnGenerateGetter = [](ScriptOutputGenerator& generator, MemberNode& member)
		{
			FunctionCallNode* funcCall = std::get_if<FunctionCallNode>(&member.ChildMemberNode->ChildMemberNode->CurrentNodeExpression->Value);

			generator.m_OutputText << "AI_ClearCurrentState(";
			generator.GenerateExpression(member.CurrentNodeExpression);
			generator.m_OutputText << ")";
		};
		newMemberParameter = {};
		newDataMember.Members.insert_or_assign(newFunctionMember.Name.Value, CreateRef<MemberType>(newFunctionMember));
		newFunctionMember = {};

		newFunctionMember.Name = { ScriptTokenType::Identifier, "ClearPreviousState" };
		newFunctionMember.Namespace = {};
		newFunctionMember.ReturnType = { ScriptTokenType::None, "None" };
		newFunctionMember.Description = "This function clears the current previous state of the entity's AIStateComponent. This function takes no parameters";
		newFunctionMember.OnGenerateGetter = [](ScriptOutputGenerator& generator, MemberNode& member)
		{
			FunctionCallNode* funcCall = std::get_if<FunctionCallNode>(&member.ChildMemberNode->ChildMemberNode->CurrentNodeExpression->Value);

			generator.m_OutputText << "AI_ClearPreviousState(";
			generator.GenerateExpression(member.CurrentNodeExpression);
			generator.m_OutputText << ")";
		};
		newMemberParameter = {};
		newDataMember.Members.insert_or_assign(newFunctionMember.Name.Value, CreateRef<MemberType>(newFunctionMember));
		newFunctionMember = {};

		newFunctionMember.Name = { ScriptTokenType::Identifier, "ClearAllStates" };
		newFunctionMember.Namespace = {};
		newFunctionMember.ReturnType = { ScriptTokenType::None, "None" };
		newFunctionMember.Description = "This function clears all AI State references inside an entity's AIStateComponent. This function takes no parameters";
		newFunctionMember.OnGenerateGetter = [](ScriptOutputGenerator& generator, MemberNode& member)
		{
			FunctionCallNode* funcCall = std::get_if<FunctionCallNode>(&member.ChildMemberNode->ChildMemberNode->CurrentNodeExpression->Value);

			generator.m_OutputText << "AI_ClearAllStates(";
			generator.GenerateExpression(member.CurrentNodeExpression);
			generator.m_OutputText << ")";
		};
		newMemberParameter = {};
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


		newPrimitiveType = {};
		newPrimitiveType.Name = "raycast_result";
		newPrimitiveType.Description = "The results of a raycast that has occured in the active physics world. This result includes: a boolean called collisionSuccess, which indicates success, an entity called collisionEntity, which references the entity that was collided with, a vector2 called collisionPoint which indicates the location of the collision in 2D space, and a vector2 called collisionNormal, which indicates the physics normal of the edge where the collision occurred.";
		newPrimitiveType.AcceptableLiteral = ScriptTokenType::None;
		newPrimitiveType.EmittedDeclaration = "Physics::RaycastResult";
		newPrimitiveType.EmittedParameter = "Physics::RaycastResult";
		newPrimitiveType.Icon = EditorUI::EditorUIService::s_IconRigidBody;
		newDataMember.Name = "collisionSuccess";
		dataMemberPrimitiveType.Type = ScriptTokenType::PrimitiveType;
		dataMemberPrimitiveType.Value = "bool";
		newDataMember.PrimitiveType = dataMemberPrimitiveType;
		newDataMember.OnGenerateGetter = [](ScriptOutputGenerator& generator, MemberNode& member)
		{
			// Output identifier for raycast_result and underlying type
			generator.GenerateExpression(member.CurrentNodeExpression);
			generator.m_OutputText << ".m_Success";
		};
		newPrimitiveType.Members.insert_or_assign(newDataMember.Name, CreateRef<MemberType>(newDataMember));
		newDataMember = {};
		dataMemberPrimitiveType = {};
		newDataMember.Name = "collisionEntity";
		dataMemberPrimitiveType.Type = ScriptTokenType::PrimitiveType;
		dataMemberPrimitiveType.Value = "entity";
		newDataMember.PrimitiveType = dataMemberPrimitiveType;
		newDataMember.OnGenerateGetter = [](ScriptOutputGenerator& generator, MemberNode& member)
		{
			// Output identifier for raycast_result and underlying type
			generator.GenerateExpression(member.CurrentNodeExpression);
			generator.m_OutputText << ".m_Entity";
		};
		newPrimitiveType.Members.insert_or_assign(newDataMember.Name, CreateRef<MemberType>(newDataMember));
		newDataMember = {};
		dataMemberPrimitiveType = {};
		newDataMember.Name = "collisionPoint";
		dataMemberPrimitiveType.Type = ScriptTokenType::PrimitiveType;
		dataMemberPrimitiveType.Value = "vector2";
		newDataMember.PrimitiveType = dataMemberPrimitiveType;
		newDataMember.OnGenerateGetter = [](ScriptOutputGenerator& generator, MemberNode& member)
		{
			// Output identifier for raycast_result and underlying type
			generator.GenerateExpression(member.CurrentNodeExpression);
			generator.m_OutputText << ".m_Location";
		};
		newPrimitiveType.Members.insert_or_assign(newDataMember.Name, CreateRef<MemberType>(newDataMember));
		newDataMember = {};
		dataMemberPrimitiveType = {};
		newDataMember.Name = "collisionNormal";
		dataMemberPrimitiveType.Type = ScriptTokenType::PrimitiveType;
		dataMemberPrimitiveType.Value = "vector2";
		newDataMember.PrimitiveType = dataMemberPrimitiveType;
		newDataMember.OnGenerateGetter = [](ScriptOutputGenerator& generator, MemberNode& member)
		{
			// Output identifier for raycast_result and underlying type
			generator.GenerateExpression(member.CurrentNodeExpression);
			generator.m_OutputText << ".m_Normal";
		};
		newPrimitiveType.Members.insert_or_assign(newDataMember.Name, CreateRef<MemberType>(newDataMember));
		newDataMember = {};
		dataMemberPrimitiveType = {};
		s_ActiveLanguageDefinition.PrimitiveTypes.insert_or_assign(newPrimitiveType.Name, newPrimitiveType);
	}

	void ScriptCompilerService::CreateKGScriptCustomLiterals()
	{
		// Load all asset type declarations
		s_ActiveLanguageDefinition.AllLiteralTypes.clear();
		s_ActiveLanguageDefinition.AllLiteralTypes =
		{
			{"AIStates", {{}, EditorUI::EditorUIService::s_IconAI}},
			{"AudioBuffers", {{}, EditorUI::EditorUIService::s_IconAudio}},
			{"EmitterConfigs", {{}, EditorUI::EditorUIService::s_IconParticles}},
			{"Fonts", {{}, EditorUI::EditorUIService::s_IconFont}},
			{"GameStates", {{}, EditorUI::EditorUIService::s_IconGameState}},
			{"InputMaps", {{}, EditorUI::EditorUIService::s_IconInput}},
			{"ProjectComponents", {{}, EditorUI::EditorUIService::s_IconProjectComponent}},
			{"Scenes", {{}, EditorUI::EditorUIService::s_IconScene}},
			{"Textures", {{}, EditorUI::EditorUIService::s_IconTexture}},
			{"UserInterfaces", {{}, EditorUI::EditorUIService::s_IconUserInterface}},
			{"ScreenResolution", {{}, EditorUI::EditorUIService::s_IconGrid}},
			{"Key", {{}, EditorUI::EditorUIService::s_IconInput}}
		};

		// Load in names of all AI States
		CustomLiteralNameToIDMap& aiMap = s_ActiveLanguageDefinition.AllLiteralTypes.at("AIStates").m_CustomLiteralNameToID;
		for (auto& [configHandle, configInfo] : Assets::AssetService::GetAIStateRegistry())
		{
			CustomLiteralMember newMember;
			newMember.m_PrimitiveType = { ScriptTokenType::PrimitiveType, "ai_state"};
			newMember.m_OutputText = std::string(configHandle);

			std::string fileName = configInfo.Data.FileLocation.stem().string();
			Utility::Operations::RemoveWhitespaceFromString(fileName);
			aiMap.insert_or_assign(fileName, newMember);
		}

		// Load in names of all audio buffers
		CustomLiteralNameToIDMap& audioMap = s_ActiveLanguageDefinition.AllLiteralTypes.at("AudioBuffers").m_CustomLiteralNameToID;
		for (auto& [configHandle, configInfo] : Assets::AssetService::GetAudioBufferRegistry())
		{
			CustomLiteralMember newMember;
			newMember.m_PrimitiveType = { ScriptTokenType::PrimitiveType, "audio_buffer" };
			newMember.m_OutputText = std::string(configHandle);

			std::string fileName = configInfo.Data.FileLocation.stem().string();
			Utility::Operations::RemoveWhitespaceFromString(fileName);
			audioMap.insert_or_assign(fileName, newMember);
		}

		// Load in names of all emitter configs
		CustomLiteralNameToIDMap& emitterConfigMap = s_ActiveLanguageDefinition.AllLiteralTypes.at("EmitterConfigs").m_CustomLiteralNameToID;
		for (auto& [configHandle, configInfo] : Assets::AssetService::GetEmitterConfigRegistry())
		{
			CustomLiteralMember newMember;
			newMember.m_PrimitiveType = { ScriptTokenType::PrimitiveType, "emitter_config" };
			newMember.m_OutputText = std::string(configHandle);

			std::string fileName = configInfo.Data.FileLocation.stem().string();
			Utility::Operations::RemoveWhitespaceFromString(fileName);
			emitterConfigMap.insert_or_assign(fileName, newMember);
		}

		// Load in names of all fonts
		CustomLiteralNameToIDMap& fontMap = s_ActiveLanguageDefinition.AllLiteralTypes.at("Fonts").m_CustomLiteralNameToID;
		for (auto& [configHandle, configInfo] : Assets::AssetService::GetFontRegistry())
		{
			CustomLiteralMember newMember;
			newMember.m_PrimitiveType = { ScriptTokenType::PrimitiveType, "font" };
			newMember.m_OutputText = std::string(configHandle);

			std::string fileName = configInfo.Data.FileLocation.stem().string();
			Utility::Operations::RemoveWhitespaceFromString(fileName);
			fontMap.insert_or_assign(fileName, newMember);
		}

		// Load in names of all game states
		CustomLiteralNameToIDMap& gameStateMap = s_ActiveLanguageDefinition.AllLiteralTypes.at("GameStates").m_CustomLiteralNameToID;
		for (auto& [configHandle, configInfo] : Assets::AssetService::GetGameStateRegistry())
		{
			CustomLiteralMember newMember;
			newMember.m_PrimitiveType = { ScriptTokenType::PrimitiveType, "game_state" };
			newMember.m_OutputText = std::string(configHandle);

			std::string fileName = configInfo.Data.FileLocation.stem().string();
			Utility::Operations::RemoveWhitespaceFromString(fileName);
			gameStateMap.insert_or_assign(fileName, newMember);
		}

		// Load in names of all input map
		CustomLiteralNameToIDMap& inputMapMap = s_ActiveLanguageDefinition.AllLiteralTypes.at("InputMaps").m_CustomLiteralNameToID;
		for (auto& [configHandle, configInfo] : Assets::AssetService::GetInputMapRegistry())
		{
			CustomLiteralMember newMember;
			newMember.m_PrimitiveType = { ScriptTokenType::PrimitiveType, "input_map" };
			newMember.m_OutputText = std::string(configHandle);

			std::string fileName = configInfo.Data.FileLocation.stem().string();
			Utility::Operations::RemoveWhitespaceFromString(fileName);
			inputMapMap.insert_or_assign(fileName, newMember);
		}

		// Load in names of all project component
		CustomLiteralNameToIDMap& projectComponentMap = s_ActiveLanguageDefinition.AllLiteralTypes.at("ProjectComponents").m_CustomLiteralNameToID;
		for (auto& [configHandle, configInfo] : Assets::AssetService::GetProjectComponentRegistry())
		{
			CustomLiteralMember newMember;
			newMember.m_PrimitiveType = { ScriptTokenType::PrimitiveType, "project_component" };
			newMember.m_OutputText = std::string(configHandle);

			std::string fileName = configInfo.Data.FileLocation.stem().string();
			Utility::Operations::RemoveWhitespaceFromString(fileName);
			projectComponentMap.insert_or_assign(fileName, newMember);
		}

		// Load in names of all scene
		CustomLiteralNameToIDMap& sceneMap = s_ActiveLanguageDefinition.AllLiteralTypes.at("Scenes").m_CustomLiteralNameToID;
		for (auto& [configHandle, configInfo] : Assets::AssetService::GetSceneRegistry())
		{
			// Get the active scene
			Ref<Scenes::Scene> currentScene{ Assets::AssetService::GetScene(configHandle) };
			KG_ASSERT(currentScene);

			CustomLiteralMember newMember;
			newMember.m_PrimitiveType = { ScriptTokenType::PrimitiveType, "scene" };
			newMember.m_OutputText = std::string(configHandle);

			for (auto& [entityHandle, enttID] : currentScene->m_EntityRegistry.m_EntityMap)
			{
				// Get the actual entity reference
				ECS::Entity entityRef = currentScene->GetEntityByEnttID(enttID);

				// Get the entity's tag component
				ECS::TagComponent& currentTagComp = entityRef.GetComponent<ECS::TagComponent>();

				// Create new literal member for each entity
				Ref<CustomLiteralMember> newEntityLiteral = CreateRef<CustomLiteralMember>();
				std::string currentTag{ currentTagComp.Tag };
				Utility::Operations::RemoveWhitespaceFromString(currentTag);
				newEntityLiteral->m_OutputText = std::to_string(entityHandle);
				newEntityLiteral->m_PrimitiveType = { ScriptTokenType::PrimitiveType, "entity" };
				newMember.m_Members.insert_or_assign(currentTag, newEntityLiteral);
			}

			std::string fileName = configInfo.Data.FileLocation.stem().string();
			Utility::Operations::RemoveWhitespaceFromString(fileName);
			sceneMap.insert_or_assign(fileName, newMember);
		}

		// Load in names of all texture 2D's
		CustomLiteralNameToIDMap& texture2DMap = s_ActiveLanguageDefinition.AllLiteralTypes.at("Textures").m_CustomLiteralNameToID;
		for (auto& [configHandle, configInfo] : Assets::AssetService::GetTexture2DRegistry())
		{
			CustomLiteralMember newMember;
			newMember.m_PrimitiveType = { ScriptTokenType::PrimitiveType, "texture_2d" };
			newMember.m_OutputText = std::string(configHandle);

			std::string fileName = configInfo.Data.FileLocation.stem().string();
			Utility::Operations::RemoveWhitespaceFromString(fileName);
			texture2DMap.insert_or_assign(fileName, newMember);
		}

		// Load in names of all UserInterface
		CustomLiteralNameToIDMap& userInterfaceMap = s_ActiveLanguageDefinition.AllLiteralTypes.at("UserInterfaces").m_CustomLiteralNameToID;
		for (auto& [configHandle, configInfo] : Assets::AssetService::GetUserInterfaceRegistry())
		{
			// Get the active user interface
			Ref<RuntimeUI::UserInterface> currentUI{ Assets::AssetService::GetUserInterface(configHandle) };
			KG_ASSERT(currentUI);

			CustomLiteralMember newMember;
			newMember.m_PrimitiveType = { ScriptTokenType::PrimitiveType, "user_interface" };
			newMember.m_OutputText = std::string(configHandle);

			size_t windowIteration{ 0 };
			for (RuntimeUI::Window& currentWindow : currentUI->m_Windows)
			{
				// Create the basic window literal
				Ref<CustomLiteralMember> newWindowLiteral = CreateRef<CustomLiteralMember>();
				std::string currentWindowLabel{ currentWindow.m_Tag };
				Utility::Operations::RemoveWhitespaceFromString(currentWindowLabel);

				// Add text replacement values for this window
				newWindowLiteral->m_OutputText = std::format("RuntimeUI::WindowID({}, {})",
					std::to_string(configHandle),
					std::to_string(windowIteration));
				newWindowLiteral->m_PrimitiveType = { ScriptTokenType::PrimitiveType, "user_interface_window" };
				
				size_t widgetIteration{ 0 };
				for (Ref<RuntimeUI::Widget> currentWidget : currentWindow.m_Widgets)
				{
					// Create the basic widget literal
					Ref<CustomLiteralMember> newWidgetLiteral = CreateRef<CustomLiteralMember>();
					std::string currentWidgetLabel{ currentWidget->m_Tag };
					Utility::Operations::RemoveWhitespaceFromString(currentWidgetLabel);

					// Add text replacement values for this widget
					newWidgetLiteral->m_OutputText = std::format("RuntimeUI::WidgetID({}, {}, {})",
						std::to_string(configHandle),
						std::to_string(windowIteration),
						std::to_string(widgetIteration));

					// Set the widget's primitive type
					switch (currentWidget->m_WidgetType)
					{
					case RuntimeUI::WidgetTypes::TextWidget:
						newWidgetLiteral->m_PrimitiveType = { ScriptTokenType::PrimitiveType, "text_widget" };
						break;
					case RuntimeUI::WidgetTypes::ButtonWidget:
						newWidgetLiteral->m_PrimitiveType = { ScriptTokenType::PrimitiveType, "button_widget" };
						break;
					case RuntimeUI::WidgetTypes::ImageWidget:
						newWidgetLiteral->m_PrimitiveType = { ScriptTokenType::PrimitiveType, "image_widget" };
						break;
					case RuntimeUI::WidgetTypes::ImageButtonWidget:
						newWidgetLiteral->m_PrimitiveType = { ScriptTokenType::PrimitiveType, "image_button_widget" };
						break;
					case RuntimeUI::WidgetTypes::CheckboxWidget:
						newWidgetLiteral->m_PrimitiveType = { ScriptTokenType::PrimitiveType, "checkbox_widget" };
						break;
					default:
						KG_ERROR("Invalid widget type provided when loading widget information into kgscript language");
						break;
					}
					
					// Add the widget literal into the window literal
					newWindowLiteral->m_Members.insert_or_assign(currentWidgetLabel, newWidgetLiteral);
					widgetIteration++;
				}

				// Add the window literal to the asset literal
				newMember.m_Members.insert_or_assign(currentWindowLabel, newWindowLiteral);
				windowIteration++;
			}

			std::string fileName = configInfo.Data.FileLocation.stem().string();
			Utility::Operations::RemoveWhitespaceFromString(fileName);
			userInterfaceMap.insert_or_assign(fileName, newMember);
		}

		// Load in names of all input keys
		CustomLiteralNameToIDMap& inputKeyMap = s_ActiveLanguageDefinition.AllLiteralTypes.at("Key").m_CustomLiteralNameToID;

		for (KeyCode code : Key::s_AllKeyCodes)
		{
			CustomLiteralMember newMember;
			newMember.m_PrimitiveType = { ScriptTokenType::PrimitiveType, "keycode" };
			newMember.m_OutputText = std::to_string(code);

			if (code == Key::None)
			{
				continue;
			}
			inputKeyMap.insert_or_assign(Utility::KeyCodeToString(code), newMember);
		}

		// Load in names of all resolutions
		CustomLiteralNameToIDMap& resolutionMap = s_ActiveLanguageDefinition.AllLiteralTypes.at("Key").m_CustomLiteralNameToID;
		for (ScreenResolution resolution : s_AllScreenResolutions)
		{
			CustomLiteralMember newMember;
			newMember.m_PrimitiveType = { ScriptTokenType::PrimitiveType, "screen_resolution" };
			newMember.m_OutputText = std::to_string((uint16_t)resolution);

			if (resolution == ScreenResolution::None)
			{
				continue;
			}
			resolutionMap.insert_or_assign(Utility::ScreenResolutionToString(resolution), newMember);
		}

	}

	void ScriptCompilerService::CreateKGScriptNamespaces()
	{
		// Add namespace descriptions
		s_ActiveLanguageDefinition.NamespaceDescriptions.insert_or_assign("UIService", "This namespace provides functions that can manage and interact with the active user interface.");
		s_ActiveLanguageDefinition.NamespaceDescriptions.insert_or_assign("GameStateService", "This namespace provides functions that can manage and interact with the active game state");
		s_ActiveLanguageDefinition.NamespaceDescriptions.insert_or_assign("SceneService", "This namespace provides functions that can manage the active scene.");
		s_ActiveLanguageDefinition.NamespaceDescriptions.insert_or_assign("InputService", "This namespace provides functions allow access to the current input state and manage the current input map/mapping");
		s_ActiveLanguageDefinition.NamespaceDescriptions.insert_or_assign("AudioService", "This namespace provides functions that can manage audio files and play audio");
		s_ActiveLanguageDefinition.NamespaceDescriptions.insert_or_assign("Math", "This namespace provides various math functions to be used.");
		s_ActiveLanguageDefinition.NamespaceDescriptions.insert_or_assign("NetworkService", "This namespace provides functions that interact with the active network connection between the current client and the server.");
		s_ActiveLanguageDefinition.NamespaceDescriptions.insert_or_assign("Scripts", "This namespace provides access to all available scripts in the current project.");
		s_ActiveLanguageDefinition.NamespaceDescriptions.insert_or_assign("AIService", "This namespace provides functions that interact with the AI system in the engine.");
		s_ActiveLanguageDefinition.NamespaceDescriptions.insert_or_assign("PhysicsService", "This namespace provides functions that interact with the physics system in the engine.");
		s_ActiveLanguageDefinition.NamespaceDescriptions.insert_or_assign("ParticleService", "This namespace provides functions that interact with the particle system in the engine.");
		s_ActiveLanguageDefinition.NamespaceDescriptions.insert_or_assign("AppService", "This namespace provides functions that interact with the application's state.");
		s_ActiveLanguageDefinition.NamespaceDescriptions.insert_or_assign("Key", "This namespace resolves to different keyboard key literals to be used with the InputService namespace.");
		s_ActiveLanguageDefinition.NamespaceDescriptions.insert_or_assign("ScreenResolution", "This namespace resolves to different screen resolution literals to be used with the AppService namespace.");

		// Add all asset types as namespaces
		for (auto& [assetType, assetNameToIDMap] : s_ActiveLanguageDefinition.AllLiteralTypes)
		{
			s_ActiveLanguageDefinition.NamespaceDescriptions.insert_or_assign(assetType, "This namespace provides access to all of the engine's " + assetType);
		}

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
		newFunctionNode.OnGenerateFunction = [](ScriptOutputGenerator& generator, FunctionCallNode& node)
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
		newFunctionNode.OnGenerateFunction = [](ScriptOutputGenerator& generator, FunctionCallNode& node)
		{
			// Ensure only a single parameter is provided to log function
			if (node.Arguments.size() != 1)
			{
				KG_WARN("Invalid parameter size inside log function");
				return;
			}

			// Prepend current script name and current line
			node.Identifier.Value = "Log";
			Ref<Expression> messageExpression = node.Arguments.at(0);
			messageExpression->GenerationAffixes = CreateRef<ExpressionGenerationAffixes>();
			messageExpression->GenerationAffixes->Prefix = "\"";
			messageExpression->GenerationAffixes->Prefix.append(generator.m_AST.m_ProgramNode.FuncNode.Name.Value);
			messageExpression->GenerationAffixes->Prefix.append(".kgscript");
			messageExpression->GenerationAffixes->Prefix.append("\"");
			messageExpression->GenerationAffixes->Prefix.append(", \"");
			messageExpression->GenerationAffixes->Prefix.append(std::to_string(node.Identifier.Line));
			messageExpression->GenerationAffixes->Prefix.append("\"");
			messageExpression->GenerationAffixes->Prefix.append(", ");
		};

		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Name = { ScriptTokenType::Identifier, "AddDebugLine" };
		newFunctionNode.ReturnType = { ScriptTokenType::None, "None" };
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "vector3" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "startPoint" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "vector3" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "endPoint" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newFunctionNode.Description = "Debug function that draws a line in the editor. This function takes the start point 3D coordinates and end the point point 3D coordinates of the line as parameters.";
		newFunctionNode.OnGenerateFunction = [](ScriptOutputGenerator& generator, FunctionCallNode& node)
		{
		};

		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Name = { ScriptTokenType::Identifier, "AddDebugPoint" };
		newFunctionNode.ReturnType = { ScriptTokenType::None, "None" };
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "vector3" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "pointLocation" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newFunctionNode.Description = "Debug function that draws a point in the editor. This function takes a vector3 that represents the 3D coordinate of the point.";
		newFunctionNode.OnGenerateFunction = [](ScriptOutputGenerator& generator, FunctionCallNode& node)
		{
		};

		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Name = { ScriptTokenType::Identifier, "ClearDebugLines" };
		newFunctionNode.ReturnType = { ScriptTokenType::None, "None" };
		newFunctionNode.Description = "Debug function clears all debug lines from the editor. This function takes no arguments.";
		newFunctionNode.OnGenerateFunction = [](ScriptOutputGenerator& generator, FunctionCallNode& node)
		{
		};
		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Name = { ScriptTokenType::Identifier, "ClearDebugPoints" };
		newFunctionNode.ReturnType = { ScriptTokenType::None, "None" };
		newFunctionNode.Description = "Debug function clears all debug points from the editor. This function takes no arguments.";
		newFunctionNode.OnGenerateFunction = [](ScriptOutputGenerator& generator, FunctionCallNode& node)
		{
		};
		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "AppService" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "ResizeApp" };
		newFunctionNode.ReturnType = { ScriptTokenType::None, "None" };
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "screen_resolution" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "newWidth" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newFunctionNode.Description = "Resize the current application viewport and window to the indicated resolution. This function a screen_resolution as a paramter such as: ScreenResolution::1920x1080";
		newFunctionNode.OnGenerateFunction = [](ScriptOutputGenerator& generator, FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "Application_Resize";
		};
		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "AppService" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "CloseApp" };
		newFunctionNode.ReturnType = { ScriptTokenType::None, "None" };
		newFunctionNode.Description = "Send a message to the engine to close the currently running application. This function terminates your application!";
		newFunctionNode.OnGenerateFunction = [](ScriptOutputGenerator& generator, FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "Application_Close";
		};
		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
		newFunctionNode = {};
		newParameter = {};


		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "AIService" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "SendMessage" };
		newFunctionNode.ReturnType = { ScriptTokenType::None, "None" };
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "message_type" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "messageType" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "entity" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "senderEntity" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "entity" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "receiverEntity" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "float" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "delayTime" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newFunctionNode.Description = "Send a message from the indicated sender entity to the indicated receiver entity. The first parameter is the message type, second parameter is the sender entity, third parameter is the receiver entity, and the final parameter is the delay as a float.";
		newFunctionNode.OnGenerateFunction = [](ScriptOutputGenerator& generator, FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "AI_SendMessage";
		};

		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "UIService" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "SetWidgetText" };
		newFunctionNode.ReturnType = { ScriptTokenType::None, "None" };
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "text_widget" });
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "button_widget" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "widget" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "string" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "text" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newFunctionNode.Description = "Change the displayed text of a TextWidget in the active user interface.";
		newFunctionNode.OnGenerateFunction = [](ScriptOutputGenerator& generator, FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "RuntimeUI_SetWidgetText";
		};
		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "UIService" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "LoadUI" };
		newFunctionNode.ReturnType = { ScriptTokenType::None, "None" };
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "user_interface" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "newUserInterface" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newFunctionNode.Description = "Change the currently open user interface. This function takes the name of the new user inteface as an argument.";
		newFunctionNode.OnGenerateFunction = [](ScriptOutputGenerator& generator, FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "RuntimeUI_LoadUserInterfaceFromHandle";
		};

		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);

		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "UIService" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "DisplayWindow" };
		newFunctionNode.ReturnType = { ScriptTokenType::None, "None" };
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "user_interface_window" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "indicatedWindow" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "bool" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "setDisplay" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newFunctionNode.Description = "Set whether a specified window is displayed in the active user interface. This function takes the name of the window to modify as a string and a boolean representing the display option as arguments.";
		newFunctionNode.OnGenerateFunction = [](ScriptOutputGenerator& generator, FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "RuntimeUI_SetDisplayWindow";
		};

		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);

		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "UIService" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "WidgetSelectable" };
		newFunctionNode.ReturnType = { ScriptTokenType::None, "None" };
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "button_widget" });
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "image_button_widget" });
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "checkbox_widget" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "widget" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "bool" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "isSelectable" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newFunctionNode.Description = "Set whether a specified widget is selectable using the keyboard/mouse/etc navigation.";
		newFunctionNode.OnGenerateFunction = [](ScriptOutputGenerator& generator, FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "RuntimeUI_SetWidgetSelectable";
		};
		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "UIService" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "IsWidgetSelected" };
		newFunctionNode.ReturnType = { ScriptTokenType::PrimitiveType, "bool" };
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "button_widget" });
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "image_button_widget" });
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "checkbox_widget" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "widget" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newFunctionNode.Description = "Check if the indicated widget is currently selected in the active UI.";
		newFunctionNode.OnGenerateFunction = [](ScriptOutputGenerator& generator, FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "RuntimeUI_IsWidgetSelected";
		};
		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "UIService" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "WidgetTextColor" };
		newFunctionNode.ReturnType = { ScriptTokenType::None, "None" };
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "text_widget" });
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "button_widget" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "widget" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "vector4" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "newColor" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newFunctionNode.Description = "Set the color of the text in a TextWidget.";
		newFunctionNode.OnGenerateFunction = [](ScriptOutputGenerator& generator, FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "RuntimeUI_SetWidgetTextColor";
		};

		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);

		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "UIService" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "WidgetBackgroundColor" };
		newFunctionNode.ReturnType = { ScriptTokenType::None, "None" };
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "button_widget" });
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "image_button_widget" });
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "checkbox_widget" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "widget" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "vector4" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "newColor" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newFunctionNode.Description = "Set the background color of the provided widget.";
		newFunctionNode.OnGenerateFunction = [](ScriptOutputGenerator& generator, FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "RuntimeUI_SetWidgetBackgroundColor";
		};
		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "UIService" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "SetSelectedWidget" };
		newFunctionNode.ReturnType = { ScriptTokenType::None, "None" };
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "button_widget" });
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "image_button_widget" });
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "checkbox_widget" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "widget" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newFunctionNode.Description = "Set the provided widget as selected in the current in-game user interface.";
		newFunctionNode.OnGenerateFunction = [](ScriptOutputGenerator& generator, FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "RuntimeUI_SetSelectedWidget";
		};
		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
		newFunctionNode = {};
		newParameter = {};
		
		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "UIService" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "SetWidgetImage" };
		newFunctionNode.ReturnType = { ScriptTokenType::None, "None" };
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "image_widget" });
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "image_button_widget" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "widget" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "texture_2d" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "newTexture" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newFunctionNode.Description = "Change the displayed texture of the indicated image widget.";
		newFunctionNode.OnGenerateFunction = [](ScriptOutputGenerator& generator, FunctionCallNode& node)
			{
				node.Namespace = {};
				node.Identifier.Value = "RuntimeUI_SetWidgetImage";
			};
		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "ParticleService" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "AddEmitter" };
		newFunctionNode.ReturnType = { ScriptTokenType::None, "None" };
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "emitter_config" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "emitterConfig" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "vector3" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "emitterPosition" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newFunctionNode.Description = "Create an instance of an emitter at the specified location. This location takes in an emitter configuration and a vector3 to denote its spawning location.";
		newFunctionNode.OnGenerateFunction = [](ScriptOutputGenerator& generator, FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "Particles_AddEmitterByHandle";
		};
		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "GameStateService" };
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
		newFunctionNode.OnGenerateFunction = [](ScriptOutputGenerator& generator, FunctionCallNode& node)
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

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "GameStateService" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "GetFieldUInt16" };
		newFunctionNode.ReturnType = { ScriptTokenType::PrimitiveType, "uint16" };
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "string" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "fieldName" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newFunctionNode.Description = "Get the specified field in the active Game State and returns it as a uint16. This function requires the name of the field as a parameter.";
		newFunctionNode.OnGenerateFunction = [](ScriptOutputGenerator& generator, FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "*(uint16_t*)GetGameStateField";
		};
		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "SceneService" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "LoadScene" };
		newFunctionNode.ReturnType = { ScriptTokenType::None, "None" };
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "scene" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "sceneName" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newFunctionNode.Description = "Change the active scene to the scene specified. This function takes the name of the scene that should be transitioned towards as an argument.";
		newFunctionNode.OnGenerateFunction = [](ScriptOutputGenerator& generator, FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "TransitionSceneFromHandle";

		};

		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);

		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "SceneService" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "GetEntity" };
		newFunctionNode.ReturnType = { ScriptTokenType::PrimitiveType, "entity" };
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "string" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "entityName" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newFunctionNode.Description = "Get a reference to the entity with provided tag within the current scene. Note that if multiple entities exist inside the current scene with the same tag, there is no guarentee which entity will be returned. This function takes the name inside the entity's tag component as an argument.";
		newFunctionNode.OnGenerateFunction = [](ScriptOutputGenerator& generator, FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "FindEntityHandleByName";

		};
		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "SceneService" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "IsSceneActive" };
		newFunctionNode.ReturnType = { ScriptTokenType::PrimitiveType, "bool" };
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "scene" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "scene" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newFunctionNode.Description = "This function indicates whether the provided scene is currently active. This function takes a string which is the local filepath to the scene and returns a bool.";
		newFunctionNode.OnGenerateFunction = [](ScriptOutputGenerator& generator, FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "Scenes_IsSceneActive";

		};
		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "InputService" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "LoadInputMap" };
		newFunctionNode.ReturnType = { ScriptTokenType::None, "None" };
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "input_map" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "inputMap" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newFunctionNode.Description = "Change the active input mapping/map. The input map maps user input to functionality/scripts. This function takes the name of the new input map as an argument.";
		newFunctionNode.OnGenerateFunction = [](ScriptOutputGenerator& generator, FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "InputMap_LoadInputMapFromHandle";

		};
		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "InputService" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "IsKeyPressed" };
		newFunctionNode.ReturnType = { ScriptTokenType::PrimitiveType, "bool" };
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "keycode" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "queryKey" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newFunctionNode.Description = "Check if the provided key is current pressed on the keyboard. This function takes a keycode as a parameter.";
		newFunctionNode.OnGenerateFunction = [](ScriptOutputGenerator& generator, FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "Input_IsKeyPressed";
		};
		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "InputService" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "IsPollingSlotPressed" };
		newFunctionNode.ReturnType = { ScriptTokenType::PrimitiveType, "bool" };
		newParameter.AllTypes = s_AllIntegerTypes;
		newParameter.Identifier = { ScriptTokenType::Identifier, "querySlot" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newFunctionNode.Description = "Check if the provided slot from the input map is current pressed on the keyboard. This function takes an integer as a parameter.";
		newFunctionNode.OnGenerateFunction = [](ScriptOutputGenerator& generator, FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "InputMap_IsPollingSlotPressed";
		};
		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "AudioService" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "PlaySound" };
		newFunctionNode.ReturnType = { ScriptTokenType::None, "None" };
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "audio_buffer" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "audioBuffer" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newFunctionNode.Description = "Play a sound file. This function call is intended for short sound segments that play as mono. This function takes the name of the sound file as an argument.";
		newFunctionNode.OnGenerateFunction = [](ScriptOutputGenerator& generator, FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "PlaySoundFromHandle";

		};

		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);

		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "AudioService" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "PlayMusic" };
		newFunctionNode.ReturnType = { ScriptTokenType::None, "None" };
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "audio_buffer" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "audioBuffer" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newFunctionNode.Description = "Play a music file. This function call is intended to play a single song file, preferebly in stereo. This function takes the name of the sound file as an argument.";
		newFunctionNode.OnGenerateFunction = [](ScriptOutputGenerator& generator, FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "PlayStereoSoundFromHandle";

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
		newFunctionNode.OnGenerateFunction = [](ScriptOutputGenerator& generator, FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "GenerateRandomInteger";

		};
		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);

		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "Random" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "GetFloat" };
		newFunctionNode.ReturnType = { ScriptTokenType::PrimitiveType, "float" };
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "float" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "lowerBound" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "float" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "upperBound" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newFunctionNode.Description = "Generate a random float between the provided lower and upper bounds. Note that this function uses a fairly expensive algorithm for generating numbers compared to pseudorandom number generators. This function takes two integers to denote the lower and upper bounds respectively.";
		newFunctionNode.OnGenerateFunction = [](ScriptOutputGenerator& generator, FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "GenerateRandomFloat";

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
		newFunctionNode.OnGenerateFunction = [](ScriptOutputGenerator& generator, FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "glm::normalize";
		};
		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "Math" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "DistanceVector2" };
		newFunctionNode.ReturnType = { ScriptTokenType::PrimitiveType, "float" };
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "vector2" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "firstVector" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "vector2" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "secondVector" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newFunctionNode.Description = "Find the distance between two vectors. This function takes in two vector2 parameters to find the distance between.";
		newFunctionNode.OnGenerateFunction = [](ScriptOutputGenerator& generator, FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "glm::distance";
		};
		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "Math" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "DistanceVector3" };
		newFunctionNode.ReturnType = { ScriptTokenType::PrimitiveType, "float" };
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "vector3" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "firstVector" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "vector3" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "secondVector" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newFunctionNode.Description = "Find the distance between two vectors. This function takes in two vector3 parameters to find the distance between.";
		newFunctionNode.OnGenerateFunction = [](ScriptOutputGenerator& generator, FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "glm::distance";
		};
		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "Math" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "MaxUInt16" };
		newFunctionNode.ReturnType = { ScriptTokenType::PrimitiveType, "uint16" };
		newFunctionNode.Description = "This function returns the maximum value for a uint16.";
		newFunctionNode.OnGenerateFunction = [](ScriptOutputGenerator& generator, FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "std::numeric_limits<uint16_t>().max";
		};
		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "NetworkService" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "GetActiveSessionSlot" };
		newFunctionNode.ReturnType = { ScriptTokenType::PrimitiveType, "uint16" };
		newFunctionNode.Description = "This function retreives the slot of the current client inside the active application session. This function takes no parameters.";
		newFunctionNode.OnGenerateFunction = [](ScriptOutputGenerator& generator, FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "GetActiveSessionSlot";

		};
		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "NetworkService" };
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
		newFunctionNode.OnGenerateFunction = [](ScriptOutputGenerator& generator, FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "SendAllEntityLocation";

		};
		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "NetworkService" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "RequestJoinSession" };
		newFunctionNode.ReturnType = { ScriptTokenType::None, "" };
		newFunctionNode.Description = "This function sends a request message to the server to join a session. This function takes no parameters.";
		newFunctionNode.OnGenerateFunction = [](ScriptOutputGenerator& generator, FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "RequestJoinSession";

		};
		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "NetworkService" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "LeaveCurrentSession" };
		newFunctionNode.ReturnType = { ScriptTokenType::None, "" };
		newFunctionNode.Description = "This function sends a message to the server to leave the active session. This function takes no parameters.";
		newFunctionNode.OnGenerateFunction = [](ScriptOutputGenerator& generator, FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "LeaveCurrentSession";
		};
		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "NetworkService" };
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
		newFunctionNode.OnGenerateFunction = [](ScriptOutputGenerator& generator, FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "SendAllEntityPhysics";

		};
		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "NetworkService" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "SignalAll" };
		newFunctionNode.ReturnType = { ScriptTokenType::None, "" };
		newParameter.AllTypes = s_AllIntegerTypes;
		newParameter.Identifier = { ScriptTokenType::Identifier, "signal" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newFunctionNode.Description = "This function sends a signal to the server, which is then sent to all other clients. This signal can be interpretted in different ways depending on your application's needs.";
		newFunctionNode.OnGenerateFunction = [](ScriptOutputGenerator& generator, FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "SignalAll";

		};
		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "NetworkService" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "RequestUserCount" };
		newFunctionNode.ReturnType = { ScriptTokenType::None, "" };
		newFunctionNode.Description = "This function sends a request to the server to find out the number of users currently online. Note that a function needs to be set up to receive the user count request.";
		newFunctionNode.OnGenerateFunction = [](ScriptOutputGenerator& generator, FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "RequestUserCount";
		};
		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "NetworkService" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "EnableReadyCheck" };
		newFunctionNode.ReturnType = { ScriptTokenType::None, "" };
		newFunctionNode.Description = "This function sends a signal to the server indicating this client is ready. This can be intepretted differently depending on your application.";
		newFunctionNode.OnGenerateFunction = [](ScriptOutputGenerator& generator, FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "EnableReadyCheck";

		};
		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);
		newFunctionNode = {};
		newParameter = {};

		newFunctionNode.Namespace = { ScriptTokenType::Identifier, "PhysicsService" };
		newFunctionNode.Name = { ScriptTokenType::Identifier, "Raycast" };
		newFunctionNode.ReturnType = { ScriptTokenType::PrimitiveType, "raycast_result" };
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "vector2" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "startPoint" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newParameter.AllTypes.push_back({ ScriptTokenType::PrimitiveType, "vector2" });
		newParameter.Identifier = { ScriptTokenType::Identifier, "endPoint" };
		newFunctionNode.Parameters.push_back(newParameter);
		newParameter = {};
		newFunctionNode.Description = "This function conducts a raycast starting from the first indicated 2D point and ends at the second indicated point. This function returns the first entity that a collision occurs with. This function takes a starting vector2 and an ending vector2 as arguments.";
		newFunctionNode.OnGenerateFunction = [](ScriptOutputGenerator& generator, FunctionCallNode& node)
		{
			node.Namespace = {};
			node.Identifier.Value = "Physics_Raycast";

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

			// Load in return type and parameters differently if using an arbitrary function
			if (script->m_FuncType == WrappedFuncType::ArbitraryFunction)
			{
				ExplicitFuncType& explicitFuncType = script->m_ExplicitFuncType;
				
				// Load in return type
				newFunctionNode.ReturnType = Utility::WrappedVarTypeToPrimitiveType(explicitFuncType.m_ReturnType);

				// Load in parameters
				std::size_t iteration{ 0 };
				bool useCustomParamNames{ explicitFuncType.m_ParameterNames.size() == explicitFuncType.m_ParameterTypes.size() };
				for (WrappedVarType paramType : explicitFuncType.m_ParameterTypes)
				{
					ScriptToken currentToken = Utility::WrappedVarTypeToPrimitiveType(paramType);
					newParameter.AllTypes.push_back(currentToken);
					FixedString32 identifier = useCustomParamNames ? explicitFuncType.m_ParameterNames.at(iteration).CString() : ("parameter" + std::to_string(iteration)).c_str();
					newParameter.Identifier = { ScriptTokenType::Identifier, identifier.CString()};
					newFunctionNode.Parameters.push_back(newParameter);
					newParameter = {};
					iteration++;
				}
			}
			else
			{
				ExplicitFuncType& explicitFuncType = script->m_ExplicitFuncType;

				// Load in return type from function type
				newFunctionNode.ReturnType = Utility::WrappedVarTypeToPrimitiveType(Utility::WrappedFuncTypeToReturnType(script->m_FuncType));


				// Load in parameters
				std::size_t iteration{ 0 };
				bool useCustomParamNames{ explicitFuncType.m_ParameterNames.size() ==
					Utility::WrappedFuncTypeToParameterTypes(script->m_FuncType).size() };
				for (WrappedVarType paramType : Utility::WrappedFuncTypeToParameterTypes(script->m_FuncType))
				{
					ScriptToken currentToken = Utility::WrappedVarTypeToPrimitiveType(paramType);
					newParameter.AllTypes.push_back(currentToken);
					FixedString32 identifier = useCustomParamNames ? explicitFuncType.m_ParameterNames.at(iteration).CString() : ("parameter" + std::to_string(iteration)).c_str();
					newParameter.Identifier = { ScriptTokenType::Identifier, identifier.CString() };
					newFunctionNode.Parameters.push_back(newParameter);
					newParameter = {};
					iteration++;
				}
			}
			
			newFunctionNode.Description = "";
			newFunctionNode.OnGenerateFunction = [](ScriptOutputGenerator& generator, FunctionCallNode& node)
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
			token.Type == ScriptTokenType::CustomLiteral ||
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
