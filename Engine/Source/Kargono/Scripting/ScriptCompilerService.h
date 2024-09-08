#pragma once
#include "Kargono/Scripting/ScriptCompilerCommon.h"

namespace Kargono::Scripting 
{
	//==============================
	// Script Compiler Service Class
	//==============================
	class ScriptCompilerService
	{
	public:
		//==============================
		// External API
		//==============================
		static std::string CompileScriptFile(const std::filesystem::path& scriptLocation);
		static void CreateKGScriptLanguageDefinition();
		static std::vector<ParserError> CheckForErrors(const std::string& text);
		static CursorContext FindCursorContext(const std::string& text);
	public:
		static bool IsLiteralOrIdentifier(ScriptToken token);
		static bool IsLiteral(ScriptToken token);
		static bool IsUnaryOperator(ScriptToken token);
		static bool IsBinaryOperator(ScriptToken token);
		static bool IsAdditionOrSubtraction(ScriptToken token);
		static bool IsMultiplicationOrDivision(ScriptToken token);
	public:
		static LanguageDefinition s_ActiveLanguageDefinition;
	};
}
