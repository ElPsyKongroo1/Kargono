#pragma once

#include "Modules/Scripting/ScriptCompilerCommon.h"

#include <unordered_map>
#include <vector>
#include <string>

namespace Kargono::Scripting
{
	class LanguageDefinition
	{
	private:
		//==============================
		// Constructors/Destructors
		//==============================
		LanguageDefinition() = default;
		~LanguageDefinition() = default;
	public:
		//==============================
		// Create/Reset Language
		//==============================
		void CreateLanguageDef();
		void ResetLanguageDef();
	private:
		// Helper(s)
		void CreateKGScriptKeywords();
		void CreateKGScriptInitializationPrototypes();
		void CreateKGScriptPrimitiveTypes();
		void CreateKGScriptCustomLiterals();
		void CreateKGScriptNamespaces();
		void CreateKGScriptFunctionDefinitions();
	public:
		//==============================
		// Operator Overloads
		//==============================
		operator bool() const;
	public:
		//==============================
		// Getters/Setters
		//==============================
		PrimitiveType GetPrimitiveTypeFromName(const std::string& name);
	public:
		//==============================
		// Public Fields
		//==============================
		std::vector<std::string> m_Keywords{};
		std::unordered_map<std::string, PrimitiveType> m_PrimitiveTypes{};
		std::unordered_map<std::string, std::string> m_NamespaceDescriptions{};
		std::unordered_map<std::string, FunctionNode> m_FunctionDefinitions{};
		std::vector<InitializationListType> m_InitListTypes{};
		std::unordered_map<std::string, CustomLiteralInfo> m_AllLiteralTypes{};
	private:
		//==============================
		// Internal Fields
		//==============================
		std::vector<ScriptToken> m_EnumTypes;
	private:
		//==============================
		// Owning Class
		//==============================
		friend class ScriptCompiler;
	};
}