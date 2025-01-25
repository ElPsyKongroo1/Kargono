#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <tuple>
#include <limits>
#include <sstream>
#include <variant>
#include <unordered_map>
#include <unordered_set>
#include <sstream>

#include "Kargono/Core/Base.h"
#include "Kargono/Core/BitField.h"
#include "Kargono/Core/WrappedData.h"

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
		MessageTypeLiteral,
		// Language specific literals
		CustomLiteral,

		// Keywords
		Keyword,

		// Primitive Types
		PrimitiveType,

		// Identifier
		Identifier,

		// Punctuation
		Semicolon,
		NamespaceResolver,
		OrOperator,
		AndOperator,
		OpenParentheses,
		CloseParentheses,
		OpenCurlyBrace,
		CloseCurlyBrace,
		Comma,
		DotOperator,

		// Arithmetic Operators
		AssignmentOperator,
		AdditionOperator,
		SubtractionOperator,
		MultiplicationOperator,
		DivisionOperator,

		// Comparison Operators
		EqualToOperator,
		NotEqualToOperator,
		GreaterThan,
		GreaterThanOrEqual,
		LessThan,
		LessThanOrEqual,
		ConditionalOperator,
		PrecedenceOperator,
		NegationOperator,

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

	struct MemberType;

	struct PrimitiveType
	{
		std::string Name {};
		std::string Description {};
		ScriptTokenType AcceptableLiteral{};
		std::string EmittedDeclaration {};
		std::string EmittedParameter {};
		Ref<Rendering::Texture2D> Icon {};
		std::unordered_map<std::string, Ref<MemberType>> Members{};
		std::unordered_set<std::string> AcceptableArithmetic{};
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
		case Scripting::ScriptTokenType::MessageTypeLiteral: return "Message Type Literal";
		case Scripting::ScriptTokenType::CustomLiteral: return "Custom Literal";

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
		case Scripting::ScriptTokenType::DotOperator: return "Dot Operator";
		case Scripting::ScriptTokenType::OrOperator: return "Or Operator";
		case Scripting::ScriptTokenType::AndOperator: return "And Operator";

		case Scripting::ScriptTokenType::AssignmentOperator: return "Assignment Operator";
		case Scripting::ScriptTokenType::AdditionOperator: return "Addition Operator";
		case Scripting::ScriptTokenType::SubtractionOperator: return "Subtraction Operator";
		case Scripting::ScriptTokenType::DivisionOperator: return "Division Operator";
		case Scripting::ScriptTokenType::MultiplicationOperator: return "Multiplication Operator";

		case Scripting::ScriptTokenType::EqualToOperator: return "Equal To Operator";
		case Scripting::ScriptTokenType::NotEqualToOperator: return "Not Equal To Operator";
		case Scripting::ScriptTokenType::GreaterThan: return "Greater Than";
		case Scripting::ScriptTokenType::GreaterThanOrEqual: return "Greater Than Or Equal";
		case Scripting::ScriptTokenType::LessThan: return "Less Than";
		case Scripting::ScriptTokenType::LessThanOrEqual: return "Less Than Or Equal";
		case Scripting::ScriptTokenType::ConditionalOperator: return "Conditional Operator";
		case Scripting::ScriptTokenType::PrecedenceOperator: return "Precedence Operator";

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
		std::string Value{};
		uint32_t Line{ InvalidLine };
		uint32_t Column{ InvalidColumn };
	public:
		operator bool() const
		{
			return Type != ScriptTokenType::None;
		}

		std::string ToString() const
		{
			std::stringstream stringStream{};
			stringStream << "  Type: " << Utility::ScriptTokenTypeToString(Type) << '\n'
				<< "  Value: " << Value << '\n'
				<< "  Line/Column: " << Line << "/" << Column << '\n';
			return stringStream.str();
		}
	};

	struct Expression;

	struct MemberType;

	struct MemberNode
	{
		Ref<MemberNode> ChildMemberNode{};
		Ref<Expression> CurrentNodeExpression{};
		ScriptToken ReturnType{};
		MemberType* m_MemberType{};
	};

	struct FunctionNode;

	struct FunctionCallNode
	{
		ScriptToken Namespace{};
		ScriptToken Identifier{};
		ScriptToken ReturnType{};
		std::vector<Ref<Expression>> Arguments{};
		FunctionNode* m_FunctionNode{};
	};

	struct CustomLiteralNode
	{
		ScriptToken Namespace{};
		ScriptToken Identifier{};
		ScriptToken ReturnType{};
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

	struct TernaryOperationNode
	{
		Ref<Expression> Conditional {};
		Ref<Expression> OptionOne {};
		Ref<Expression> OptionTwo {};
		ScriptToken ReturnType{};
	};

	struct TokenExpressionNode
	{
		ScriptToken Value{};
		ScriptToken ReturnType{};
	};

	struct ExpressionGenerationAffixes
	{
		std::string Prefix;
		std::string Postfix;
	};

	struct Expression
	{
		std::variant<FunctionCallNode, TokenExpressionNode, UnaryOperationNode, BinaryOperationNode, InitializationListNode, MemberNode, TernaryOperationNode, CustomLiteralNode> Value{};
		Ref<ExpressionGenerationAffixes> GenerationAffixes{ nullptr };

		ScriptToken GetReturnType()
		{
			if (FunctionCallNode* functionCallNodePtr = std::get_if<FunctionCallNode>(&Value))
			{
				FunctionCallNode& functionCallNode = *functionCallNodePtr;
				return functionCallNode.ReturnType;
			}
			else if (CustomLiteralNode* customLiteralNodePtr = std::get_if<CustomLiteralNode>(&Value))
			{
				CustomLiteralNode& customLiteralNode = *customLiteralNodePtr;
				return customLiteralNode.ReturnType;
			}
			else if (UnaryOperationNode* unaryOperationNodePtr = std::get_if<UnaryOperationNode>(&Value))
			{
				UnaryOperationNode& unaryOperationNode = *unaryOperationNodePtr;
				return unaryOperationNode.ReturnType;
			}
			else if (BinaryOperationNode* binaryOperationNodePtr = std::get_if<BinaryOperationNode>(&Value))
			{
				BinaryOperationNode& binaryOperationNode = *binaryOperationNodePtr;
				return binaryOperationNode.ReturnType;
			}
			else if (InitializationListNode* initializationListNodePtr = std::get_if<InitializationListNode>(&Value))
			{
				InitializationListNode& initializationListNode = *initializationListNodePtr;
				return initializationListNode.ReturnType;
			}
			else if (TernaryOperationNode* ternaryOperationNodePtr = std::get_if<TernaryOperationNode>(&Value))
			{
				TernaryOperationNode& ternaryOperationNode = *ternaryOperationNodePtr;
				return ternaryOperationNode.ReturnType;
			}
			else if (TokenExpressionNode* scriptTokenPtr = std::get_if<TokenExpressionNode>(&Value))
			{
				TokenExpressionNode& tokenExpressionNode = *scriptTokenPtr;
				return tokenExpressionNode.ReturnType;
			}
			else if (MemberNode* memberNodePtr = std::get_if<MemberNode>(&Value))
			{
				MemberNode& memberNode = *memberNodePtr;
				return memberNode.ReturnType;
			}

			KG_WARN("Invalid expression type attempted to be parsed");
			return {};
		}
	};

	struct Statement;

	struct StatementEmpty
	{
	};

	struct StatementReturn
	{
		Ref<Expression> ReturnValue{ nullptr };
	};

	struct StatementExpression
	{
		Ref<Expression> Value{ nullptr };
	};

	struct StatementDeclaration
	{
		ScriptToken Type{};
		ScriptToken Name{};
	};

	struct StatementAssignment
	{
		Ref<Expression> Name{};
		Ref<Expression> Value{ nullptr };
	};

	struct StatementDeclarationAssignment
	{
		ScriptToken Type{};
		ScriptToken Name{};
		Ref<Expression> Value{ nullptr };
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
		Ref<Expression> ConditionExpression{ nullptr };
		std::vector<Ref<Statement>> BodyStatements{};
		std::vector<Ref<Statement>> ChainedConditionals{};
	};

	struct StatementWhileLoop
	{
		Ref<Expression> ConditionExpression{ nullptr };
		std::vector<Ref<Statement>> BodyStatements{};
	};

	struct StatementBreak
	{

	};

	struct StatementContinue
	{

	};

}

