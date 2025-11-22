#pragma once
#include "Modules/Scripting/ScriptCompilerCommon.h"
#include "Modules/Scripting/ScriptLanguageDefinition.h"

namespace Kargono::Rendering { class Texture2D; }

namespace Kargono::Scripting 
{
	struct SuggestionSpec
	{
		Assets::AssetRef<Rendering::Texture2D> m_Icon {};
		std::string m_Label {};
		std::string m_ReplacementText {};
		int16_t m_ShiftValue{ 0 };
	};

	using SuggestionList = std::vector<SuggestionSpec>;

	class SuggestionGenerator
	{
	private:
		//==============================
		// Constructors/Destructors
		//==============================
		SuggestionGenerator(LanguageDefinition& parentLanguage) : i_ParentLangDef(parentLanguage)
		{
		};
		~SuggestionGenerator() = default;
	public:
		//==============================
		// Generate Suggestions
		//==============================
		SuggestionList GetSuggestions(const std::string& scriptText, const std::string& queryText);
	private:
		// Helpers
		void GetSuggestionsForAfterNamespace(SuggestionList& allSuggestions, const CursorContext& context, const std::string& queryText);
		void GetSuggestionsForIsParameter(SuggestionList& allSuggestions, const CursorContext& context, const std::string& queryText);
		void GetSuggestionsForIsDataMember(SuggestionList& allSuggestions, const CursorContext& context, const std::string& queryText);
		void GetSuggestionsDefault(SuggestionList& allSuggestions, const CursorContext& context, const std::string& queryText);
		void GetSuggestionsForLiteralMember(SuggestionList& allSuggestions, const CursorContext& context, const std::string& queryText);
	public:
		//==============================
		// Injected Dependencies
		//==============================
		LanguageDefinition& i_ParentLangDef;
	private:
		//==============================
		// Owning Class
		//==============================
		friend class ScriptCompiler;
	};

	class ScriptCompiler
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		ScriptCompiler() = default;
		~ScriptCompiler() = default;
	public:
		//==============================
		// Lifecycle Functions
		//==============================
		void Terminate();
	public:
		//==============================
		// Compile
		//==============================
		std::string CompileScriptFile(const std::filesystem::path& scriptLocation);
	public:
		//==============================
		// Check For Issues
		//==============================
		std::vector<ParserError> CheckForErrors(const std::string& text);
		CursorContext FindCursorContext(const std::string& text);
	public:
		//==============================
		// Public Fields
		//==============================
		LanguageDefinition m_ActiveLanguageDefinition;
		SuggestionGenerator m_SuggestGen{ m_ActiveLanguageDefinition };
	};

	class ScriptCompilerService // TODO: EWWWWW UGHHHHHHH
	{
	public:
		//==============================
		// Getters/Setters
		//==============================
		static ScriptCompiler& GetActiveContext() { return s_ScriptCompiler; }
	private:
		//==============================
		// Internal Fields
		//==============================
		static inline ScriptCompiler s_ScriptCompiler{};
	};
}
