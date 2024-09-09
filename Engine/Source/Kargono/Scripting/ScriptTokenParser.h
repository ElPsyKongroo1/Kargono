#pragma once

#include "Kargono/Scripting/ScriptCompilerCommon.h"

namespace Kargono::Scripting
{
	class ScriptTokenParser
	{
	public:
		std::tuple<bool, ScriptAST> ParseTokens(std::vector<ScriptToken>&& tokens);
		void PrintAST();
		void PrintTokens();
		void PrintErrors();
		std::tuple<bool, CursorContext> GetCursorContext();
	public:
		std::vector<ParserError> GetErrors() { return m_Errors; }
	private:
		std::tuple<bool, Ref<Statement>> ParseStatementNode();
		std::tuple<bool, FunctionNode> ParseFunctionNode();
		std::tuple<bool, Ref<Expression>> ParseExpressionNode(uint32_t& parentExpressionSize);
	private:

		std::tuple<bool, Ref<Expression>> ParseExpressionTerm(uint32_t& parentExpressionSize, bool checkBinaryOperations = true);
		std::tuple<bool, Ref<Expression>> ParseExpressionLiteral(uint32_t& parentExpressionSize);
		std::tuple<bool, Ref<Expression>> ParseExpressionIdentifier(uint32_t& parentExpressionSize);
		std::tuple<bool, Ref<Expression>> ParseExpressionFunctionCall(uint32_t& parentExpressionSize);
		std::tuple<bool, Ref<Expression>> ParseExpressionUnaryOperation(uint32_t& parentExpressionSize);
		std::tuple<bool, Ref<Expression>> ParseExpressionInitializationList(uint32_t& parentExpressionSize);

		std::tuple<bool, Ref<Statement>> ParseStatementEmpty();
		std::tuple<bool, Ref<Statement>> ParseStatementExpression();
		std::tuple<bool, Ref<Statement>> ParseStatementDeclaration();
		std::tuple<bool, Ref<Statement>> ParseStatementAssignment();
		std::tuple<bool, Ref<Statement>> ParseStatementDeclarationAssignment();
		std::tuple<bool, Ref<Statement>> ParseStatementConditional(bool chainConditions);
	private:
		ScriptToken GetToken(int32_t location);
		ScriptToken GetCurrentToken(int32_t offset = 0);
		void Advance(uint32_t count = 1);
		void StoreStackVariable(ScriptToken type, ScriptToken identifier);
		void AddStackFrame();
		void PopStackFrame();
		bool CheckStackForIdentifier(ScriptToken identifier);
		bool CheckCurrentStackFrameForIdentifier(ScriptToken identifier);
		StackVariable GetStackVariable(ScriptToken identifier);
		void StoreParseError(ParseErrorType errorType, const std::string& message, ScriptToken errorToken);
		bool CheckForErrors();
		bool IsContextProbe(ScriptToken token);
		bool IsContextProbe(Ref<Expression> expression);
		bool PrimitiveTypeAcceptableToken(const std::string& type, Scripting::ScriptToken token);
		ScriptToken GetPrimitiveTypeFromToken(Scripting::ScriptToken token);
	private:
		std::vector<ScriptToken> m_Tokens{};
		std::vector<ParserError> m_Errors {};
		std::vector<std::vector<StackVariable>> m_StackVariables{};
		ScriptAST m_AST{};
		uint32_t m_TokenLocation{ 0 };
		CursorContext m_CursorContext{};
	};
}
