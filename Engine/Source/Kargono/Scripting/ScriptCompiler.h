#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <tuple>
#include <limits>
#include <sstream>
#include <variant>

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

		// Identifier
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

	enum class ParseErrorType
	{
		// Invalid Error Code
		None = 0,

		// Parse Function Error Codes
		FuncReturn,
		FuncName,
		FuncParam,
		FuncPunc,
		FuncBody,

		// Parse Statement Error Codes
		StatePunc,
		StateValue,

		// Program Boundaries
		ProgEnd
	};

	static inline uint32_t InvalidLine { std::numeric_limits<uint32_t>::max() };
	static inline uint32_t InvalidColumn { std::numeric_limits<uint32_t>::max() };

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

		case Scripting::ScriptTokenType::None: return "None";
		default:
		{
			KG_CRITICAL("Unknown Type of ScriptTokenType.");
			return {};

		}
		}
	}

	inline std::vector<Scripting::ScriptTokenType> PrimitiveTypeAcceptableInput(const std::string& type)
	{
		if (type == "String")
		{
			return { Scripting::ScriptTokenType::StringLiteral };
		}

		if (type == "UInt16")
		{
			return { Scripting::ScriptTokenType::IntegerLiteral };
		}

		return {};
	}

	inline std::string ParseErrorTypeToString(Scripting::ParseErrorType type)
	{
		switch (type)
		{
			case Scripting::ParseErrorType::FuncReturn: return "FuncReturn";
			case Scripting::ParseErrorType::FuncName: return "FuncName";
			case Scripting::ParseErrorType::FuncParam: return "FuncParam";
			case Scripting::ParseErrorType::FuncPunc: return "FuncPunc";
			case Scripting::ParseErrorType::FuncBody: return "FuncBody";

			case Scripting::ParseErrorType::StatePunc: return "StatePunc";
			case Scripting::ParseErrorType::StateValue: return "StateValue";

			case Scripting::ParseErrorType::ProgEnd: return "ProgEnd";

			case Scripting::ParseErrorType::None:
			default:
			{
				KG_CRITICAL("Unknown ParseErrorType");
				return {"Unknown Error"};

			}
		}
	}
}

namespace Kargono::Scripting
{
	struct ScriptToken
	{
	public:
		ScriptTokenType Type{ ScriptTokenType::None };
		std::string Value {};
		uint32_t Line{ InvalidLine };
		uint32_t Column{ InvalidColumn };
	public:
		operator bool() const
		{
			return Type != ScriptTokenType::None;
		}

		std::string ToString() const
		{
			std::stringstream stringStream {};
			stringStream << "Type ("
				<< Utility::ScriptTokenTypeToString(Type)
				<< ") Value (" << Value
				<< ") Line/Column (" << Line << "/" << Column << ")";
			return stringStream.str();
		}
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
		void AddTokenAndClearBuffer(ScriptTokenType type, const std::string& value);
	private:
		std::string m_ScriptText{};
		std::string m_TextBuffer{};
		std::vector<ScriptToken> m_Tokens {};
		uint32_t m_TextLocation{ 0 };
		uint32_t m_LineCount{ 1 };
		uint32_t m_ColumnCount{ 0 };

		std::vector<std::string> m_Keywords {"return", "void"};
		std::vector<std::string> m_PrimitiveTypes {"String", "UInt16"};
	};

	struct StatementEmpty
	{
	};

	struct StatementLiteral
	{
		ScriptToken ExpressionValue{};
	};

	struct StatementDeclaration
	{
		ScriptToken Type{};
		ScriptToken Name{};
	};

	struct StatementAssignment
	{
		ScriptToken Type{};
		ScriptToken Name{};
		ScriptToken Value{};
	};

	using Statement = std::variant<StatementEmpty, StatementLiteral, StatementDeclaration, StatementAssignment>;

	struct FunctionParameter
	{
		ScriptToken ParameterType{};
		ScriptToken ParameterName{};
	};

	struct FunctionNode
	{
		ScriptToken Name{};
		ScriptToken ReturnType{};
		std::vector<FunctionParameter> Parameters{};
		std::vector<Statement> Statements{};

		operator bool() const
		{
			return Name && ReturnType;
		}
	};

	struct ProgramNode
	{
	public:
		FunctionNode FuncNode;
	public:
		operator bool() const
		{
			return FuncNode;
		}
	};

	struct ScriptAST
	{
	public:
		ProgramNode ProgramNode {};

	public:
		operator bool() const 
		{ 
			return ProgramNode; 
		}
	};

	struct ParserError
	{
	public:
		ParseErrorType Type{ ParseErrorType::None };
		std::string Message{};
		ScriptToken CurrentToken{};
		ScriptToken PreviousToken{};
	public:
		std::string ToString() const
		{
			std::stringstream stringStream {};
			stringStream << "[Error " << Utility::ParseErrorTypeToString(Type) <<"]: " << Message << '\n';
			stringStream << "  Current Token: " << CurrentToken.ToString() << '\n';
			stringStream << "  Previous Token: " << PreviousToken.ToString();
			return stringStream.str();
		}

	};

	class TokenParser
	{
	public:
		std::tuple<bool, ScriptAST> ParseTokens(std::vector<ScriptToken> tokens);
		void PrintAST();
		void PrintErrors();
	private:
		std::tuple<bool, Statement> ParseStatementNode();
		std::tuple<bool, FunctionNode> ParseFunctionNode();

		ScriptToken GetCurrentToken(int32_t offset = 0);
		void Advance(uint32_t count = 1);
		void StoreParseError(ParseErrorType errorType, const std::string& message);
		bool CheckForErrors();
	private:
		std::vector<ScriptToken> m_Tokens{};
		std::vector<ParserError> m_Errors {};
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
