#pragma once

#include "Modules/Scripting/ScriptCompilerCommon.h"

namespace Kargono::Scripting
{

	using StackFrame = std::vector<StackVariable>;

	class StackContext
	{
	private:
		//==============================
		// Constructors/Destructors
		//==============================
		StackContext() = default;
		~StackContext() = default;
	public:
		//==============================
		// Modify Stack
		//==============================
		void StoreStackVariable(ScriptToken type, ScriptToken identifier);
		void AddStackFrame();
		void PopStackFrame();
	public:
		//==============================
		// Query Stack
		//==============================
		bool CheckStackForIdentifier(ScriptToken identifier);
		bool CheckCurrentStackFrameForIdentifier(ScriptToken identifier);
		StackVariable GetStackVariable(ScriptToken identifier);
	public:
		//==============================
		// Public Fields
		//==============================
		std::vector<StackFrame> m_StackVariables{};
	private:
		//==============================
		// Owning Class
		//==============================
		friend class ScriptTokenParser;
	};

	class ScriptTokenParser
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		ScriptTokenParser() = default;
		~ScriptTokenParser() = default;
	public:
		//==============================
		// Parse Tokens
		//==============================
		std::tuple<bool, ScriptAST> ParseTokens(std::vector<ScriptToken>&& tokens);
	private:
		std::tuple<bool, FunctionNode> ParseFunctionNode();
		// Expression Nodes
		std::tuple<bool, Ref<Expression>> ParseExpressionNode(uint32_t& parentExpressionSize);
		std::tuple<bool, Ref<Expression>> ParseExpressionTerm(uint32_t& parentExpressionSize, bool checkBinaryOperations = true);
		std::tuple<bool, Ref<Expression>> ParseExpressionLiteral(uint32_t& parentExpressionSize);
		std::tuple<bool, Ref<Expression>> ParseExpressionIdentifier(uint32_t& parentExpressionSize);
		std::tuple<bool, Ref<Expression>> ParseExpressionFunctionCall(uint32_t& parentExpressionSize);
		std::tuple<bool, Ref<Expression>> ParseExpressionCustomLiteral(uint32_t& parentExpressionSize);
		std::tuple<bool, Ref<Expression>> ParseExpressionUnaryOperation(uint32_t& parentExpressionSize);
		std::tuple<bool, Ref<Expression>> ParseExpressionInitializationList(uint32_t& parentExpressionSize);
		std::tuple<bool, Ref<Expression>> ParseExpressionTernaryOperation(Ref<Expression> currentExpression, uint32_t& parentExpressionSize);
		std::tuple<bool, Ref<Expression>> ParseExpressionMember(uint32_t& parentExpressionSize, bool dataMemberOnly = false);
		// Statement Nodes
		std::tuple<bool, Ref<Statement>> ParseStatementNode();
		std::tuple<bool, Ref<Statement>> ParseStatementEmpty();
		std::tuple<bool, Ref<Statement>> ParseStatementExpression();
		std::tuple<bool, Ref<Statement>> ParseStatementDeclaration();
		std::tuple<bool, Ref<Statement>> ParseStatementAssignment();
		std::tuple<bool, Ref<Statement>> ParseStatementDeclarationAssignment();
		std::tuple<bool, Ref<Statement>> ParseStatementConditional(bool chainConditions);
		std::tuple<bool, Ref<Statement>> ParseStatementWhileLoop();
		std::tuple<bool, Ref<Statement>> ParseStatementBreak();
		std::tuple<bool, Ref<Statement>> ParseStatementContinue();
		std::tuple<bool, Ref<Statement>> ParseStatementReturn();
	public:
		//==============================
		// Debug Printing
		//==============================
		void PrintAST();
		void PrintTokens();
		void PrintErrors();
	private:
		// Helper(s)
		void PrintFunction(const Scripting::FunctionNode& funcNode, uint32_t indentation = 0);
		void PrintStatement(const Ref<Scripting::Statement> statement, uint32_t indentation = 0);
		void PrintExpression(Ref<Scripting::Expression> expression, uint32_t indentation = 0);
		void PrintToken(const Scripting::ScriptToken& token, uint32_t indentation = 0);
		std::string GetIndentation(uint32_t count);
	public:
		//==============================
		// Error Checking
		//==============================
		std::vector<ParserError> GetErrors() { return m_Errors; }
		std::tuple<bool, CursorContext> GetCursorContext();
	private:
		// Helpers
		void StoreParseError(ParseErrorType errorType, const std::string& message, ScriptToken errorToken);
		bool CheckForErrors();
		bool IsContextProbe(ScriptToken token);
		bool IsContextProbe(Ref<Expression> expression);
	private:
		// Token Parsering
		ScriptToken GetToken(int32_t location);
		ScriptToken GetCurrentToken(int32_t offset = 0);
		Ref<Expression> TokenToExpression(ScriptToken token);
		bool PrimitiveTypeAcceptableToken(const std::string& queryType, Scripting::ScriptToken queryToken);
		ScriptToken GetPrimitiveTypeFromToken(Scripting::ScriptToken token);
		// For/while loop handling
		void IncrementLoopDepth();
		void DecrimentLoopDepth();
		// Other
		void Advance(int32_t count = 1);

	public:
		//==============================
		// Public Fields
		//==============================
		StackContext m_StackContext{};
	private:
		//==============================
		// Internal Fields
		//==============================
		std::vector<ScriptToken> m_Tokens{};
		std::vector<ParserError> m_Errors {};
		uint32_t m_LoopDepth{ 0 };
		ScriptAST m_AST{};
		uint32_t m_TokenLocation{ 0 };
		ScriptToken m_CurrentReturnType{};
		CursorContext m_CursorContext{};
	};
}
