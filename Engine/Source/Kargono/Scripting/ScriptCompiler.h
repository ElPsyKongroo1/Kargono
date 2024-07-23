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
		Keyword,

		// Primitive Types
		PrimitiveType,

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

	class ScriptTokenizer
	{
	public:
		std::vector<ScriptToken> TokenizeString(std::string m_ScriptText);
	private:
		char GetCurrentChar(int32_t offset = 0);
		bool CurrentLocationValid();
		void AddCurrentCharToBuffer();
		void Advance(uint32_t count = 1);
		void AddTokenAndClearBuffer(const ScriptToken& token);
	private:
		std::string m_ScriptText{};
		std::string m_TextBuffer{};
		std::vector<ScriptToken> m_Tokens {};
		uint32_t m_TextLocation{ 0 };

		std::vector<std::string> m_Keywords {"return", "void"};
		std::vector<std::string> m_PrimitiveTypes {"String", "UInt16"};
	};

	struct FunctionNode
	{
		
	};

	struct ProgramNode
	{
	public:

	public:
		FunctionNode funcNode;
		// TODO: Add function node
	};

	struct ScriptAST
	{
		Ref<ProgramNode> ProgramNode { nullptr };
	};


	class TokenParser
	{
	public:
		ScriptAST ParseTokens(std::vector<ScriptToken> tokens);

	private:
		ScriptToken& GetCurrentToken(int32_t offset = 0);
		void Advance(uint32_t count = 1);
	private:
		std::vector<ScriptToken> m_Tokens{};
		ScriptAST m_AST{};
		uint32_t m_TokenLocation{0};
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

			
			case Scripting::ScriptTokenType::Keyword: return "Keyword";
			case Scripting::ScriptTokenType::PrimitiveType: return "Primitive Type";
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
