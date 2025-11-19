#include "kgpch.h"

#include "Modules/Scripting/ScriptCompiler.h"

#include "Modules/FileSystem/FileSystem.h"
#include "Kargono/Utility/Regex.h"
#include "Modules/EditorUI/EditorUIInclude.h"
#include "Modules/Scripting/ScriptTokenizer.h"
#include "Modules/Scripting/ScriptTokenParser.h"
#include "Modules/Scripting/ScriptOutputGenerator.h"
#include "Modules/Assets/AssetService.h"
#include "Modules/ECSInternal/CustomComponent.h"
#include "Modules/ECS/Entity.h"
#include "Kargono/ProjectData/ProjectEnum.h"
#include "Kargono/Utility/Operations.h"
#include "Kargono/Core/KeyCodes.h"
#include "Modules/RuntimeUI/RuntimeUIContext.h"
#include "Modules/Scenes/Assets/Scene.h"

namespace Kargono::Scripting
{
	void ScriptCompiler::Terminate()
	{
		m_ActiveLanguageDefinition.ResetLanguageDef();
	}

	std::string ScriptCompiler::CompileScriptFile(const std::filesystem::path& scriptLocation)
	{
		// Lazy loading KGScript language def
		if (!m_ActiveLanguageDefinition)
		{
			m_ActiveLanguageDefinition.CreateLanguageDef();
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

	std::vector<ParserError> ScriptCompiler::CheckForErrors(const std::string& text)
	{
		// Lazy loading KGScript language def
		if (!m_ActiveLanguageDefinition)
		{
			m_ActiveLanguageDefinition.CreateLanguageDef();
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

	CursorContext ScriptCompiler::FindCursorContext(const std::string& text)
	{
		// Lazy loading KGScript language def
		if (!m_ActiveLanguageDefinition)
		{
			m_ActiveLanguageDefinition.CreateLanguageDef();
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

	void SuggestionGenerator::GetSuggestionsForAfterNamespace(SuggestionList& allSuggestions, const CursorContext& context, const std::string& queryText)
	{
		// Generate suggestions for function identifiers
		for (auto& [funcName, funcNode] : i_ParentLangDef.m_FunctionDefinitions)
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
				newSuggestion.m_Icon = EditorUI::EditorUIContext::m_ScriptingIcons.m_Function;
				newSuggestion.m_ShiftValue = -1;
				allSuggestions.push_back(newSuggestion);
			}
		}


		// Generate suggestions for all assets
		for (auto& [assetType, assetTypeInfo] : i_ParentLangDef.m_AllLiteralTypes)
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

	void SuggestionGenerator::GetSuggestionsForIsParameter(SuggestionList& allSuggestions, const CursorContext& context, const std::string& queryText)
	{
		UNREFERENCED_PARAMETER(context);

		// Generate suggestions for primitive types
		for (auto& [name, primitiveType] : i_ParentLangDef.m_PrimitiveTypes)
		{
			if (Utility::Regex::GetMatchSuccess(primitiveType.Name, queryText, false))
			{
				SuggestionSpec newSuggestion;
				newSuggestion.m_Label = primitiveType.Name;
				newSuggestion.m_ReplacementText = primitiveType.Name;
				newSuggestion.m_Icon = Kargono::EditorUI::EditorUIContext::m_SceneIcons.m_Entity;
				allSuggestions.push_back(newSuggestion);
			}
		}
	}

	void SuggestionGenerator::GetSuggestionsForIsDataMember(SuggestionList& allSuggestions, const CursorContext& context, const std::string& queryText)
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
					newSuggestion.m_Icon = Kargono::EditorUI::EditorUIContext::m_SceneIcons.m_Entity;
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
					newSuggestion.m_Icon = Kargono::EditorUI::EditorUIContext::m_ScriptingIcons.m_Function;
					newSuggestion.m_ShiftValue = -1;
					allSuggestions.push_back(newSuggestion);
				}
			}
		}
	}

	void SuggestionGenerator::GetSuggestionsDefault(SuggestionList& allSuggestions, const CursorContext& context, const std::string& queryText)
	{
		// Store the return types in a set for easy checking
		std::unordered_set<std::string> returnTypes;

		// Fill return types set
		for (const ScriptToken& type : context.AllReturnTypes)
		{
			returnTypes.insert(type.Value);
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
						newSuggestion.m_Icon = Kargono::EditorUI::EditorUIContext::m_SceneIcons.m_Entity;
						allSuggestions.push_back(newSuggestion);
					}
				}
			}
		}

		// Generate suggestions for all namespaces
		for (auto& [name, primitiveType] : i_ParentLangDef.m_NamespaceDescriptions)
		{
			if (Utility::Regex::GetMatchSuccess(name, queryText, false))
			{
				SuggestionSpec newSuggestion;
				newSuggestion.m_Label = name;
				newSuggestion.m_ReplacementText = name + "::";
				newSuggestion.m_Icon = Kargono::EditorUI::EditorUIContext::m_ContentBrowserIcons.m_Directory;
				allSuggestions.push_back(newSuggestion);
			}
		}

		// Handle Functions Identifiers
		for (auto& [funcName, funcNode] : i_ParentLangDef.m_FunctionDefinitions)
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
					newSuggestion.m_Icon = EditorUI::EditorUIContext::m_ScriptingIcons.m_Function;
					newSuggestion.m_ShiftValue = -1;
					allSuggestions.push_back(newSuggestion);
				}
			}
		}

		// Generate suggestions for primitive types
		if (context.m_Flags.IsFlagSet((uint8_t)CursorFlags::AllowAllVariableTypes))
		{
			for (auto& [name, primitiveType] : i_ParentLangDef.m_PrimitiveTypes)
			{
				if (Utility::Regex::GetMatchSuccess(primitiveType.Name, queryText, false))
				{
					SuggestionSpec newSuggestion;
					newSuggestion.m_Label = primitiveType.Name;
					newSuggestion.m_ReplacementText = primitiveType.Name;
					newSuggestion.m_Icon = Kargono::EditorUI::EditorUIContext::m_SceneIcons.m_Entity;
					allSuggestions.push_back(newSuggestion);
				}
			}
		}

	}

	void SuggestionGenerator::GetSuggestionsForLiteralMember(SuggestionList& allSuggestions, const CursorContext& context, const std::string& queryText)
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



	std::vector<SuggestionSpec> SuggestionGenerator::GetSuggestions(const std::string& scriptText, const std::string& queryText)
	{
		// Lazy loading KGScript language def
		if (!i_ParentLangDef)
		{
			i_ParentLangDef.CreateLanguageDef();
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
}