namespace Kargono::Utility
{
	inline std::string ConditionalTypeToString(Scripting::ConditionalType type)
	{
		switch (type)
		{
		case Scripting::ConditionalType::IF: return "IF";

		case Scripting::ConditionalType::ELSE: return "ELSE";

		case Scripting::ConditionalType::ELSEIF: return "ELSE IF";

		case Scripting::ConditionalType::None:
		default:
		{
			KG_CRITICAL("Unknown ConditionalType");
			return { "Unknown Error" };

		}
		}
	}
}

namespace Kargono::Scripting
{

	struct Statement
	{
		std::variant<StatementEmpty, StatementExpression, 
			StatementDeclaration, StatementAssignment, 
			StatementDeclarationAssignment, StatementConditional,
			StatementReturn, StatementWhileLoop,
			StatementBreak, StatementContinue> Value;
	};

	struct FunctionParameter
	{
		std::vector<ScriptToken> AllTypes{};
		ScriptToken Identifier{};
	};

	class ScriptOutputGenerator;

	struct FunctionNode
	{
		ScriptToken Name{};
		ScriptToken ReturnType{};
		ScriptToken Namespace{};
		std::vector<FunctionParameter> Parameters{};
		std::vector<Ref<Statement>> Statements{};
		std::function<void(ScriptOutputGenerator&, FunctionCallNode&)> OnGenerateFunction{ nullptr };
		std::function<void(ScriptOutputGenerator&, MemberNode&)> OnGenerateGetter { nullptr };
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
		ProgramNode m_ProgramNode{};

