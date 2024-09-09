#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <tuple>
#include <limits>
#include <sstream>
#include <variant>
#include <unordered_map>
#include <sstream>

#include "Kargono/Core/Base.h"

namespace Kargono::Rendering
{
	class Texture2D;
}

namespace Kargono::Scripting
{
	enum class ScriptTokenType
	{
		None = 0,

		// Literals
		IntegerLiteral,
		StringLiteral,
		BooleanLiteral,
		FloatLiteral,

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
		DivisionOperator,

		// Comments
		SingleLineComment,
		MultiLineComment
	};

	enum class ParseErrorType
	{
		// Invalid Error Code
		None = 0,
		Function,
		Expression,
		Statement,
		ContextProbe,
		Program
	};

	static inline uint32_t InvalidLine { std::numeric_limits<uint32_t>::max() };
	static inline uint32_t InvalidColumn { std::numeric_limits<uint32_t>::max() };


	struct PrimitiveType
	{
		std::string Name {};
		std::string Description {};
		ScriptTokenType AcceptableLiteral{};
		std::string EmittedDeclaration {};
		std::string EmittedParameter {};
		Ref<Rendering::Texture2D> Icon {};
	};

}

namespace Kargono::Utility
{
	inline std::string ScriptTokenTypeToString(Scripting::ScriptTokenType type)
	{
		switch (type)
		{
		case Scripting::ScriptTokenType::BooleanLiteral: return "Boolean Literal";
		case Scripting::ScriptTokenType::IntegerLiteral: return "Integer Literal";
		case Scripting::ScriptTokenType::StringLiteral: return "String Literal";
		case Scripting::ScriptTokenType::FloatLiteral: return "Float Literal";

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

		case Scripting::ScriptTokenType::SingleLineComment: return "Single Line Comment";
		case Scripting::ScriptTokenType::MultiLineComment: return "Multi Line Comment";

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
		case Scripting::ParseErrorType::Function: return "Function";

		case Scripting::ParseErrorType::Expression: return "Expression";

		case Scripting::ParseErrorType::Statement: return "Statement";
		case Scripting::ParseErrorType::ContextProbe: return "Context Probe";

		case Scripting::ParseErrorType::Program: return "Program";

		case Scripting::ParseErrorType::None:
		default:
		{
			KG_CRITICAL("Unknown ParseErrorType");
			return { "Unknown Error" };

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
			stringStream << "  Type: " << Utility::ScriptTokenTypeToString(Type) << '\n'
				<< "  Value: " << Value << '\n'
				<< "  Line/Column: " << Line << "/" << Column << '\n';
			return stringStream.str();
		}
	};

	struct Expression;

	struct FunctionCallNode
	{
		ScriptToken Namespace{};
		ScriptToken Identifier{};
		ScriptToken ReturnType{};
		std::vector<Ref<Expression>> Arguments{};
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
		Ref<Expression> LeftOperand{};
		Ref<Expression> RightOperand{};
	};

	struct InitializationListNode
	{
		std::vector<Ref<Expression>> Arguments{};
		ScriptToken ReturnType{};
	};

	struct ExpressionGenerationAffixes
	{
		std::string Prefix;
		std::string Postfix;
	};

	struct Expression
	{
		std::variant<FunctionCallNode, ScriptToken, UnaryOperationNode, BinaryOperationNode, InitializationListNode> Value {};
		Ref<ExpressionGenerationAffixes> GenerationAffixes {nullptr};

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
					else if constexpr (std::is_same_v<valueType, InitializationListNode>)
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

	struct Statement;

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

	enum class ConditionalType
	{
		None = 0,
		IF,
		ELSE,
		ELSEIF
	};

	struct StatementConditional
	{
		ConditionalType Type{ ConditionalType::None };
		Ref<Expression> ConditionExpression { nullptr };
		std::vector<Ref<Statement>> BodyStatements{};
		std::vector<Ref<Statement>> ChainedConditionals{};
	};

	struct Statement
	{
		std::variant<StatementEmpty, StatementExpression, StatementDeclaration, StatementAssignment, StatementDeclarationAssignment, StatementConditional> Value;
	};

	struct FunctionParameter
	{
		std::vector<ScriptToken> AllTypes{};
		ScriptToken Identifier{};
	};

	struct FunctionNode
	{
		ScriptToken Name{};
		ScriptToken ReturnType{};
		ScriptToken Namespace{};
		std::vector<FunctionParameter> Parameters{};
		std::vector<Ref<Statement>> Statements{};
		std::function<void(FunctionCallNode&)> OnGenerateFunction{ nullptr };
		std::string Description { "Built-in Function"};

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
		ProgramNode ProgramNode{};

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
	public:
		std::string ToString() const
		{
			std::stringstream stringStream {};
			stringStream << "[" << Utility::ParseErrorTypeToString(Type) << " Error]: " << Message << '\n';
			stringStream << CurrentToken.ToString();
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

	inline std::string ContextProbe {"CONTEXT_PROBE_INTERNAL"};

	struct CursorContext
	{
		std::vector<ScriptToken> AllReturnTypes{};
		std::vector<std::vector<StackVariable>> StackVariables {};
		bool IsFunctionParameter{ false };

		operator bool() const
		{
			return AllReturnTypes.size() > 0 || StackVariables.size() > 0;
		}
	};

	struct InitializationListType
	{
		std::vector<ScriptToken> ParameterTypes {};
		ScriptToken ReturnType{};
	};

	struct LanguageDefinition
	{
	public:
		std::vector<std::string> Keywords{};
		std::vector<PrimitiveType> PrimitiveTypes {};
		std::unordered_map<std::string, std::string> NamespaceDescriptions {};
		std::unordered_map<std::string, FunctionNode> FunctionDefinitions {};
		std::vector<InitializationListType> InitListTypes {};
	public:
		PrimitiveType GetPrimitiveTypeFromName(const std::string& name)
		{
			for (auto& type : PrimitiveTypes)
			{
				if (type.Name == name)
				{
					return type;
				}
			}

			return {};
		}
	public:
		operator bool() const
		{
			return Keywords.size() > 0 || PrimitiveTypes.size() > 0 || FunctionDefinitions.size() > 0;
		}
	};
}
