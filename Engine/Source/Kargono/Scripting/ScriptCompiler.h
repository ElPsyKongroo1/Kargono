#pragma once
#include <string>
#include <vector>
#include <filesystem>

namespace Kargono::Scripting
{
	enum class ScriptTokenType
	{
		None = 0,

		// Literals
		IntegerLiteral,
		StringLiteral,

		// Keywords
		Return,
		Void,

		// Primitive Types
		String,
		UInt16,

		// Variable
		Identifier,

		// Punctuation
		Semicolon,
		NamespaceResolver,
		OpenParentheses,
		CloseParentheses,
		OpenCurlyBrace,
		CloseCurlyBrace,
		Comma,

		// Binary Operators
		AssignmentOperator,
		AdditionOperator
	};

	struct ScriptToken
	{
		ScriptTokenType Type{ ScriptTokenType::None };
		std::string Value {};
	};

	//==============================
	// Script Compiler Class
	//==============================
	class ScriptCompiler
	{
	public:
		//==============================
		// External API
		//==============================
		static std::string CompileScriptFile(const std::filesystem::path& scriptLocation);
	private:
		//==============================
		// Internal Functionality
		//==============================
		static std::vector<ScriptToken> ConvertTextToTokens(const std::string& text);
	};
}

namespace Kargono::Utility
{
	inline std::string ScriptTokenTypeToString(Scripting::ScriptTokenType type)
	{
		switch (type)
		{
			case Scripting::ScriptTokenType::IntegerLiteral: return "Integer Literal";
			case Scripting::ScriptTokenType::StringLiteral: return "String Literal";

			case Scripting::ScriptTokenType::Return: return "Return";
			case Scripting::ScriptTokenType::Void: return "Void";

			case Scripting::ScriptTokenType::String: return "String";
			case Scripting::ScriptTokenType::UInt16: return "UInt16";

			case Scripting::ScriptTokenType::Identifier: return "Identifier";

			case Scripting::ScriptTokenType::Semicolon: return "Semicolon";
			case Scripting::ScriptTokenType::NamespaceResolver: return "Namespace Resolver";
			case Scripting::ScriptTokenType::OpenParentheses: return "Open Parentheses";
			case Scripting::ScriptTokenType::CloseParentheses: return "Close Parentheses";
			case Scripting::ScriptTokenType::OpenCurlyBrace: return "Open Curly Brace";
			case Scripting::ScriptTokenType::CloseCurlyBrace: return "Close Curly Brace";
			case Scripting::ScriptTokenType::Comma: return "Comma";

			case Scripting::ScriptTokenType::AssignmentOperator: return "Assignment Operator";
			case Scripting::ScriptTokenType::AdditionOperator: return "Addition Operator";

			case Scripting::ScriptTokenType::None:
			default:
			{
				KG_CRITICAL("Unknown Type of ScriptTokenType.");
				return {};
				
			}
		}
	}
}
