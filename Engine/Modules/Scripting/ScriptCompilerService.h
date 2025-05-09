#pragma once
#include "Modules/Scripting/ScriptCompilerCommon.h"

namespace Kargono::Rendering { class Texture2D; }

namespace Kargono::Scripting 
{

	struct SuggestionSpec
	{
		Ref<Rendering::Texture2D> m_Icon { nullptr };
		std::string m_Label {};
		std::string m_ReplacementText {};
		int16_t m_ShiftValue{ 0 };
	};

	//==============================
	// Script Compiler Service Class
	//==============================
	class ScriptCompilerService
	{
	public:
		//==============================
		// Lifecycle Functions
		//==============================
		static void Terminate();
		
		//==============================
		// External API
		//==============================
		static std::string CompileScriptFile(const std::filesystem::path& scriptLocation);
		static std::vector<ParserError> CheckForErrors(const std::string& text);
		static CursorContext FindCursorContext(const std::string& text);
		static std::vector<SuggestionSpec> GetSuggestions(const std::string& scriptText, const std::string& queryText);

	private:
		static void GetSuggestionsForAfterNamespace(std::vector<SuggestionSpec>& allSuggestions, const CursorContext& context, const std::string& queryText);
		static void GetSuggestionsForIsParameter(std::vector<SuggestionSpec>& allSuggestions, const CursorContext& context, const std::string& queryText);
		static void GetSuggestionsForIsDataMember(std::vector<SuggestionSpec>& allSuggestions, const CursorContext& context, const std::string& queryText);
		static void GetSuggestionsDefault(std::vector<SuggestionSpec>& allSuggestions, const CursorContext& context, const std::string& queryText);
		static void GetSuggestionsForLiteralMember(std::vector<SuggestionSpec>& allSuggestions, const CursorContext& context, const std::string& queryText);
		
	public:
		static void CreateKGScriptLanguageDefinition();
	private:
		static void CreateKGScriptKeywords();
		static void CreateKGScriptInitializationPrototypes();
		static void CreateKGScriptPrimitiveTypes();
		static void CreateKGScriptCustomLiterals();
		static void CreateKGScriptNamespaces();
		static void CreateKGScriptFunctionDefinitions();
	public:
		static bool IsLiteralOrIdentifier(ScriptToken token);
		static bool IsLiteral(ScriptToken token);
		static bool IsUnaryOperator(ScriptToken token);
		static bool IsBinaryOperator(ScriptToken token);
		static bool IsAdditionOrSubtractionOperator(ScriptToken token);
		static bool IsMultiplicationOrDivisionOperator(ScriptToken token);
		static bool IsComparisonOperator(ScriptToken token);
		static bool IsBooleanOperator(ScriptToken token);
	public:
		static LanguageDefinition s_ActiveLanguageDefinition;
	};
}
