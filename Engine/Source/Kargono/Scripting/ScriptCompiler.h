#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <tuple>
#include <limits>
#include <sstream>
#include <variant>
#include <unordered_map>

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
		AdditionOperator,
		SubtractionOperator,
		MultiplicationOperator,
		DivisionOperator
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

		// Parse Expression Error Codes
		ExpressionValue,

		// Parse Statement Error Codes
		StatePunc,
		StateValue,

		// Validation Function Error Codes
		PrimTypeAccep,


		// Program Boundaries
		ProgEnd
	};

	static inline uint32_t InvalidLine { std::numeric_limits<uint32_t>::max() };
	static inline uint32_t InvalidColumn { std::numeric_limits<uint32_t>::max() };


	struct PrimitiveType
	{
		std::string Name {};
		ScriptTokenType AcceptableLiteral{};
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
		case Scripting::ScriptTokenType::SubtractionOperator: return "Subtraction Operator";
		case Scripting::ScriptTokenType::DivisionOperator: return "Division Operator";
		case Scripting::ScriptTokenType::MultiplicationOperator: return "Multiplication Operator";

		case Scripting::ScriptTokenType::None: return "None";
		default:
		{
			KG_CRITICAL("Unknown Type of ScriptTokenType.");
			return {};

		}
		}
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

			case Scripting::ParseErrorType::ExpressionValue: return "ExpressionValue";

			case Scripting::ParseErrorType::StatePunc: return "StatePunc";
			case Scripting::ParseErrorType::StateValue: return "StateValue";

			case Scripting::ParseErrorType::PrimTypeAccep: return "PrimTypeAccep";

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
	};

	struct Expression;

	struct FunctionCallNode
	{
		ScriptToken Namespace{};
		ScriptToken Identifier{};
		ScriptToken ReturnType{};
		std::vector<ScriptToken> Arguments{};
	};

	struct UnaryOperationNode
	{
		ScriptToken Operator{};
		ScriptToken ReturnType{};
		ScriptToken Operand{};
	};

	struct BinaryOperationNode
	{
		ScriptToken Operator{};
		ScriptToken ReturnType{};
		Ref<Expression> Operand1{};
		Ref<Expression> Operand2{};
	};

	struct Expression
	{
		std::variant<FunctionCallNode, ScriptToken, UnaryOperationNode , BinaryOperationNode> Value {};

		ScriptToken GetReturnType()
		{
			ScriptToken returnType;
			std::visit([&](auto&& value)
			{
				using valueType = std::decay_t<decltype(value)>;
				if constexpr (std::is_same_v<valueType, FunctionCallNode>)
				{
					returnType = value.ReturnType;
				}
				else if constexpr (std::is_same_v<valueType, UnaryOperationNode>)
				{
					returnType = value.ReturnType;
				}
				else if constexpr (std::is_same_v<valueType, BinaryOperationNode>)
				{
					returnType = value.ReturnType;
				}
				else if constexpr (std::is_same_v<valueType, ScriptToken>)
				{
					returnType = value;
				}
				
			}, Value);

			return returnType;
		}
	};

	struct StatementEmpty
	{
	};

	struct StatementExpression
	{
		Ref<Expression> Value { nullptr };
	};

	struct StatementDeclaration
	{
		ScriptToken Type{};
		ScriptToken Name{};
	};

	struct StatementAssignment
	{
		ScriptToken Name{};
		Ref<Expression> Value { nullptr };
	};

	struct StatementDeclarationAssignment
	{
		ScriptToken Type{};
		ScriptToken Name{};
		Ref<Expression> Value { nullptr };
	};

	using Statement = std::variant<StatementEmpty, StatementExpression, StatementDeclaration, StatementAssignment , StatementDeclarationAssignment>;

	struct FunctionParameter
	{
		ScriptToken Type{};
		ScriptToken Identifier{};
	};

	struct FunctionNode
	{
		ScriptToken Name{};
		ScriptToken ReturnType{};
		ScriptToken Namespace{};
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

	struct StackVariable 
	{
	public:
		ScriptToken Type{};
		ScriptToken Identifier{};
	public:
		operator bool() const
		{
			return Type && Identifier;
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
		std::tuple<bool, Ref<Expression>> ParseExpressionNode(uint32_t& parentExpressionSize);
	private:

		std::tuple<bool, Ref<Expression>> ParseExpressionTerm(uint32_t& parentExpressionSize, bool checkBinaryOperations = true);
		std::tuple<bool, Ref<Expression>> ParseExpressionLiteral(uint32_t& parentExpressionSize);
		std::tuple<bool, Ref<Expression>> ParseExpressionIdentifier(uint32_t& parentExpressionSize);
		std::tuple<bool, Ref<Expression>> ParseExpressionFunctionCall(uint32_t& parentExpressionSize);
		std::tuple<bool, Ref<Expression>> ParseExpressionUnaryOperation(uint32_t& parentExpressionSize);

		std::tuple<bool, Statement> ParseStatementEmpty();
		std::tuple<bool, Statement> ParseStatementExpression();
		std::tuple<bool, Statement> ParseStatementDeclaration();
		std::tuple<bool, Statement> ParseStatementAssignment();
		std::tuple<bool, Statement> ParseStatementDeclarationAssignment();
	private:
		ScriptToken GetCurrentToken(int32_t offset = 0);
		void Advance(uint32_t count = 1);
		void StoreStackVariable(ScriptToken type, ScriptToken identifier);
		void AddStackFrame();
		void PopStackFrame();
		bool CheckStackForIdentifier(ScriptToken identifier);
		bool CheckCurrentStackFrameForIdentifier(ScriptToken identifier);
		StackVariable GetStackVariable(ScriptToken identifier);
		void StoreParseError(ParseErrorType errorType, const std::string& message);
		bool CheckForErrors();
		bool IsLiteralOrIdentifier(ScriptToken token);
		bool IsLiteral(ScriptToken token);
		bool IsUnaryOperator(ScriptToken token);
		bool IsBinaryOperator(ScriptToken token);
		bool IsAdditionOrSubtraction(ScriptToken token);
		bool IsMultiplicationOrDivision(ScriptToken token);
		bool PrimitiveTypeAcceptableToken(const std::string& type, Scripting::ScriptToken token);
		ScriptToken GetPrimitiveTypeFromToken(Scripting::ScriptToken token);
	private:
		std::vector<ScriptToken> m_Tokens{};
		std::vector<ParserError> m_Errors {};
		std::vector<std::vector<StackVariable>> m_StackVariables{};
		ScriptAST m_AST{};
		uint32_t m_TokenLocation{0};
	};

	struct LanguageDefinition
	{
	public:
		std::vector<std::string> Keywords {};
		std::vector<PrimitiveType> PrimitiveTypes {};
		std::unordered_map<std::string, FunctionNode> FunctionDefinitions {};
	public:
		operator bool() const
		{
			return Keywords.size() > 0 || PrimitiveTypes.size() > 0 || FunctionDefinitions.size() > 0;
		}
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
		static void CreateKGScriptLanguageDefinition();
	public:
		static LanguageDefinition s_ActiveLanguageDefinition;
	};
}