	public:
		operator bool() const
		{
			return m_ProgramNode;
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

	enum class CursorFlags : uint8_t
	{
		None = 0,
		IsFunctionParameter,
		IsDataMember,
		AfterNamespaceResolution,
		AllowAllVariableTypes
	};

	struct CursorContext
	{
		std::vector<ScriptToken> AllReturnTypes{};
		std::vector<std::vector<StackVariable>> StackVariables {};
		ScriptToken CurrentNamespace{};
		std::unordered_map<std::string, Ref<MemberType>> DataMembers{};
		BitField<uint8_t> m_Flags {};

		operator bool() const
		{
			return AllReturnTypes.size() > 0 || StackVariables.size() > 0 || CurrentNamespace || m_Flags || DataMembers.size() > 0;
		}
	};

	struct InitializationListType
	{
		std::vector<ScriptToken> ParameterTypes {};
		ScriptToken ReturnType{};
	};

	struct MemberType;

	struct DataMember
	{
		ScriptToken PrimitiveType{};
		std::string Name{};   
		std::string Description{};
		std::unordered_map<std::string, Ref<MemberType>> Members{};
		std::function<void(ScriptOutputGenerator&, MemberNode&)> OnGenerateGetter { nullptr };
		std::function<void(ScriptOutputGenerator&, StatementAssignment&)> OnGenerateSetter { nullptr };
	};

	struct MemberType
	{
		std::variant<FunctionNode, DataMember> Value {};
	};

	struct CustomLiteralMember
	{
		std::string m_OutputText{};
		ScriptToken m_PrimitiveType{};
		std::unordered_map<std::string, Ref<CustomLiteralMember>> m_Members{};
	};

	// Asset management structs
	using CustomLiteralNameToIDMap = std::unordered_map<std::string, CustomLiteralMember>;
	struct CustomLiteralInfo
	{
		CustomLiteralNameToIDMap m_CustomLiteralNameToID;
		Ref<Rendering::Texture2D> m_LiteralIcon;
	};

	struct LanguageDefinition
	{
	public:
		std::vector<std::string> Keywords{};
		std::unordered_map<std::string, PrimitiveType> PrimitiveTypes {};
		std::unordered_map<std::string, std::string> NamespaceDescriptions {};
		std::unordered_map<std::string, FunctionNode> FunctionDefinitions {};
		std::vector<InitializationListType> InitListTypes {};
		std::unordered_set<std::string> AllMessageTypes{};

		// All Assets
		std::unordered_map<std::string, CustomLiteralInfo> AllLiteralTypes{};
	public:
		PrimitiveType GetPrimitiveTypeFromName(const std::string& name)
		{
			if (PrimitiveTypes.contains(name))
			{
				return PrimitiveTypes.at(name);
			}

			return {};
		}
	public:

		void Clear()
		{
			Keywords.clear();
			PrimitiveTypes.clear();
			NamespaceDescriptions.clear();
			FunctionDefinitions.clear();
			InitListTypes.clear();
			AllMessageTypes.clear();
			AllLiteralTypes.clear();
		}

		operator bool() const
		{
			return Keywords.size() > 0 || PrimitiveTypes.size() > 0 || FunctionDefinitions.size() > 0;
		}
	};
}
