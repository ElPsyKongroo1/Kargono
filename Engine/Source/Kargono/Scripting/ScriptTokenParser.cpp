#include "kgpch.h"
#include "Kargono/Scripting/ScriptTokenParser.h"
#include "Kargono/Scripting/ScriptCompilerService.h"
#include "Kargono/Core/KeyCodes.h"
#include "Kargono/Core/Resolution.h"

namespace Kargono::Utility
{
	static std::string GetIndentation(uint32_t count)
	{
		std::string outputIndentation {};
		for (uint32_t iteration{ 0 }; iteration < count; iteration++)
		{
			outputIndentation += "  ";
		}
		return outputIndentation;
	}

	static void PrintToken(const Scripting::ScriptToken& token, uint32_t indentation = 0)
	{
		KG_INFO("{}Type: {}", GetIndentation(indentation), Utility::ScriptTokenTypeToString(token.Type));
		KG_INFO("{}Value: {}", GetIndentation(indentation), token.Value);
	}

	static void PrintExpression(Ref<Scripting::Expression> expression, uint32_t indentation = 0)
	{
		if (!expression)
		{
			return;
		}
		if (Scripting::TokenExpressionNode* tokenExpression = std::get_if<Scripting::TokenExpressionNode>(&expression->Value))
		{
			KG_INFO("{}Expression Token", GetIndentation(indentation));
			KG_INFO("{}Expression Value", GetIndentation(indentation + 1));
			PrintToken(tokenExpression->Value, indentation + 2);
			KG_INFO("{}Expression Return Type", GetIndentation(indentation + 1));
			PrintToken(tokenExpression->ReturnType, indentation + 2);
		}
		else if (Scripting::FunctionCallNode* functionCallExpression = std::get_if<Scripting::FunctionCallNode>(&expression->Value))
		{
			KG_INFO("{}Expression Function Call", GetIndentation(indentation));
			KG_INFO("{}Namespace", GetIndentation(indentation + 1));
			PrintToken(functionCallExpression->Namespace, indentation + 2);
			KG_INFO("{}Identifier", GetIndentation(indentation + 1));
			PrintToken(functionCallExpression->Identifier, indentation + 2);
			KG_INFO("{}Return Type", GetIndentation(indentation + 1));
			PrintToken(functionCallExpression->ReturnType, indentation + 2);
			for (auto& argument : functionCallExpression->Arguments)
			{
				KG_INFO("{}Argument", GetIndentation(indentation + 1));
				PrintExpression(argument, indentation + 2);
			}
		}
		else if (Scripting::CustomLiteralNode* customLiteralExpression = std::get_if<Scripting::CustomLiteralNode>(&expression->Value))
		{
			KG_INFO("{}Expression Custom Literal", GetIndentation(indentation));
			KG_INFO("{}Namespace", GetIndentation(indentation + 1));
			PrintToken(customLiteralExpression->Namespace, indentation + 2);
			KG_INFO("{}Identifier", GetIndentation(indentation + 1));
			PrintToken(customLiteralExpression->Identifier, indentation + 2);
			KG_INFO("{}Return Type", GetIndentation(indentation + 1));
			PrintToken(customLiteralExpression->ReturnType, indentation + 2);
		}
		else if (Scripting::InitializationListNode* initListExpression = std::get_if<Scripting::InitializationListNode>(&expression->Value))
		{
			KG_INFO("{}Expression Initialization List", GetIndentation(indentation));
			KG_INFO("{}Return Type", GetIndentation(indentation + 1));
			PrintToken(initListExpression->ReturnType, indentation + 2);
			for (auto& argument : initListExpression->Arguments)
			{
				KG_INFO("{}Argument", GetIndentation(indentation + 1));
				PrintExpression(argument, indentation + 2);
			}
		}
		else if (Scripting::MemberNode* memberNodeExpression = std::get_if<Scripting::MemberNode>(&expression->Value))
		{
			KG_INFO("{}Expression Member Node", GetIndentation(indentation));
			KG_INFO("{}Expression", GetIndentation(indentation + 1));
			PrintExpression(memberNodeExpression->CurrentNodeExpression, indentation + 2);
			KG_INFO("{}Return Type", GetIndentation(indentation + 1));
			PrintToken(memberNodeExpression->ReturnType, indentation + 2);
			KG_INFO("{}All Child Nodes", GetIndentation(indentation + 1));
			Ref<Scripting::MemberNode> currentNode = memberNodeExpression->ChildMemberNode;
			while (currentNode)
			{
				KG_INFO("{}Child Expression", GetIndentation(indentation + 2));
				PrintExpression(currentNode->CurrentNodeExpression, indentation + 3);
				KG_INFO("{}Child Return Type", GetIndentation(indentation + 2));
				PrintToken(currentNode->ReturnType, indentation + 3);
				currentNode = currentNode->ChildMemberNode;
			}
		}
		else if (Scripting::UnaryOperationNode* unaryOperationExpression = std::get_if<Scripting::UnaryOperationNode>(&expression->Value))
		{
			KG_INFO("{}Expression Unary Operation", GetIndentation(indentation));
			KG_INFO("{}Operand", GetIndentation(indentation + 1));
			PrintToken(unaryOperationExpression->Operand, indentation + 2);
			KG_INFO("{}Operator", GetIndentation(indentation + 1));
			PrintToken(unaryOperationExpression->Operator, indentation + 2);
			KG_INFO("{}Return Type", GetIndentation(indentation + 1));
			PrintToken(unaryOperationExpression->ReturnType, indentation + 2);
		}
		else if (Scripting::BinaryOperationNode* binaryOperationExpression = std::get_if<Scripting::BinaryOperationNode>(&expression->Value))
		{
			KG_INFO("{}Expression Binary Operation", GetIndentation(indentation));
			KG_INFO("{}Operand 1", GetIndentation(indentation + 1));
			PrintExpression(binaryOperationExpression->LeftOperand, indentation + 2);
			KG_INFO("{}Operand 2", GetIndentation(indentation + 1));
			PrintExpression(binaryOperationExpression->RightOperand, indentation + 2);
			KG_INFO("{}Operator", GetIndentation(indentation + 1));
			PrintToken(binaryOperationExpression->Operator, indentation + 2);
			KG_INFO("{}Return Type", GetIndentation(indentation + 1));
			PrintToken(binaryOperationExpression->ReturnType, indentation + 2);
		}
		else if (Scripting::TernaryOperationNode* ternaryOperationNode = std::get_if<Scripting::TernaryOperationNode>(&expression->Value))
		{
			KG_INFO("{}Expression Ternary Operation", GetIndentation(indentation));
			KG_INFO("{}Condition", GetIndentation(indentation + 1));
			PrintExpression(ternaryOperationNode->Conditional, indentation + 2);
			KG_INFO("{}Option 1", GetIndentation(indentation + 1));
			PrintExpression(ternaryOperationNode->OptionOne, indentation + 2);
			KG_INFO("{}Option 2", GetIndentation(indentation + 1));
			PrintExpression(ternaryOperationNode->OptionTwo, indentation + 2);
			KG_INFO("{}Return Type", GetIndentation(indentation + 1));
			PrintToken(ternaryOperationNode->ReturnType, indentation + 2);
		}
	}

	static void PrintStatement(const Ref<Scripting::Statement> statement, uint32_t indentation = 0)
	{
		if (!statement)
		{
			return;
		}

		if (Scripting::StatementEmpty* emptyStatement = std::get_if<Scripting::StatementEmpty>(&statement->Value))
		{
			KG_INFO("{}Single Semicolon Statement", GetIndentation(indentation));
		}
		else if (Scripting::StatementReturn* expressionStatement = std::get_if<Scripting::StatementReturn>(&statement->Value))
		{
			KG_INFO("{}Return Statement", GetIndentation(indentation));
			KG_INFO("{}Return Value", GetIndentation(indentation + 1));
			PrintExpression(expressionStatement->ReturnValue, indentation + 2);
		}
		else if (Scripting::StatementExpression* expressionStatement = std::get_if<Scripting::StatementExpression>(&statement->Value))
		{
			KG_INFO("{}Expression Statement", GetIndentation(indentation));
			KG_INFO("{}Expression Value", GetIndentation(indentation + 1));
			PrintExpression(expressionStatement->Value, indentation + 2);
		}
		else if (Scripting::StatementDeclaration* declarationStatement = std::get_if<Scripting::StatementDeclaration>(&statement->Value))
		{
			KG_INFO("{}Declaration Statement", GetIndentation(indentation));
			KG_INFO("{}Declaration Type", GetIndentation(indentation + 1));
			PrintToken(declarationStatement->Type, indentation + 2);
			KG_INFO("{}Declaration Name/Identifier", GetIndentation(indentation + 1));
			PrintToken(declarationStatement->Name, indentation + 2);
		}
		else if (Scripting::StatementAssignment* assignmentStatement = std::get_if<Scripting::StatementAssignment>(&statement->Value))
		{
			KG_INFO("{}Assignment Statement", GetIndentation(indentation));
			KG_INFO("{}Assignment Identifier", GetIndentation(indentation + 1));
			PrintExpression(assignmentStatement->Name, indentation + 2);
			KG_INFO("{}Assignment Value", GetIndentation(indentation + 1));
			PrintExpression(assignmentStatement->Value, indentation + 2);
		}
		else if (Scripting::StatementDeclarationAssignment* declarationAssignmentStatement = std::get_if<Scripting::StatementDeclarationAssignment>(&statement->Value))
		{
			KG_INFO("{}Declaration/Assignment Statement", GetIndentation(indentation));
			KG_INFO("{}Declared Type", GetIndentation(indentation + 1));
			PrintToken(declarationAssignmentStatement->Type, indentation + 2);
			KG_INFO("{}Declared Identifier", GetIndentation(indentation + 1));
			PrintToken(declarationAssignmentStatement->Name, indentation + 2);
			KG_INFO("{}Assignment Value", GetIndentation(indentation + 1));
			PrintExpression(declarationAssignmentStatement->Value, indentation + 2);
		}
		else if (Scripting::StatementConditional* conditionalStatement = std::get_if<Scripting::StatementConditional>(&statement->Value))
		{
			KG_INFO("{}Conditional Statement", GetIndentation(indentation));
			KG_INFO("{}Type", GetIndentation(indentation + 1));
			KG_INFO("{}{}", GetIndentation(indentation + 2), Utility::ConditionalTypeToString(conditionalStatement->Type));
			KG_INFO("{}Condition Expression", GetIndentation(indentation + 1));
			PrintExpression(conditionalStatement->ConditionExpression, indentation + 2);
			KG_INFO("{}Body Statements", GetIndentation(indentation + 1));
			for (auto bodyStatement : conditionalStatement->BodyStatements)
			{
				PrintStatement(bodyStatement, indentation + 2);
			}
			KG_INFO("{}Chained Conditional Statements", GetIndentation(indentation + 1));
			for (auto chainedStatement : conditionalStatement->ChainedConditionals)
			{
				PrintStatement(chainedStatement, indentation + 2);
			}
		}
		else if (Scripting::StatementWhileLoop* whileLoopStatement = std::get_if<Scripting::StatementWhileLoop>(&statement->Value))
		{
			KG_INFO("{}While Loop Statement", GetIndentation(indentation));
			KG_INFO("{}Condition Expression", GetIndentation(indentation + 1));
			PrintExpression(whileLoopStatement->ConditionExpression, indentation + 2);
			KG_INFO("{}Body Statements", GetIndentation(indentation + 1));
			for (auto bodyStatement : whileLoopStatement->BodyStatements)
			{
				PrintStatement(bodyStatement, indentation + 2);
			}
		}
		else if (Scripting::StatementBreak* breakStatement = std::get_if<Scripting::StatementBreak>(&statement->Value))
		{
			KG_INFO("{}Break Statement", GetIndentation(indentation));
		}
		else if (Scripting::StatementContinue* continueStatement = std::get_if<Scripting::StatementContinue>(&statement->Value))
		{
			KG_INFO("{}Continue Statement", GetIndentation(indentation));
		}
	}

	static void PrintFunction(const Scripting::FunctionNode& funcNode, uint32_t indentation = 0)
	{
		KG_INFO("Function Node");
		KG_INFO("{}Name:", GetIndentation(indentation + 1));
		PrintToken(funcNode.Name, indentation + 2);
		KG_INFO("{}Return Type:", GetIndentation(indentation + 1));
		PrintToken(funcNode.ReturnType, indentation + 2);
		for (auto& parameter : funcNode.Parameters)
		{
			KG_INFO("{}Parameter:", GetIndentation(indentation + 1));
			KG_INFO("{}ParameterType:", GetIndentation(indentation + 2));
			for (auto& token : parameter.AllTypes)
			{
				PrintToken(token, indentation + 3);
			}
			KG_INFO("{}ParameterName:", GetIndentation(indentation + 2));
			PrintToken(parameter.Identifier, indentation + 3);
		}
		for (auto& statement : funcNode.Statements)
		{
			KG_INFO("{}Statement:", GetIndentation(indentation + 1));
			PrintStatement(statement, indentation + 2);
		}
	}
}

namespace Kargono::Scripting
{
	std::tuple<bool, ScriptAST> ScriptTokenParser::ParseTokens(std::vector<ScriptToken>&& tokens)
	{
		m_Tokens = std::move(tokens);

		auto [success, newFunctionNode] = ParseFunctionNode();
		if (!success)
		{
			return { false, m_AST };
		}

		m_AST.m_ProgramNode = { newFunctionNode };

		Advance();
		ScriptToken tokenBuffer = GetCurrentToken();
		if (tokenBuffer.Type != ScriptTokenType::None || tokenBuffer.Value != "End of File")
		{
			StoreParseError(ParseErrorType::Program, "End of file expected", GetToken(0));
			return { false, m_AST };
		}

		return { true, m_AST };
	}

	void ScriptTokenParser::PrintAST()
	{
		if (m_AST.m_ProgramNode)
		{
			FunctionNode& funcNode = m_AST.m_ProgramNode.FuncNode;
			if (funcNode)
			{
				Utility::PrintFunction(funcNode);
			}
		}
	}
	void ScriptTokenParser::PrintTokens()
	{
		for (auto& token : m_Tokens)
		{
			KG_WARN("Token: Type ({}) Value ({}) at {}:{}",
				Utility::ScriptTokenTypeToString(token.Type), token.Value, token.Line, token.Column);
		}
	}
	void ScriptTokenParser::PrintErrors()
	{
		for (auto& error : m_Errors)
		{
			KG_WARN(error.ToString());
		}
	}

	std::tuple<bool, CursorContext> ScriptTokenParser::GetCursorContext()
	{
		if (m_CursorContext)
		{
			return { true, m_CursorContext };
		}

		return { false, {} };
	}


	std::tuple<bool, Ref<Statement>> ScriptTokenParser::ParseStatementNode()
	{
		ScriptToken tokenBuffer = GetCurrentToken();

		// Parse Statement Empty
		{
			auto [success, statement] = ParseStatementEmpty();
			if (success)
			{
				return { success, statement };
			}
		}

		if (CheckForErrors())
		{
			StoreParseError(ParseErrorType::Statement, "Invalid empty statement", tokenBuffer);
			return { false, {} };
		}

		// Parse Statement Return
		{
			auto [success, statement] = ParseStatementReturn();
			if (success)
			{
				return { success, statement };
			}
		}

		if (CheckForErrors())
		{
			StoreParseError(ParseErrorType::Statement, "Invalid return statement", tokenBuffer);
			return { false, {} };
		}

		// Parse Statement Break
		{
			auto [success, statement] = ParseStatementBreak();
			if (success)
			{
				return { success, statement };
			}
		}

		if (CheckForErrors())
		{
			StoreParseError(ParseErrorType::Statement, "Invalid break statement", tokenBuffer);
			return { false, {} };
		}

		// Parse Statement Continue
		{
			auto [success, statement] = ParseStatementContinue();
			if (success)
			{
				return { success, statement };
			}
		}

		if (CheckForErrors())
		{
			StoreParseError(ParseErrorType::Statement, "Invalid continue statement", tokenBuffer);
			return { false, {} };
		}

		// Parse Conditional Statement
		{
			auto [success, statement] = ParseStatementConditional(false);
			if (success)
			{
				return { success, statement };
			}
		}

		if (CheckForErrors())
		{
			StoreParseError(ParseErrorType::Statement, "Invalid if/else/else-if statement", tokenBuffer);
			return { false, {} };
		}

		// Parse While Loop Statement
		{
			auto [success, statement] = ParseStatementWhileLoop();
			if (success)
			{
				return { success, statement };
			}
		}

		if (CheckForErrors())
		{
			StoreParseError(ParseErrorType::Statement, "Invalid while loop statement", tokenBuffer);
			return { false, {} };
		}

		// Parse Statement Expression
		{
			auto [success, statement] = ParseStatementExpression();
			if (success)
			{
				return { success, statement };
			}
		}

		if (CheckForErrors())
		{
			StoreParseError(ParseErrorType::Statement, "Invalid expression statement", tokenBuffer);
			return { false, {} };
		}

		// Parse Statement Declaration
		{
			auto [success, statement] = ParseStatementDeclaration();
			if (success)
			{
				return { success, statement };
			}
		}

		if (CheckForErrors())
		{
			StoreParseError(ParseErrorType::Statement, "Invalid declaration statement", tokenBuffer);
			return { false, {} };
		}

		// Parse Statement Assignment
		{
			auto [success, statement] = ParseStatementAssignment();
			if (success)
			{
				return { success, statement };
			}
		}

		if (CheckForErrors())
		{
			StoreParseError(ParseErrorType::Statement, "Invalid assignment statement", tokenBuffer);
			return { false, {} };
		}

		// Parse Statement Declaration Assignment
		{
			auto [success, statement] = ParseStatementDeclarationAssignment();
			if (success)
			{
				return { success, statement };
			}
		}

		if (CheckForErrors())
		{
			StoreParseError(ParseErrorType::Statement, "Invalid declaration/assignment statement", tokenBuffer);
			return { false, {} };
		}
		Ref<Statement> newStatement = CreateRef<Statement>();
		return { false, newStatement };
	}

	std::tuple<bool, FunctionNode> ScriptTokenParser::ParseFunctionNode()
	{
		FunctionNode newFunctionNode{};
		// Store return value
		ScriptToken tokenBuffer = GetCurrentToken();
		if (tokenBuffer.Type == ScriptTokenType::Keyword && tokenBuffer.Value == "void")
		{
			newFunctionNode.ReturnType = tokenBuffer;
			m_CurrentReturnType = tokenBuffer;
		}
		else if (tokenBuffer.Type == ScriptTokenType::PrimitiveType)
		{
			newFunctionNode.ReturnType = tokenBuffer;
			m_CurrentReturnType = tokenBuffer;
		}
		else
		{
			StoreParseError(ParseErrorType::Function, "Invalid/Empty return type provided for function signature", tokenBuffer);
			return { false, newFunctionNode };
		}
		

		// Get Function Name
		Advance();
		tokenBuffer = GetCurrentToken();
		if (tokenBuffer.Type != ScriptTokenType::Identifier)
		{
			StoreParseError(ParseErrorType::Function, "Invalid/Empty function name provided for function signature", tokenBuffer);
			return { false, newFunctionNode };
		}
		newFunctionNode.Name = { tokenBuffer };

		// Parameter Open Parentheses
		Advance();
		tokenBuffer = GetCurrentToken();
		if (tokenBuffer.Type != ScriptTokenType::OpenParentheses)
		{
			StoreParseError(ParseErrorType::Function, "Expecting an open parentheses in function signature", tokenBuffer);
			return { false, newFunctionNode };
		}

		// Parameter List
		AddStackFrame();
		Advance();
		tokenBuffer = GetCurrentToken();

		if (IsContextProbe(tokenBuffer))
		{
			CursorContext newContext;
			newContext.m_Flags.SetFlag((uint8_t)CursorFlags::AllowAllVariableTypes);
			newContext.m_Flags.SetFlag((uint8_t)CursorFlags::IsFunctionParameter);
			m_CursorContext = newContext;
			StoreParseError(ParseErrorType::ContextProbe, "Found context probe in parameter identifier location", tokenBuffer);
			return { false, {} };
		}

		while (tokenBuffer.Type == ScriptTokenType::PrimitiveType)
		{
			FunctionParameter newParameter{};
			// Store type of current parameter
			newParameter.AllTypes.push_back(tokenBuffer);

			// Check and store parameter name
			Advance();
			tokenBuffer = GetCurrentToken();
			if (tokenBuffer.Type != ScriptTokenType::Identifier)
			{
				StoreParseError(ParseErrorType::Function, "Expecting an identifier for parameter in function signature", tokenBuffer);
				return { false, newFunctionNode };
			}
			newParameter.Identifier = tokenBuffer;
			newFunctionNode.Parameters.push_back(newParameter);
			StoreStackVariable(newParameter.AllTypes.at(0), newParameter.Identifier);

			// Check for comma
			Advance();
			tokenBuffer = GetCurrentToken();
			if (tokenBuffer.Type == ScriptTokenType::Comma)
			{
				if (IsContextProbe(GetCurrentToken(1)))
				{
					CursorContext newContext;
					newContext.m_Flags.SetFlag((uint8_t)CursorFlags::IsFunctionParameter);
					newContext.m_Flags.SetFlag((uint8_t)CursorFlags::AllowAllVariableTypes);
					m_CursorContext = newContext;
					StoreParseError(ParseErrorType::ContextProbe, "Found context probe in parameter identifier location", GetCurrentToken(1));
					return { false, {} };
				}

				if (GetCurrentToken(1).Type != ScriptTokenType::PrimitiveType)
				{
					StoreParseError(ParseErrorType::Function,
						"Expecting a type after comma separator for parameter list in function signature", tokenBuffer);
					return { false, newFunctionNode };
				}

				// Move to next token if comma is present
				Advance();
				tokenBuffer = GetCurrentToken();
			}
		}
		// Check for close parentheses
		tokenBuffer = GetCurrentToken();
		if (tokenBuffer.Type != ScriptTokenType::CloseParentheses)
		{
			if (tokenBuffer.Type != ScriptTokenType::PrimitiveType)
			{
				StoreParseError(ParseErrorType::Function, "Invalid parameter type declaration", tokenBuffer);
				return { false, newFunctionNode };
			}
			StoreParseError(ParseErrorType::Function, "Expecting an closing parentheses in function signature", tokenBuffer);
			return { false, newFunctionNode };
		}

		// Check for open curly braces
		Advance();
		tokenBuffer = GetCurrentToken();
		if (tokenBuffer.Type != ScriptTokenType::OpenCurlyBrace)
		{
			StoreParseError(ParseErrorType::Function, "Expecting an opening curly brace in function signature", tokenBuffer);
			return { false, newFunctionNode };
		}

		// Parse Statements
		Advance();
		bool success = false;
		Ref<Statement> statement {nullptr};
		do
		{
			std::tie(success, statement) = ParseStatementNode();
			if (success)
			{
				newFunctionNode.Statements.push_back(statement);
			}
		} while (success);

		if (CheckForErrors())
		{
			StoreParseError(ParseErrorType::Function, "Unable to parse function body statements", newFunctionNode.Name);
			return { false, newFunctionNode };
		}

		// Check for close curly braces
		tokenBuffer = GetCurrentToken();
		if (tokenBuffer.Type != ScriptTokenType::CloseCurlyBrace)
		{
			if (tokenBuffer.Type == ScriptTokenType::None && tokenBuffer.Value == "End of File")
			{
				StoreParseError(ParseErrorType::Function, "Expecting a closing curly brace to terminate function, however, the file ended abruptly\n Did you forget a closing curly brace somewhere?", newFunctionNode.Name);
			}
			else
			{
				StoreParseError(ParseErrorType::Function, "Expecting a closing curly brace to terminate function body\n Did you forget a semicolon?", tokenBuffer);
			}
			return { false, newFunctionNode };
		}

		// Remove current stack variables
		PopStackFrame();

		return { true, newFunctionNode };
	}
	std::tuple<bool, Ref<Expression>> ScriptTokenParser::ParseExpressionNode(uint32_t& parentExpressionSize)
	{
		Ref<Expression> newExpression { nullptr };

		// Parse initial expression term / initial operand if within binary expression
		{
			auto [success, expression] = ParseExpressionTerm(parentExpressionSize);
			if (!success)
			{
				return { false, {} };
			}
			if (CheckForErrors())
			{
				StoreParseError(ParseErrorType::Expression, "Invalid expression", GetCurrentToken(parentExpressionSize));
				return { false, {} };
			}
			newExpression = expression;
		}

		// Parse Ternary Operation
		{
			auto [success, expression] = ParseExpressionTernaryOperation(newExpression, parentExpressionSize);
			if (success)
			{
				return { true, expression };
			}
			if (CheckForErrors())
			{
				StoreParseError(ParseErrorType::Expression, "Invalid ternary operation", GetCurrentToken(parentExpressionSize));
				return { false, {} };
			}
		}

		// Check for addition / subtraction / boolean binary operations
		while (ScriptCompilerService::IsAdditionOrSubtractionOperator(GetCurrentToken(parentExpressionSize)) ||
			ScriptCompilerService::IsBooleanOperator(GetCurrentToken(parentExpressionSize)))
		{
			Ref<Expression> newBinaryExpression{ CreateRef<Expression>() };
			BinaryOperationNode newBinaryOperation{};

			// Store first operand and operator in newBinaryOperation
			newBinaryOperation.LeftOperand = newExpression;
			newBinaryOperation.Operator = GetCurrentToken(parentExpressionSize);

			// Parse next operand of binary expression
			{
				parentExpressionSize++;
				auto [success, expression] = ParseExpressionTerm(parentExpressionSize);
				if (!success)
				{
					return { false, {} };
				}
				if (CheckForErrors())
				{
					StoreParseError(ParseErrorType::Expression, "Invalid right operand in binary expression", GetCurrentToken(parentExpressionSize));
					return { false, {} };
				}
				newBinaryOperation.RightOperand = expression;
			}

			if (IsContextProbe(newBinaryOperation.LeftOperand))
			{
				CursorContext newContext;
				newContext.AllReturnTypes.push_back(GetPrimitiveTypeFromToken(newBinaryOperation.RightOperand->GetReturnType()));
				if (newContext.AllReturnTypes.size() == 0)
				{
					newContext.m_Flags.SetFlag((uint8_t)CursorFlags::AllowAllVariableTypes);
				}
				newContext.StackVariables = m_StackVariables;
				m_CursorContext = newContext;
				StoreParseError(ParseErrorType::ContextProbe, "Found context probe in left operand of addition/subtraction operation", newBinaryOperation.Operator);
				return { false, {} };
			}

			if (IsContextProbe(newBinaryOperation.RightOperand))
			{
				CursorContext newContext;
				newContext.AllReturnTypes.push_back(GetPrimitiveTypeFromToken(newBinaryOperation.LeftOperand->GetReturnType()));
				if (newContext.AllReturnTypes.size() == 0)
				{
					newContext.m_Flags.SetFlag((uint8_t)CursorFlags::AllowAllVariableTypes);
				}
				newContext.StackVariables = m_StackVariables;
				m_CursorContext = newContext;
				StoreParseError(ParseErrorType::ContextProbe, "Found context probe in right operand of addition/subtraction operation", newBinaryOperation.Operator);
				return { false, {} };
			}

			PrimitiveType leftOperandType = ScriptCompilerService::s_ActiveLanguageDefinition.PrimitiveTypes.at(GetPrimitiveTypeFromToken(newBinaryOperation.LeftOperand->GetReturnType()).Value);
			PrimitiveType rightOperandType = ScriptCompilerService::s_ActiveLanguageDefinition.PrimitiveTypes.at(GetPrimitiveTypeFromToken(newBinaryOperation.RightOperand->GetReturnType()).Value);

			// Ensure the return type of both operands is identical
			if (ScriptCompilerService::IsBooleanOperator(newBinaryOperation.Operator))
			{
				// Check boolean operator operands
				if (leftOperandType.Name != "bool" || rightOperandType.Name != "bool")
				{
					std::string errorMessage = fmt::format("Invalid boolean operation. Boolean operations require boolean operands.\n Left operand return type: {}\n Right operand return type: {}",
						GetPrimitiveTypeFromToken(newBinaryOperation.LeftOperand->GetReturnType()).Value,
						GetPrimitiveTypeFromToken(newBinaryOperation.RightOperand->GetReturnType()).Value);
					StoreParseError(ParseErrorType::Expression, errorMessage, newBinaryOperation.Operator);
					return { false, {} };
				}

				ScriptToken boolToken;
				boolToken.Type = ScriptTokenType::PrimitiveType;
				boolToken.Value = "bool";
				newBinaryOperation.ReturnType = boolToken;
			}
			else if (ScriptCompilerService::IsAdditionOrSubtractionOperator(newBinaryOperation.Operator))
			{
				if (PrimitiveTypeAcceptableToken(leftOperandType.Name, GetPrimitiveTypeFromToken(newBinaryOperation.RightOperand->GetReturnType())))
				{
					// Store Return Type
					newBinaryOperation.ReturnType = GetPrimitiveTypeFromToken(newBinaryOperation.LeftOperand->GetReturnType());
				}
				// If not, check if either primitive type accepts a different type of data to do arithmetic operations
				else if (leftOperandType.AcceptableArithmetic.contains(rightOperandType.Name) || rightOperandType.AcceptableArithmetic.contains(leftOperandType.Name))
				{
					// Store Return Type
					bool leftFocused = leftOperandType.AcceptableArithmetic.contains(rightOperandType.Name);
					bool rightFocused = rightOperandType.AcceptableArithmetic.contains(leftOperandType.Name);
					if ((leftFocused && rightFocused) || leftFocused)
					{
						newBinaryOperation.ReturnType = GetPrimitiveTypeFromToken(newBinaryOperation.LeftOperand->GetReturnType());
					}
					else
					{
						newBinaryOperation.ReturnType = GetPrimitiveTypeFromToken(newBinaryOperation.RightOperand->GetReturnType());
					}
				}
				else
				{
					std::string errorMessage = fmt::format("Return types do not match in binary expression\n Left operand return type: {}\n Right operand return type: {}",
						GetPrimitiveTypeFromToken(newBinaryOperation.LeftOperand->GetReturnType()).Value,
						GetPrimitiveTypeFromToken(newBinaryOperation.RightOperand->GetReturnType()).Value);
					StoreParseError(ParseErrorType::Expression, errorMessage, newBinaryOperation.Operator);
					return { false, {} };
				}
			}
			newBinaryExpression->Value = newBinaryOperation;
			newExpression = newBinaryExpression;
		}


		return { true, newExpression };
	}
	std::tuple<bool, Ref<Expression>> ScriptTokenParser::ParseExpressionTerm(uint32_t& parentExpressionSize, bool checkBinaryOperations)
	{
		Ref<Expression> newExpression {nullptr};
		bool foundValidExpression = false;

		// Parse Expression Unary Operation
		if (!foundValidExpression)
		{
			auto [success, expression] = ParseExpressionUnaryOperation(parentExpressionSize);
			if (success)
			{
				newExpression = expression;
				foundValidExpression = true;
			}
			if (CheckForErrors())
			{
				StoreParseError(ParseErrorType::Expression, "Invalid unary operation", GetCurrentToken(parentExpressionSize));
				return { false, {} };
			}
		}

		// Parse Expression Function Call
		if (!foundValidExpression)
		{
			auto [success, expression] = ParseExpressionFunctionCall(parentExpressionSize);
			if (success)
			{
				newExpression = expression;
				foundValidExpression = true;
			}
			if (CheckForErrors())
			{
				StoreParseError(ParseErrorType::Expression, "Invalid function call", GetCurrentToken(parentExpressionSize));
				return { false, {} };
			}
		}

		// Parse Expression Asset
		if (!foundValidExpression)
		{
			auto [success, expression] = ParseExpressionCustomLiteral(parentExpressionSize);
			if (success)
			{
				newExpression = expression;
				foundValidExpression = true;
			}
			if (CheckForErrors())
			{
				StoreParseError(ParseErrorType::Expression, "Invalid custom literal identifier", GetCurrentToken(parentExpressionSize));
				return { false, {} };
			}
		}

		// Parse Expression Initialization List
		if (!foundValidExpression)
		{
			auto [success, expression] = ParseExpressionInitializationList(parentExpressionSize);
			if (success)
			{
				newExpression = expression;
				foundValidExpression = true;
			}
			if (CheckForErrors())
			{
				StoreParseError(ParseErrorType::Expression, "Invalid initialization list", GetCurrentToken(parentExpressionSize));
				return { false, {} };
			}
		}

		// Parse Expression Member
		if (!foundValidExpression)
		{
			auto [success, expression] = ParseExpressionMember(parentExpressionSize);
			if (success)
			{
				newExpression = expression;
				foundValidExpression = true;
			}
			if (CheckForErrors())
			{
				StoreParseError(ParseErrorType::Expression, "Invalid member", GetCurrentToken(parentExpressionSize));
				return { false, {} };
			}
		}

		// Parse Expression Literal
		if (!foundValidExpression)
		{
			auto [success, expression] = ParseExpressionLiteral(parentExpressionSize);
			if (success)
			{
				newExpression = expression;
				foundValidExpression = true;
			}
			if (CheckForErrors())
			{
				StoreParseError(ParseErrorType::Expression, "Invalid literal", GetCurrentToken(parentExpressionSize));
				return { false, {} };
			}
		}

		// Parse Expression Identifier
		if (!foundValidExpression)
		{
			auto [success, expression] = ParseExpressionIdentifier(parentExpressionSize);
			if (success)
			{
				newExpression = expression;
				foundValidExpression = true;
			}
			if (CheckForErrors())
			{
				StoreParseError(ParseErrorType::Expression, "Invalid identifier", GetCurrentToken(parentExpressionSize));
				return { false, {} };
			}
		}

		// Initial Expression term not found!
		if (!foundValidExpression)
		{
			return { false, {} };
		}

		if (checkBinaryOperations)
		{
			// Check for multiplication / division / comparison binary operations
			while (ScriptCompilerService::IsMultiplicationOrDivisionOperator(GetCurrentToken(parentExpressionSize)) ||
				ScriptCompilerService::IsComparisonOperator(GetCurrentToken(parentExpressionSize)))
			{
				Ref<Expression> newBinaryExpression{ CreateRef<Expression>() };
				BinaryOperationNode newBinaryOperation{};

				// Store first operand and operator in newBinaryOperation
				newBinaryOperation.LeftOperand = newExpression;
				newBinaryOperation.Operator = GetCurrentToken(parentExpressionSize);

				// Parse next operand of binary expression
				{
					parentExpressionSize++;
					auto [success, expression] = ParseExpressionTerm(parentExpressionSize, false);
					if (!success || CheckForErrors())
					{
						return { false, {} };
					}
					newBinaryOperation.RightOperand = expression;
				}

				// Check for Context Probe
				if (IsContextProbe(newBinaryOperation.LeftOperand))
				{
					CursorContext newContext;
					newContext.AllReturnTypes.push_back(GetPrimitiveTypeFromToken(newBinaryOperation.RightOperand->GetReturnType()));
					if (newContext.AllReturnTypes.size() == 0)
					{
						newContext.m_Flags.SetFlag((uint8_t)Kargono::Scripting::CursorFlags::AllowAllVariableTypes);
					}
					newContext.StackVariables = m_StackVariables;
					m_CursorContext = newContext;
					StoreParseError(ParseErrorType::ContextProbe, "Found context probe in left operand of multiplication/division/comparison operation", newBinaryOperation.Operator);
					return { false, {} };
				}

				if (IsContextProbe(newBinaryOperation.RightOperand))
				{
					CursorContext newContext;
					newContext.AllReturnTypes.push_back(GetPrimitiveTypeFromToken(newBinaryOperation.LeftOperand->GetReturnType()));
					if (newContext.AllReturnTypes.size() == 0)
					{
						newContext.m_Flags.SetFlag((uint8_t)Kargono::Scripting::CursorFlags::AllowAllVariableTypes);
					}
					newContext.StackVariables = m_StackVariables;
					m_CursorContext = newContext;
					StoreParseError(ParseErrorType::ContextProbe, "Found context probe in right operand of multiplication/division/comparison operation", newBinaryOperation.Operator);
					return { false, {} };
				}

				// Get return types of each operand
				ScriptToken leftOperandReturnToken = GetPrimitiveTypeFromToken(newBinaryOperation.LeftOperand->GetReturnType());
				ScriptToken rightOperandReturnToken = GetPrimitiveTypeFromToken(newBinaryOperation.RightOperand->GetReturnType());

				// Ensure no none types are provided
				if (leftOperandReturnToken.Type == ScriptTokenType::None || rightOperandReturnToken.Type == ScriptTokenType::None)
				{
					StoreParseError(ParseErrorType::ContextProbe, "None type found while parsing binary expression", newBinaryOperation.Operator);
					return { false, {} };
				}

				PrimitiveType leftOperandType = ScriptCompilerService::s_ActiveLanguageDefinition.PrimitiveTypes.at(leftOperandReturnToken.Value);
				PrimitiveType rightOperandType = ScriptCompilerService::s_ActiveLanguageDefinition.PrimitiveTypes.at(rightOperandReturnToken.Value);

				// Ensure the return type of both operands is identical
				if (PrimitiveTypeAcceptableToken(leftOperandType.Name, GetPrimitiveTypeFromToken(newBinaryOperation.RightOperand->GetReturnType())))
				{
					// Store Return Type
					if (ScriptCompilerService::IsComparisonOperator(newBinaryOperation.Operator))
					{
						ScriptToken boolToken;
						boolToken.Type = ScriptTokenType::PrimitiveType;
						boolToken.Value = "bool";
						newBinaryOperation.ReturnType = boolToken;
					}
					else
					{
						newBinaryOperation.ReturnType = GetPrimitiveTypeFromToken(newBinaryOperation.LeftOperand->GetReturnType());
					}
				}
				// If not, check if either primitive type accepts a different type of data to do arithmetic operations
				else if (leftOperandType.AcceptableArithmetic.contains(rightOperandType.Name) || rightOperandType.AcceptableArithmetic.contains(leftOperandType.Name))
				{
					// Store Return Type
					if (!ScriptCompilerService::IsComparisonOperator(newBinaryOperation.Operator))
					{
						bool leftFocused = leftOperandType.AcceptableArithmetic.contains(rightOperandType.Name);
						bool rightFocused = rightOperandType.AcceptableArithmetic.contains(leftOperandType.Name);
						if ((leftFocused && rightFocused) || leftFocused)
						{
							newBinaryOperation.ReturnType = GetPrimitiveTypeFromToken(newBinaryOperation.LeftOperand->GetReturnType());
						}
						else
						{
							newBinaryOperation.ReturnType = GetPrimitiveTypeFromToken(newBinaryOperation.RightOperand->GetReturnType());
						}
					}
					else
					{
						std::string errorMessage = fmt::format("Cannot use comparison operators for acceptable arithmetic list of primitive types\n Left operand return type: {}\n Right operand return type: {}",
							GetPrimitiveTypeFromToken(newBinaryOperation.LeftOperand->GetReturnType()).Value,
							GetPrimitiveTypeFromToken(newBinaryOperation.RightOperand->GetReturnType()).Value);
						StoreParseError(ParseErrorType::Expression, errorMessage, newBinaryOperation.Operator);
						return { false, {} };
					}
				}
				else
				{
					std::string errorMessage = fmt::format("Return types do not match in binary expression\n Left operand return type: {}\n Right operand return type: {}",
						GetPrimitiveTypeFromToken(newBinaryOperation.LeftOperand->GetReturnType()).Value,
						GetPrimitiveTypeFromToken(newBinaryOperation.RightOperand->GetReturnType()).Value);
					StoreParseError(ParseErrorType::Expression, errorMessage, newBinaryOperation.Operator);
					return { false, {} };
				}

				newBinaryExpression->Value = newBinaryOperation;
				newExpression = newBinaryExpression;
			}
		}

		return { true, newExpression };
	}
	std::tuple<bool, Ref<Expression>> ScriptTokenParser::ParseExpressionLiteral(uint32_t& parentExpressionSize)
	{
		// Check for a single literal
		TokenExpressionNode tokenNode;
		tokenNode.Value = GetCurrentToken(parentExpressionSize);
		if (!ScriptCompilerService::IsLiteral(GetCurrentToken(parentExpressionSize)))
		{
			return { false, {} };
		}

		// Get return type for literal
		tokenNode.ReturnType = GetPrimitiveTypeFromToken(tokenNode.Value);

		// Store new expression and return
		Ref<Expression> newExpression { CreateRef<Expression>()};
		newExpression->Value = tokenNode;
		parentExpressionSize++;
		return { true, newExpression };
	}
	std::tuple<bool, Ref<Expression>> ScriptTokenParser::ParseExpressionIdentifier(uint32_t& parentExpressionSize)
	{
		// Check for a single identifier
		TokenExpressionNode tokenNode;
		if ((GetCurrentToken(parentExpressionSize).Type != ScriptTokenType::Identifier))
		{
			return { false, {} };
		}
		tokenNode.Value = GetCurrentToken(parentExpressionSize);

		// Ensure identifier is a proper value
		if (!IsContextProbe(tokenNode.Value) && tokenNode.Value.Type == ScriptTokenType::Identifier && !CheckStackForIdentifier(tokenNode.Value))
		{
			StoreParseError(ParseErrorType::Expression, "Unknown variable", tokenNode.Value);
			return { false, {} };
		}

		// Get return type from variable
		StackVariable currentVariable = GetStackVariable(tokenNode.Value);
		tokenNode.ReturnType = currentVariable.Type;

		// Store new expression and return
		Ref<Expression> newExpression{ CreateRef<Expression>() };
		newExpression->Value = tokenNode;
		parentExpressionSize++;
		return { true, newExpression };
	}
	std::tuple<bool, Ref<Expression>> ScriptTokenParser::ParseExpressionFunctionCall(uint32_t& parentExpressionSize)
	{
		Ref<Expression> newFunctionExpression{ CreateRef<Expression>() };
		FunctionCallNode newFunctionCallNode{};

		// Check for function namespace, namespace resolver symbol, function identifier, and open parentheses
		ScriptToken tokenBuffer = GetCurrentToken(parentExpressionSize);
		int32_t initialAdvance{ 0 };
		if (tokenBuffer.Type == ScriptTokenType::Identifier &&
			GetCurrentToken(parentExpressionSize + 1).Type == ScriptTokenType::NamespaceResolver &&
			GetCurrentToken(parentExpressionSize + 2).Type == ScriptTokenType::Identifier &&
			GetCurrentToken(parentExpressionSize + 3).Type == ScriptTokenType::OpenParentheses)
		{
			newFunctionCallNode.Namespace = tokenBuffer;
			newFunctionCallNode.Identifier = GetCurrentToken(parentExpressionSize + 2);
			initialAdvance = 4;
		}
		else if (tokenBuffer.Type == ScriptTokenType::Identifier &&
			GetCurrentToken(parentExpressionSize + 1).Type == ScriptTokenType::OpenParentheses)
		{
			newFunctionCallNode.Identifier = tokenBuffer;
			initialAdvance = 2;
		}
		else if (tokenBuffer.Type == ScriptTokenType::Identifier &&
			GetCurrentToken(parentExpressionSize + 1).Type == ScriptTokenType::NamespaceResolver &&
			IsContextProbe(GetCurrentToken(parentExpressionSize + 2)))
		{
			// Ensure namespace identifier exists
			if (!ScriptCompilerService::s_ActiveLanguageDefinition.NamespaceDescriptions.contains(tokenBuffer.Value))
			{
				StoreParseError(ParseErrorType::ContextProbe, "Found context probe, however, namespace node is invalid", tokenBuffer);
				return { false, {} };
			}
			// Store context probe for argument
			CursorContext newContext;
			newContext.m_Flags.SetFlag((uint8_t)Kargono::Scripting::CursorFlags::AllowAllVariableTypes);
			newContext.m_Flags.SetFlag((uint8_t)CursorFlags::AfterNamespaceResolution);
			newContext.CurrentNamespace = tokenBuffer;
			m_CursorContext = newContext;
			StoreParseError(ParseErrorType::ContextProbe, "Found context probe for function namespace", tokenBuffer);
			return { false, {} };
		}
		else
		{
			return { false, {} };
		}

		// Check for arguments and commas
		uint32_t currentArgumentLocation = parentExpressionSize + initialAdvance;
		{
			bool continueLoop;
			do
			{
				continueLoop = false;
				// Parse Expression Node, store expression, and check for context probe
				auto [success, expression] = ParseExpressionNode(currentArgumentLocation);
				if (success)
				{
					newFunctionCallNode.Arguments.push_back(expression);
					if (IsContextProbe(expression))
					{
						// Ensure function identifier exists and get function node
						if (!ScriptCompilerService::s_ActiveLanguageDefinition.FunctionDefinitions.contains(newFunctionCallNode.Identifier.Value))
						{
							StoreParseError(ParseErrorType::ContextProbe, "Found context probe, however, function node is invalid", newFunctionCallNode.Identifier);
							return { false, {} };
						}
						// Ensure the current argument is not overflowing the parameter list
						FunctionNode& functionNode = ScriptCompilerService::s_ActiveLanguageDefinition.FunctionDefinitions.at(newFunctionCallNode.Identifier.Value);
						if (newFunctionCallNode.Arguments.size() > functionNode.Parameters.size())
						{
							StoreParseError(ParseErrorType::ContextProbe, "Found context probe, however, there are too many arguments for function node", newFunctionCallNode.Identifier);
							return { false, {} };
						}
						// Store context probe for argument
						CursorContext newContext;
						newContext.AllReturnTypes = functionNode.Parameters.at(newFunctionCallNode.Arguments.size() - 1).AllTypes;
						if (newContext.AllReturnTypes.size() == 0)
						{
							newContext.m_Flags.SetFlag((uint8_t)Kargono::Scripting::CursorFlags::AllowAllVariableTypes);
						}
						newContext.StackVariables = m_StackVariables;
						m_CursorContext = newContext;
						StoreParseError(ParseErrorType::ContextProbe, "Found context probe inside function argument", newFunctionCallNode.Identifier);
						return { false, {} };
					}
				}

				if (CheckForErrors())
				{
					StoreParseError(ParseErrorType::Expression, "Invalid function argument", GetCurrentToken(currentArgumentLocation));
					return { false, {} };
				}

				// Decide whether to continue looking for more arguments
				if (success && GetCurrentToken(currentArgumentLocation).Type == ScriptTokenType::Comma)
				{
					currentArgumentLocation++;
					continueLoop = true;
				}

			} while (continueLoop);
		}

		// Check for closing parentheses
		if (GetCurrentToken(currentArgumentLocation).Type != ScriptTokenType::CloseParentheses)
		{
			return { false, {} };
		}
		currentArgumentLocation++;

		// Ensure function identifier exists and get function node
		if (!ScriptCompilerService::s_ActiveLanguageDefinition.FunctionDefinitions.contains(newFunctionCallNode.Identifier.Value))
		{
			StoreParseError(ParseErrorType::Expression, "Unknown function identifier", newFunctionCallNode.Identifier);
			return { false, {} };
		}
		FunctionNode& functionNode = ScriptCompilerService::s_ActiveLanguageDefinition.FunctionDefinitions.at(newFunctionCallNode.Identifier.Value);

		// Ensure namespace of function matches
		if (functionNode.Namespace.Value != newFunctionCallNode.Namespace.Value)
		{
			StoreParseError(ParseErrorType::Expression, "Unknown function namespace identifier", newFunctionCallNode.Namespace);
			return { false, {} };
		}

		// Ensure number of arguments in function call match the number of arguments in the function
		if (functionNode.Parameters.size() != newFunctionCallNode.Arguments.size())
		{
			std::string errorMessage =
				fmt::format("Argument count in function call ({}) does not match the function parameter count ({})",
					newFunctionCallNode.Arguments.size(), functionNode.Parameters.size());
			StoreParseError(ParseErrorType::Expression, errorMessage, newFunctionCallNode.Identifier);
			return { false, {} };
		}

		// Ensure each argument type matches the parameter type
		uint32_t parameterIteration{ 0 };
		for (auto& parameter : functionNode.Parameters)
		{
			bool valid = false;

			for (auto& type : parameter.AllTypes)
			{
				if (PrimitiveTypeAcceptableToken(type.Value, newFunctionCallNode.Arguments.at(parameterIteration)->GetReturnType()))
				{
					valid = true;
				}
			}
			if (!valid)
			{
				std::string errorMessage =
					fmt::format("Argument type is not acceptable for function parameter\n Argument Type: {}\n Parameter Type(s):",
						GetPrimitiveTypeFromToken((newFunctionCallNode.Arguments.at(parameterIteration)->GetReturnType())).Value);
				for (auto& type : parameter.AllTypes)
				{
					errorMessage = errorMessage + "\n " + type.Value;
				}
				StoreParseError(ParseErrorType::Expression, errorMessage, newFunctionCallNode.Arguments.at(parameterIteration)->GetReturnType());
				return { false, {} };
			}
			parameterIteration++;
		}

		// Get return type from function node and emplace it into the functionCallNode
		newFunctionCallNode.ReturnType = functionNode.ReturnType;

		// Fill the expression buffer and exit
		newFunctionExpression->Value = newFunctionCallNode;
		parentExpressionSize = currentArgumentLocation;
		return { true, newFunctionExpression };
	}
	std::tuple<bool, Ref<Expression>> ScriptTokenParser::ParseExpressionCustomLiteral(uint32_t& parentExpressionSize)
	{
		Ref<Expression> newCustomLiteralExpression{ CreateRef<Expression>() };
		CustomLiteralNode newAssetNode{};

		// Check for asset namespace, namespace resolver symbol, and custom literal identifier
		ScriptToken tokenBuffer = GetCurrentToken(parentExpressionSize);
		int32_t initialAdvance{ 0 };
		if (tokenBuffer.Type == ScriptTokenType::Identifier &&
			GetCurrentToken(parentExpressionSize + 1).Type == ScriptTokenType::NamespaceResolver &&
			GetCurrentToken(parentExpressionSize + 2).Type == ScriptTokenType::CustomLiteral)
		{
			newAssetNode.Namespace = tokenBuffer;
			newAssetNode.Identifier = GetCurrentToken(parentExpressionSize + 2);
			initialAdvance = 3;
		}
		else
		{
			return { false, {} };
		}

		// Check for context probe
		if (IsContextProbe(GetCurrentToken(parentExpressionSize + 2)))
		{
			// Ensure namespace identifier exists
			if (!ScriptCompilerService::s_ActiveLanguageDefinition.NamespaceDescriptions.contains(tokenBuffer.Value))
			{
				StoreParseError(ParseErrorType::ContextProbe, "Found context probe, however, namespace node is invalid", tokenBuffer);
				return { false, {} };
			}
			// Store context probe for argument
			CursorContext newContext;
			newContext.m_Flags.SetFlag((uint8_t)Kargono::Scripting::CursorFlags::AllowAllVariableTypes);
			newContext.m_Flags.SetFlag((uint8_t)CursorFlags::AfterNamespaceResolution);
			newContext.CurrentNamespace = tokenBuffer;
			m_CursorContext = newContext;
			StoreParseError(ParseErrorType::ContextProbe, "Found context probe for function namespace", tokenBuffer);
			return { false, {} };
		}

		// Ensure asset namespace exists
		if (!ScriptCompilerService::s_ActiveLanguageDefinition.AllLiteralTypes.contains(newAssetNode.Namespace.Value))
		{
			StoreParseError(ParseErrorType::Expression, "Unknown custom literal type provided", newAssetNode.Namespace);
			return { false, {} };
		}

		// Get the asset information
		CustomLiteralInfo& assetInfo{ ScriptCompilerService::s_ActiveLanguageDefinition.AllLiteralTypes.at(newAssetNode.Namespace.Value) };

		// Get the asset map appropriate for this asset type
		CustomLiteralNameToIDMap& assetMap = assetInfo.m_CustomLiteralNameToID;

		// Ensure the asset identifier is valid
		if (!assetMap.contains(newAssetNode.Identifier.Value))
		{
			StoreParseError(ParseErrorType::Expression, "Unknown custom type identifier provided", newAssetNode.Identifier);
			return { false, {} };
		}

		// Get the script member to indicate return type
		CustomLiteralMember& literalMember = assetMap.at(newAssetNode.Identifier.Value);

		// Get return type from function node and emplace it into the assetNode
		newAssetNode.ReturnType = literalMember.m_PrimitiveType;

		// Fill the expression buffer and exit
		newCustomLiteralExpression->Value = newAssetNode;
		parentExpressionSize += initialAdvance;
		return { true, newCustomLiteralExpression };
	}

	std::tuple<bool, Ref<Expression>> ScriptTokenParser::ParseExpressionUnaryOperation(uint32_t& parentExpressionSize)
	{
		Ref<Expression> newExpression{ CreateRef<Expression>() };
		UnaryOperationNode newUnaryOperation{};

		// Check for operator
		if (!ScriptCompilerService::IsUnaryOperator(GetCurrentToken(parentExpressionSize)))
		{
			return { false, {} };
		}
		newUnaryOperation.Operator = GetCurrentToken(parentExpressionSize);

		// Check for operand
		if (!ScriptCompilerService::IsLiteralOrIdentifier(GetCurrentToken(parentExpressionSize + 1)))
		{
			return { false, {} };
		}
		newUnaryOperation.Operand = GetCurrentToken(parentExpressionSize + 1);

		// Fill return value
		if (newUnaryOperation.Operator.Type == ScriptTokenType::NegationOperator)
		{
			if (GetPrimitiveTypeFromToken(newUnaryOperation.Operand).Value != "bool")
			{
				StoreParseError(ParseErrorType::Expression, "Expecting a bool type for negation operator", newUnaryOperation.Operand);
				return { false, {} };
			}
			newUnaryOperation.ReturnType = { ScriptTokenType::PrimitiveType, "bool" };
		}
		else
		{
			newUnaryOperation.ReturnType = GetPrimitiveTypeFromToken(newUnaryOperation.Operand);
		}

		// Fill the expression buffer and exit
		newExpression->Value = newUnaryOperation;
		parentExpressionSize += 2;
		return { true, newExpression };
	}
	std::tuple<bool, Ref<Expression>> ScriptTokenParser::ParseExpressionInitializationList(uint32_t& parentExpressionSize)
	{
		Ref<Expression> newInitListExpression{ CreateRef<Expression>() };
		InitializationListNode initListNode{};

		// Check for first open curly brace
		ScriptToken tokenBuffer = GetCurrentToken(parentExpressionSize);
		if (tokenBuffer.Type != ScriptTokenType::OpenCurlyBrace)
		{
			return { false, nullptr };
		}

		// Check for arguments and commas
		uint32_t currentArgumentLocation = parentExpressionSize + 1;
		{
			bool continueLoop;
			do 
			{
				continueLoop = false;
				// Parse Expression Node and store expression
				auto [success, expression] = ParseExpressionNode(currentArgumentLocation);
				if (success)
				{
					initListNode.Arguments.push_back(expression);
					if (IsContextProbe(expression))
					{
						// Store context probe for argument
						CursorContext newContext;
						newContext.StackVariables = m_StackVariables;
						newContext.m_Flags.SetFlag((uint8_t)Kargono::Scripting::CursorFlags::AllowAllVariableTypes);
						m_CursorContext = newContext;
						StoreParseError(ParseErrorType::ContextProbe, "Found context probe inside initialization list argument", GetCurrentToken(currentArgumentLocation));
						return { false, {} };
					}
				}

				if (CheckForErrors())
				{
					StoreParseError(ParseErrorType::Expression, "Invalid initialization list argument", GetCurrentToken(currentArgumentLocation));
					return { false, nullptr };
				}

				// Decide whether to continue looking for more arguments
				if (success && GetCurrentToken(currentArgumentLocation).Type == ScriptTokenType::Comma)
				{
					currentArgumentLocation++;
					continueLoop = true;
				}

			} while (continueLoop);
		}

		// Check for closing curly brace
		if (GetCurrentToken(currentArgumentLocation).Type != ScriptTokenType::CloseCurlyBrace)
		{
			return { false, nullptr };
		}
		currentArgumentLocation++;

		// Search for initialization list type with identical parameters
		InitializationListType* foundListType{ nullptr };
		for (InitializationListType& listType : ScriptCompilerService::s_ActiveLanguageDefinition.InitListTypes)
		{
			if (listType.ParameterTypes.size() == initListNode.Arguments.size())
			{
				for (uint32_t iteration{ 0 }; iteration < listType.ParameterTypes.size(); ++iteration)
				{
					if (listType.ParameterTypes.at(iteration).Value == GetPrimitiveTypeFromToken(initListNode.Arguments.at(iteration)->GetReturnType()).Value)
					{
						foundListType = &listType;
						break;
					}
				}
			}
			if (foundListType)
			{
				break;
			}
		}

		if (!foundListType)
		{
			StoreParseError(ParseErrorType::Expression, "Could not locate matching type for arguments provided", GetCurrentToken(parentExpressionSize));
			return { false, nullptr };
		}

		// Get return type from function node and emplace it into the initListNode
		initListNode.ReturnType = foundListType->ReturnType;

		// Fill the expression buffer and exit
		newInitListExpression->Value = initListNode;
		parentExpressionSize = currentArgumentLocation;
		return { true, newInitListExpression };
	}
	std::tuple<bool, Ref<Expression>> ScriptTokenParser::ParseExpressionTernaryOperation(Ref<Expression> currentExpression, uint32_t& parentExpressionSize)
	{
		TernaryOperationNode ternaryOperationNode{};
		uint32_t currentLocation = parentExpressionSize;

		// Check for an initial conditional expression
		if (currentExpression->GetReturnType().Value != "bool")
		{
			return { false, {} };
		}
		ternaryOperationNode.Conditional = currentExpression;

		
		// Check for conditional operator
		if (GetCurrentToken(currentLocation).Type != ScriptTokenType::ConditionalOperator)
		{
			return { false, nullptr };
		}
		currentLocation++;

		// Parse first ternary option
		{
			auto [success, expression] = ParseExpressionTerm(currentLocation);
			if (!success)
			{
				return { false, {} };
			}
			if (CheckForErrors())
			{
				StoreParseError(ParseErrorType::Expression, "Invalid first option of ternary expression", GetCurrentToken(currentLocation));
				return { false, {} };
			}
			ternaryOperationNode.OptionOne = expression;
		}

		// Check for precedence operator
		if (GetCurrentToken(currentLocation).Type != ScriptTokenType::PrecedenceOperator)
		{
			return { false, nullptr };
		}
		currentLocation++;

		// Parse second ternary option
		{
			auto [success, expression] = ParseExpressionTerm(currentLocation);
			if (!success)
			{
				return { false, {} };
			}
			if (CheckForErrors())
			{
				StoreParseError(ParseErrorType::Expression, "Invalid second option of ternary expression", GetCurrentToken(currentLocation));
				return { false, {} };
			}
			ternaryOperationNode.OptionTwo = expression;
		}


		// Get return type from ternary operator
		if (ternaryOperationNode.OptionOne->GetReturnType().Value == ternaryOperationNode.OptionTwo->GetReturnType().Value ||
			PrimitiveTypeAcceptableToken(ternaryOperationNode.OptionOne->GetReturnType().Value, ternaryOperationNode.OptionTwo->GetReturnType()))
		{
			ternaryOperationNode.ReturnType = ternaryOperationNode.OptionOne->GetReturnType();
		}
		else if (PrimitiveTypeAcceptableToken(ternaryOperationNode.OptionTwo->GetReturnType().Value, ternaryOperationNode.OptionOne->GetReturnType()))
		{
			ternaryOperationNode.ReturnType = ternaryOperationNode.OptionTwo->GetReturnType();
		}
		else
		{
			std::string errorMessage =
				fmt::format("Return types of options in ternary operation do not match\n Option 1: {}\n Option 2: {}",
					ternaryOperationNode.OptionOne->GetReturnType().Value, ternaryOperationNode.OptionTwo->GetReturnType().Value);
			StoreParseError(ParseErrorType::Expression, errorMessage, GetCurrentToken(1));
			return { false, {} };
		}

		// Fill the expression buffer and exit
		Ref<Expression> ternaryExpression{ CreateRef<Expression>() };
		ternaryExpression->Value = ternaryOperationNode;
		parentExpressionSize = currentLocation;
		return { true, ternaryExpression };
	}
	std::tuple<bool, Ref<Expression>> ScriptTokenParser::ParseExpressionMember(uint32_t& parentExpressionSize, bool dataMemberOnly)
	{
		Ref<Expression> newMemberExpression{ CreateRef<Expression>() };
		uint32_t currentLocation = parentExpressionSize;
		Ref<MemberNode> returnMemberNode = CreateRef<MemberNode>();
		ScriptToken initialVariable{};
		std::vector<ScriptToken> memberList{};

		// Look for primitive type and first dot operator
		if (GetCurrentToken(currentLocation).Type != ScriptTokenType::Identifier ||
			GetCurrentToken(currentLocation + 1).Type != ScriptTokenType::DotOperator ||
			GetCurrentToken(currentLocation + 2).Type != ScriptTokenType::Identifier)
		{
			return { false, nullptr };
		}

		// Store variable and first member identifier
		initialVariable = GetCurrentToken(currentLocation);
		memberList.push_back(GetCurrentToken(currentLocation + 2));
		currentLocation += 3;

		// Store all subsequent identifiers
		while (GetCurrentToken(currentLocation).Type == ScriptTokenType::DotOperator &&
			GetCurrentToken(currentLocation + 1).Type == ScriptTokenType::Identifier)
		{
			memberList.push_back(GetCurrentToken(currentLocation + 1));
			currentLocation += 2;
		}

		// Get stack variable from identifier and ensure it is valid
		StackVariable currentStackVariable = GetStackVariable(initialVariable);
		if (!currentStackVariable)
		{
			StoreParseError(ParseErrorType::Expression, "Could not locate stack variable from member declaration", initialVariable);
			return { false, nullptr };
		}

		// Get primitive type and ensure it is valid
		PrimitiveType currentPrimitiveType;
		if (!ScriptCompilerService::s_ActiveLanguageDefinition.PrimitiveTypes.contains(currentStackVariable.Type.Value))
		{
			StoreParseError(ParseErrorType::Expression, "Invalid primitive type of stack variable found when parsing data member", initialVariable);
			return { false, nullptr };
		}
		currentPrimitiveType = ScriptCompilerService::s_ActiveLanguageDefinition.PrimitiveTypes.at(currentStackVariable.Type.Value);

		// Store initial variable identifier
		returnMemberNode->CurrentNodeExpression = CreateRef<Expression>(TokenExpressionNode(initialVariable, {ScriptTokenType::PrimitiveType, currentPrimitiveType.Name}));

		// Check for first context probe
		if (IsContextProbe(memberList.at(0)))
		{
			CursorContext newContext;
			newContext.m_Flags.SetFlag((uint8_t)CursorFlags::IsDataMember);
			newContext.DataMembers = currentPrimitiveType.Members;
			m_CursorContext = newContext;
			StoreParseError(ParseErrorType::ContextProbe, "Found context probe in statement expression", GetCurrentToken(currentLocation));
			return { false, nullptr };
		}

		// Get first member type from primitive type and ensure it is valid
		Ref<MemberType> currentMemberType;
		if (!currentPrimitiveType.Members.contains(memberList.at(0).Value))
		{
			std::string errorMessage =
				fmt::format("Could not locate member for provided primitive type\n Primitive Type: {}\n Member: {}",
					currentPrimitiveType.Name, memberList.at(0).Value);
			StoreParseError(ParseErrorType::Expression, errorMessage, memberList.at(0));
			return { false, nullptr };
		}
		currentMemberType = currentPrimitiveType.Members.at(memberList.at(0).Value);
		
		// Process remaining members in list
		Ref<MemberNode> finalParentNode;
		ScriptToken finalToken{};
		if (memberList.size() > 1)
		{
			DataMember* currentDataMember = std::get_if<DataMember>(&currentMemberType->Value);
			if (!currentDataMember)
			{
				std::string errorMessage =
					fmt::format("Found intermediate member that is invalid\n Member: {}", memberList.at(0).Value);
				StoreParseError(ParseErrorType::Expression, errorMessage, memberList.at(0));
				return { false, nullptr };
			}

			// Store first member of memberList
			Ref<MemberNode> childNode = CreateRef<MemberNode>();
			childNode->ChildMemberNode = nullptr;
			childNode->CurrentNodeExpression = CreateRef<Expression>(TokenExpressionNode(memberList.at(0), currentDataMember->PrimitiveType));
			childNode->m_MemberType = currentMemberType.get();
			returnMemberNode->ChildMemberNode = childNode;

			// Iterate through all remaining members of memberList
			for (std::size_t iteration {1}; iteration < memberList.size() - 1; iteration++)
			{
				// Check for context probe
				if (IsContextProbe(memberList.at(iteration)))
				{
					CursorContext newContext;
					newContext.m_Flags.SetFlag((uint8_t)CursorFlags::IsDataMember);
					newContext.DataMembers = currentDataMember->Members;
					m_CursorContext = newContext;
					StoreParseError(ParseErrorType::ContextProbe, "Found context probe in statement expression", GetCurrentToken(memberList.at(iteration)));
					return { false, nullptr };
				}

				if (!currentDataMember->Members.contains(memberList.at(iteration).Value))
				{
					StoreParseError(ParseErrorType::Expression, "Could not locate m_MemberType from identifier", memberList.at(iteration));
					return { false, nullptr };
				}
				currentMemberType = currentDataMember->Members.at(memberList.at(iteration).Value);
				currentDataMember = std::get_if<DataMember>(&currentMemberType->Value);
				if (!currentDataMember)
				{
					std::string errorMessage =
						fmt::format("Found intermediate member that is invalid\n Member: {}", memberList.at(0).Value);
					StoreParseError(ParseErrorType::Expression, errorMessage, memberList.at(0));
					return { false, nullptr };
				}
				Ref<MemberNode> newNode = CreateRef<MemberNode>();
				newNode->ChildMemberNode = nullptr;
				newNode->CurrentNodeExpression = CreateRef<Expression>(TokenExpressionNode(memberList.at(iteration), currentDataMember->PrimitiveType));
				childNode->ChildMemberNode = newNode;
				childNode->m_MemberType = currentMemberType.get();
				childNode = newNode;
			}
			// Check for final context probe
			if (IsContextProbe(memberList.at(memberList.size() - 1)))
			{
				CursorContext newContext;
				newContext.m_Flags.SetFlag((uint8_t)CursorFlags::IsDataMember);
				newContext.DataMembers = currentDataMember->Members;
				m_CursorContext = newContext;
				StoreParseError(ParseErrorType::ContextProbe, "Found context probe in statement expression", GetCurrentToken(memberList.at(memberList.size() - 1)));
				return { false, nullptr };
			}
			if (!currentDataMember->Members.contains(memberList.at(memberList.size() - 1).Value))
			{
				StoreParseError(ParseErrorType::Expression, "Could not locate m_MemberType from identifier", memberList.at(memberList.size() - 1));
				return { false, nullptr };
			}
			currentMemberType = currentDataMember->Members.at(memberList.at(memberList.size() - 1).Value);
			finalParentNode = childNode;
			finalToken = memberList.at(memberList.size() - 1);
		}
		else
		{
			finalParentNode = returnMemberNode;
			finalToken = memberList.at(0);
		}
		
		// Process final member node
		if (DataMember* dataMemberPtr = std::get_if<DataMember>(&currentMemberType->Value))
		{
			// Fill member node's data
			DataMember currentDataMember = *dataMemberPtr;
			returnMemberNode->ReturnType = currentDataMember.PrimitiveType;
			Ref<MemberNode> childNode = CreateRef<MemberNode>();
			childNode->ChildMemberNode = nullptr;
			childNode->CurrentNodeExpression = CreateRef<Expression>(TokenExpressionNode(finalToken, currentDataMember.PrimitiveType));
			childNode->m_MemberType = currentMemberType.get();
			finalParentNode->ChildMemberNode = childNode;
		}
		else if (FunctionNode* functionMemberPtr = std::get_if<FunctionNode>(&currentMemberType->Value))
		{
			if (dataMemberOnly)
			{
				return { false, nullptr };
			}
			// Fill function call node
			FunctionNode currentFunctionMember = *functionMemberPtr;
			FunctionCallNode newFunctionCall;
			// Fill function call node
			returnMemberNode->ReturnType = currentFunctionMember.ReturnType;
			newFunctionCall.ReturnType = currentFunctionMember.ReturnType;
			newFunctionCall.Identifier = finalToken;
			// Check for opening parentheses
			if (GetCurrentToken(currentLocation).Type != ScriptTokenType::OpenParentheses)
			{
				return { false, {} };
			}
			currentLocation++;
			
			// Check for arguments and commas
			bool continueLoop;
			do
			{
				continueLoop = false;
				// Parse Expression Node, store expression, and check for context probe
				auto [success, expression] = ParseExpressionNode(currentLocation);
				if (success)
				{
					newFunctionCall.Arguments.push_back(expression);
				}

				if (CheckForErrors())
				{
					StoreParseError(ParseErrorType::Expression, "Invalid function argument", GetCurrentToken(currentLocation));
					return { false, {} };
				}

				// Decide whether to continue looking for more arguments
				if (success && GetCurrentToken(currentLocation).Type == ScriptTokenType::Comma)
				{
					currentLocation++;
					continueLoop = true;
				}

			} while (continueLoop);

			// Check for closing parentheses
			if (GetCurrentToken(currentLocation).Type != ScriptTokenType::CloseParentheses)
			{
				return { false, {} };
			}
			currentLocation++;

			// Ensure number of arguments in function call match the number of arguments in the function
			if (currentFunctionMember.Parameters.size() != newFunctionCall.Arguments.size())
			{
				std::string errorMessage =
					fmt::format("Argument count in function call ({}) does not match the function parameter count ({})",
						newFunctionCall.Arguments.size(), currentFunctionMember.Parameters.size());
				StoreParseError(ParseErrorType::Expression, errorMessage, newFunctionCall.Identifier);
				return { false, {} };
			}

			// Ensure each argument type matches the parameter type
			uint32_t parameterIteration{ 0 };
			for (auto& parameter : currentFunctionMember.Parameters)
			{
				bool valid = false;

				for (auto& type : parameter.AllTypes)
				{
					if (PrimitiveTypeAcceptableToken(type.Value, newFunctionCall.Arguments.at(parameterIteration)->GetReturnType()))
					{
						valid = true;
					}
				}
				if (!valid)
				{
					std::string errorMessage =
						fmt::format("Argument type is not acceptable for member function parameter\n Argument Type: {}\n Parameter Type(s):",
							GetPrimitiveTypeFromToken((newFunctionCall.Arguments.at(parameterIteration)->GetReturnType())).Value);
					for (auto& type : parameter.AllTypes)
					{
						errorMessage = errorMessage + "\n " + type.Value;
					}
					StoreParseError(ParseErrorType::Expression, errorMessage, newFunctionCall.Arguments.at(parameterIteration)->GetReturnType());
					return { false, {} };
				}
				parameterIteration++;
			}

			newFunctionCall.m_FunctionNode = functionMemberPtr;
			Ref<MemberNode> childNode = CreateRef<MemberNode>();
			childNode->ChildMemberNode = nullptr;
			childNode->CurrentNodeExpression = CreateRef<Expression>();
			childNode->CurrentNodeExpression->Value = newFunctionCall;
			finalParentNode->ChildMemberNode = childNode;
		}
		else
		{
			return { false, nullptr };
		}

		// Fill the expression buffer and exit
		newMemberExpression->Value = *returnMemberNode;
		parentExpressionSize = currentLocation;
		return { true, newMemberExpression };
	}
	std::tuple<bool, Ref<Statement>> ScriptTokenParser::ParseStatementEmpty()
	{
		ScriptToken tokenBuffer = GetCurrentToken();

		if (tokenBuffer.Type != ScriptTokenType::Semicolon)
		{
			return { false, nullptr };
		}

		Ref<Statement> newStatement = CreateRef<Statement>();
		newStatement->Value = StatementEmpty();
		Advance();
		return { true, newStatement };
	}
	std::tuple<bool, Ref<Statement>> ScriptTokenParser::ParseStatementExpression()
	{
		Ref<Expression> newExpression{ CreateRef<Expression>() };
		ScriptToken tokenBuffer = GetCurrentToken();

		// Check for an expression
		uint32_t expressionSize{ 0 };
		{
			auto [success, expression] = ParseExpressionNode(expressionSize);

			if (!success)
			{
				return { false, nullptr };
			}
			newExpression = expression;
		}

		// Check for context probe
		if (IsContextProbe(newExpression))
		{
			CursorContext newContext;
			newContext.m_Flags.SetFlag((uint8_t)CursorFlags::AllowAllVariableTypes);
			newContext.StackVariables = m_StackVariables;
			m_CursorContext = newContext;
			StoreParseError(ParseErrorType::ContextProbe, "Found context probe in statement expression", tokenBuffer);
			return { false, nullptr };
		}

		// Check for a terminating semicolon
		if (GetCurrentToken(expressionSize).Type != ScriptTokenType::Semicolon)
		{
			return { false, nullptr };
		}

		if (CheckForErrors())
		{
			StoreParseError(ParseErrorType::Statement, "Invalid expression provided in expression statement", tokenBuffer);
			return { false, nullptr };
		}

		Ref<Statement> newStatement = CreateRef<Statement>();
		StatementExpression newStatementExpression{ newExpression };
		newStatement->Value = newStatementExpression;
		Advance(1 + expressionSize);
		return { true, newStatement };
	}
	std::tuple<bool, Ref<Statement>> ScriptTokenParser::ParseStatementDeclaration()
	{
		ScriptToken tokenBuffer = GetCurrentToken();

		if (tokenBuffer.Type != ScriptTokenType::PrimitiveType ||
			GetCurrentToken(1).Type != ScriptTokenType::Identifier ||
			GetCurrentToken(2).Type != ScriptTokenType::Semicolon)
		{
			return { false, nullptr };
		}

		if (CheckCurrentStackFrameForIdentifier(GetCurrentToken(1)))
		{
			StoreParseError(ParseErrorType::Statement, "Duplicate identifier found during declaration", GetCurrentToken(1));
			return { false, nullptr };
		}
		Ref<Statement> newStatement = CreateRef<Statement>();
		StatementDeclaration newStatementDeclaration{ tokenBuffer, GetCurrentToken(1) };
		newStatement->Value = newStatementDeclaration;
		StoreStackVariable(tokenBuffer, GetCurrentToken(1));
		Advance(3);
		return { true, newStatement };
	}
	std::tuple<bool, Ref<Statement>> ScriptTokenParser::ParseStatementAssignment()
	{
		uint32_t currentLocation{ 0 };
		StatementAssignment newStatementAssignment;

		// Check for name that is either a member expression or an individual identifier (name = value;)
		{
			
			uint32_t expressionSize{ currentLocation };
			auto [success, expression] = ParseExpressionMember(expressionSize, true);
			if (success)
			{
				// Store member expression
				newStatementAssignment.Name = expression;
				currentLocation = expressionSize;
			}
			else if (GetCurrentToken(currentLocation).Type == ScriptTokenType::Identifier)
			{
				// Store individual identifier
				newStatementAssignment.Name = TokenToExpression(GetCurrentToken(currentLocation));
				currentLocation++;
			}
			else
			{
				// Exit if other
				return { false, nullptr };
			}
			if (CheckForErrors())
			{
				StoreParseError(ParseErrorType::Expression, "Unable to parse assignment expression name", GetCurrentToken(currentLocation));
				return { false, {} };
			}
		}

		// Check for assignment operator
		if (GetCurrentToken(currentLocation).Type != ScriptTokenType::AssignmentOperator)
		{
			return { false, nullptr };
		}
		currentLocation++;

		// Check for assignment statement value (name = value;)
		{
			uint32_t expressionSize{ currentLocation };
			auto [success, expression] = ParseExpressionNode(expressionSize);
			if (!success)
			{
				return { false, {} };
			}
			if (CheckForErrors())
			{
				StoreParseError(ParseErrorType::Expression, "Unable to parse value expression in assignment statement", GetCurrentToken(currentLocation));
				return { false, {} };
			}
			// Check for context probe
			if (IsContextProbe(expression))
			{
				ScriptToken returnType = GetPrimitiveTypeFromToken(newStatementAssignment.Name->GetReturnType());
				CursorContext newContext;
				newContext.AllReturnTypes.push_back(returnType);
				if (returnType.Value == "None")
				{
					newContext.m_Flags.SetFlag((uint8_t)CursorFlags::AllowAllVariableTypes);
				}
				newContext.StackVariables = m_StackVariables;
				m_CursorContext = newContext;
				StoreParseError(ParseErrorType::ContextProbe, "Found context probe in statement assignment", GetCurrentToken(currentLocation));
				return { false, {} };
			}
			newStatementAssignment.Value = expression;
			currentLocation = expressionSize;
		}

		// Check for semicolon
		if (GetCurrentToken(currentLocation).Type != ScriptTokenType::Semicolon)
		{
			return { false, {} };
		}
		currentLocation++;

		// Get the return type of the Name and Value
		ScriptToken statementNameType;
		if (MemberNode* currentStatementName = std::get_if<MemberNode>(&newStatementAssignment.Name->Value))
		{
			// Add logic for storing member node
			statementNameType = currentStatementName->ReturnType;
		}
		else if (TokenExpressionNode* currentStatementName = std::get_if<TokenExpressionNode>(&newStatementAssignment.Name->Value))
		{
			// Ensure identifer is a valid StackVariable
			if (!CheckStackForIdentifier(currentStatementName->Value))
			{
				StoreParseError(ParseErrorType::Statement, "Unknown variable found in assignment statement", currentStatementName->Value);
				return { false, {} };
			}
			StackVariable statementNameVariable = GetStackVariable(currentStatementName->Value);
			statementNameType = statementNameVariable.Type;
		}
		else
		{
			StoreParseError(ParseErrorType::Statement, "Unknown assignment statement name expression", GetCurrentToken());
			return { false, {} };
		}

		// Ensure statement value is a valid to be assigned to the statement name
		bool success = PrimitiveTypeAcceptableToken(statementNameType.Value, newStatementAssignment.Value->GetReturnType());
		if (!success)
		{
			std::string errorMessage =
				fmt::format("Invalid expression return type provided in assignment statement\n Declared type: {}\n ExpressionType: {}",
					statementNameType.Value, GetPrimitiveTypeFromToken(newStatementAssignment.Value->GetReturnType()).Value);
			StoreParseError(ParseErrorType::Statement, errorMessage, GetCurrentToken(1));
			return { false, {} };
		}

		Ref<Statement> newStatement = CreateRef<Statement>();
		newStatement->Value = newStatementAssignment;
		Advance(currentLocation);
		return { true, newStatement };
	}
	std::tuple<bool, Ref<Statement>> ScriptTokenParser::ParseStatementDeclarationAssignment()
	{
		Ref<Expression> newExpression{ CreateRef<Expression>() };
		ScriptToken tokenBuffer = GetCurrentToken();

		// Check for Type, Identifier, and Assignment Operator
		if (tokenBuffer.Type != ScriptTokenType::PrimitiveType ||
			GetCurrentToken(1).Type != ScriptTokenType::Identifier ||
			GetCurrentToken(2).Type != ScriptTokenType::AssignmentOperator)
		{
			return { false, {} };
		}

		// Check for an expression
		uint32_t expressionSize{ 0 };
		{
			Advance(3);
			auto [success, expression] = ParseExpressionNode(expressionSize);
			Advance(-3);
			if (!success)
			{
				return { false, {} };
			}
			newExpression = expression;
		}

		// Check for context probe
		if (IsContextProbe(newExpression))
		{
			CursorContext newContext;
			newContext.AllReturnTypes.push_back(tokenBuffer);
			if (tokenBuffer.Value == "None")
			{
				newContext.m_Flags.SetFlag((uint8_t)CursorFlags::AllowAllVariableTypes);
			}
			newContext.StackVariables = m_StackVariables;
			m_CursorContext = newContext;
			StoreParseError(ParseErrorType::ContextProbe, "Found context probe in statement declaration/assignment", tokenBuffer);
			return { false, {} };
		}

		// Check for semicolon
		if (GetCurrentToken(3 + expressionSize).Type != ScriptTokenType::Semicolon)
		{
			return { false, {} };
		}

		if (CheckForErrors())
		{
			StoreParseError(ParseErrorType::Statement, "Invalid expression provided in declaration/assignment statement", tokenBuffer);
			return { false, {} };
		}

		// Ensure expression value is a valid type to be assigned to new identifier
		bool success = PrimitiveTypeAcceptableToken(GetCurrentToken().Value, newExpression->GetReturnType());
		if (!success)
		{
			std::string errorMessage =
				fmt::format("Invalid expression return type provided in assignment statement\n Declared type: {}\n ExpressionType: {}",
					tokenBuffer.Value, GetPrimitiveTypeFromToken(newExpression->GetReturnType()).Value);
			StoreParseError(ParseErrorType::Statement, errorMessage, GetCurrentToken(2));
			return { false, nullptr };
		}

		// Ensure identifer is not being declared twice in the current stack frame
		if (CheckCurrentStackFrameForIdentifier(GetCurrentToken(1)))
		{
			StoreParseError(ParseErrorType::Statement, "Duplicate identifier found during declaration", GetCurrentToken(1));
			return { false, nullptr };
		}

		Ref<Statement> newStatement = CreateRef<Statement>();
		StatementDeclarationAssignment newStatementAssignment{ tokenBuffer, GetCurrentToken(1), newExpression };
		newStatement->Value = newStatementAssignment;
		StoreStackVariable(tokenBuffer, GetCurrentToken(1));
		Advance(4 + expressionSize);
		return { true, newStatement };
	}
	std::tuple<bool, Ref<Statement>> ScriptTokenParser::ParseStatementConditional(bool isCurrentConditionChained)
	{
		StatementConditional newStatementConditional{};
		ScriptToken tokenBuffer = GetCurrentToken();
		uint32_t initialAdvance{ 0 };
		uint32_t initialKeywordLocation = m_TokenLocation;
		
		// Only allow else/else-if if predicate says this new conditional is part of a chain
		if (isCurrentConditionChained)
		{
			// Check for else/else-if
			if (tokenBuffer.Type != ScriptTokenType::Keyword || tokenBuffer.Value != "else")
			{
				// Exit peacefully
				return { false, nullptr };
			}
			// Store conditional type
			if (tokenBuffer.Value == "else" && GetCurrentToken(1).Value == "if")
			{
				newStatementConditional.Type = ConditionalType::ELSEIF;
				initialAdvance = 2;
			}
			else
			{
				newStatementConditional.Type = ConditionalType::ELSE;
				initialAdvance = 1;
			}
		}
		else
		{
			// Check only for if type
			if (tokenBuffer.Type != ScriptTokenType::Keyword || tokenBuffer.Value != "if")
			{
				// Attempt to start if/else/else-if chain with else/else-if
				if (tokenBuffer.Type == ScriptTokenType::Keyword && tokenBuffer.Value == "else")
				{
					StoreParseError(ParseErrorType::Statement, "Cannot start an if/else/else-if chain with an else/else-if", tokenBuffer);
					return { false, nullptr };
				}

				// Exit peacefully
				return { false, {} };
			}

			newStatementConditional.Type = ConditionalType::IF;
			initialAdvance = 1;
		}
		Advance(initialAdvance);

		if (newStatementConditional.Type != ConditionalType::ELSE)
		{
			// Check for opening parentheses
			tokenBuffer = GetCurrentToken();
			if (tokenBuffer.Type != ScriptTokenType::OpenParentheses)
			{
				StoreParseError(ParseErrorType::Statement, "Expecting an open parentheses for if/else-if statement", GetToken(initialKeywordLocation));
				return { false, nullptr };
			}
			Advance();

			// Parse Condition Expression
			tokenBuffer = GetCurrentToken();
			uint32_t conditionExpressionSize{ 0 };
			{
				auto [success, expression] = ParseExpressionNode(conditionExpressionSize);

				// Error checking for general errors/invalid return type for condition expression
				if (!success || CheckForErrors())
				{
					StoreParseError(ParseErrorType::Statement, "Could not parse condition for if/else-if statement", GetToken(initialKeywordLocation));
					return { false, nullptr };
				}

				if (IsContextProbe(expression))
				{
					CursorContext newContext;
					ScriptToken newReturnType;
					newReturnType.Type = ScriptTokenType::PrimitiveType;
					newReturnType.Value = "bool";
					newContext.AllReturnTypes.push_back(newReturnType);
					newContext.StackVariables = m_StackVariables;
					m_CursorContext = newContext;
					StoreParseError(ParseErrorType::ContextProbe, "Found context probe in if/else-if condition", tokenBuffer);
					return { false, nullptr };
				}

				if (GetPrimitiveTypeFromToken(expression->GetReturnType()).Value != "bool")
				{
					StoreParseError(ParseErrorType::Statement, "Condition does not return a true/false value in if/else-if statement", GetToken(initialKeywordLocation));
					return { false, nullptr };
				}
				// Store expression if successful
				newStatementConditional.ConditionExpression = expression;
			}
			Advance(conditionExpressionSize);

			// Check for closing parentheses
			tokenBuffer = GetCurrentToken();
			if (tokenBuffer.Type != ScriptTokenType::CloseParentheses)
			{
				StoreParseError(ParseErrorType::Statement, "Expecting an closing parentheses for if/else-if statement", GetToken(initialKeywordLocation));
				return { false, nullptr };
			}
			Advance();
		}

		// Check for opening curly brace
		tokenBuffer = GetCurrentToken();
		if (tokenBuffer.Type != ScriptTokenType::OpenCurlyBrace)
		{
			StoreParseError(ParseErrorType::Statement, "Expecting an opening curly for if/else/else-if statement", GetToken(initialKeywordLocation));
			return { false, nullptr };
		}
		Advance();

		// Parse conditional statement's body statements
		{
			AddStackFrame();
			bool success = false;
			tokenBuffer = GetCurrentToken();
			Ref<Statement> statement {nullptr};
			do
			{
				std::tie(success, statement) = ParseStatementNode();
				if (success)
				{
					newStatementConditional.BodyStatements.push_back(statement);
				}
			} while (success);

			if (CheckForErrors())
			{
				StoreParseError(ParseErrorType::Statement, "Could not parse body statements for if/else/else-if statement", GetToken(initialKeywordLocation));
				return { false, nullptr };
			}

			// Check for close curly braces
			tokenBuffer = GetCurrentToken();
			if (tokenBuffer.Type != ScriptTokenType::CloseCurlyBrace)
			{
				if (tokenBuffer.Type == ScriptTokenType::None && tokenBuffer.Value == "End of File")
				{
					StoreParseError(ParseErrorType::Statement, "Expecting an closing curly brace for if/else/else-if statement, however, the file ended abruptly\n Did you forget a closing curly brace somewhere?", GetToken(initialKeywordLocation));
				}
				else
				{
					StoreParseError(ParseErrorType::Statement, "Expecting an closing curly brace for if/else/else-if statement\n Did you forget a semicolon?", tokenBuffer);
				}
				return { false, nullptr };
			}

			// Remove current stack variables
			PopStackFrame();
			Advance();
		}

		// Parse chained conditional statements
		if (newStatementConditional.Type != ConditionalType::ELSE)
		{
			bool success = false;
			tokenBuffer = GetCurrentToken();
			Ref<Statement> statement {nullptr};
			do
			{
				std::tie(success, statement) = ParseStatementConditional(true);
				if (success)
				{
					newStatementConditional.ChainedConditionals.push_back(statement);
				}

				if (StatementConditional* conditional = std::get_if<StatementConditional>(&statement->Value))
				{
					if (conditional->Type == ConditionalType::ELSE)
					{
						break;
					}
				}
			} while (success);

			if (CheckForErrors())
			{
				StoreParseError(ParseErrorType::Statement, "Could not parse chained if/else/else-if statements", tokenBuffer);
				return { false, nullptr };
			}
		}

		Ref<Statement> newStatement = CreateRef<Statement>();
		newStatement->Value = newStatementConditional;

		return { true, newStatement };
	}
	std::tuple<bool, Ref<Statement>> ScriptTokenParser::ParseStatementWhileLoop()
	{
		StatementWhileLoop newStatementWhileLoop{};
		ScriptToken tokenBuffer = GetCurrentToken();
		uint32_t initialKeywordLocation = m_TokenLocation;

		if (tokenBuffer.Type != ScriptTokenType::Keyword || tokenBuffer.Value != "while")
		{
			// Exit peacefully, statement is not a while loop
			return { false, nullptr };
		}
		Advance();

		// Check for opening parentheses
		tokenBuffer = GetCurrentToken();
		if (tokenBuffer.Type != ScriptTokenType::OpenParentheses)
		{
			StoreParseError(ParseErrorType::Statement, "Expecting an open parentheses for if/else-if statement", GetToken(initialKeywordLocation));
			return { false, nullptr };
		}
		Advance();

		// Parse Condition Expression
		tokenBuffer = GetCurrentToken();
		uint32_t conditionExpressionSize{ 0 };
		{
			// Attempt to parse expression
			auto [success, expression] = ParseExpressionNode(conditionExpressionSize);

			// Error checking for general errors/invalid return type for condition expression
			if (!success || CheckForErrors())
			{
				StoreParseError(ParseErrorType::Statement, "Could not parse condition for while statement", GetToken(initialKeywordLocation));
				return { false, nullptr };
			}

			// Check for context probe (for in-editor suggestions)
			if (IsContextProbe(expression))
			{
				CursorContext newContext;
				ScriptToken newReturnType;
				newReturnType.Type = ScriptTokenType::PrimitiveType;
				newReturnType.Value = "bool";
				newContext.AllReturnTypes.push_back(newReturnType);
				newContext.StackVariables = m_StackVariables;
				m_CursorContext = newContext;
				StoreParseError(ParseErrorType::ContextProbe, "Found context probe in while condition", tokenBuffer);
				return { false, nullptr };
			}

			// Ensure expression return type is a boolean
			if (GetPrimitiveTypeFromToken(expression->GetReturnType()).Value != "bool")
			{
				StoreParseError(ParseErrorType::Statement, "Condition does not return a true/false value in while statement", GetToken(initialKeywordLocation));
				return { false, nullptr };
			}
			// Store expression if successful
			newStatementWhileLoop.ConditionExpression = expression;
		}
		Advance(conditionExpressionSize);

		// Check for closing parentheses
		tokenBuffer = GetCurrentToken();
		if (tokenBuffer.Type != ScriptTokenType::CloseParentheses)
		{
			StoreParseError(ParseErrorType::Statement, "Expecting an closing parentheses for if/else-if statement", GetToken(initialKeywordLocation));
			return { false, nullptr };
		}
		Advance();
		

		// Check for opening curly brace
		tokenBuffer = GetCurrentToken();
		if (tokenBuffer.Type != ScriptTokenType::OpenCurlyBrace)
		{
			StoreParseError(ParseErrorType::Statement, "Expecting an opening curly for if/else/else-if statement", GetToken(initialKeywordLocation));
			return { false, nullptr };
		}
		Advance();

		// Parse conditional statement's body statements
		{
			AddStackFrame();
			IncrementLoopDepth();
			bool success = false;
			tokenBuffer = GetCurrentToken();

			// Loop through statements inside while loop body
			Ref<Statement> statement {nullptr};
			do
			{
				std::tie(success, statement) = ParseStatementNode();
				if (success)
				{
					newStatementWhileLoop.BodyStatements.push_back(statement);
				}
			} while (success);

			if (CheckForErrors())
			{
				StoreParseError(ParseErrorType::Statement, "Could not parse body statements for while statement", GetToken(initialKeywordLocation));
				return { false, nullptr };
			}

			// Check for close curly braces
			tokenBuffer = GetCurrentToken();
			if (tokenBuffer.Type != ScriptTokenType::CloseCurlyBrace)
			{
				if (tokenBuffer.Type == ScriptTokenType::None && tokenBuffer.Value == "End of File")
				{
					StoreParseError(ParseErrorType::Statement, "Expecting an closing curly brace for while statement, however, the file ended abruptly\n Did you forget a closing curly brace somewhere?", GetToken(initialKeywordLocation));
				}
				else
				{
					StoreParseError(ParseErrorType::Statement, "Expecting an closing curly brace for while statement\n Did you forget a semicolon?", tokenBuffer);
				}
				return { false, nullptr };
			}

			// Remove current stack variables
			PopStackFrame();
			DecrimentLoopDepth();
			Advance();
		}

		Ref<Statement> newStatement = CreateRef<Statement>();
		newStatement->Value = newStatementWhileLoop;

		return { true, newStatement };
	}
	std::tuple<bool, Ref<Statement>> ScriptTokenParser::ParseStatementBreak()
	{
		StatementBreak newStatementBreak{};
		uint32_t currentLocation{ 0 };

		// Check for a break keyword
		if (GetCurrentToken(currentLocation).Type != ScriptTokenType::Keyword || GetCurrentToken(currentLocation).Value != "break")
		{
			return { false, nullptr };
		}
		currentLocation++;

		// Ensure that current loop depth is greater than 0 (i.e. we are inside of a loop body)
		if (m_LoopDepth == 0)
		{
			StoreParseError(ParseErrorType::Statement, "Attempt to add break statement when not inside of a loop", GetCurrentToken());
			return { false, nullptr };
		}

		// Check for a terminating semicolon
		if (GetCurrentToken(currentLocation).Type != ScriptTokenType::Semicolon)
		{
			StoreParseError(ParseErrorType::Statement, "Expecting a semicolon to complete break statement", GetCurrentToken());
			return { false, nullptr };
		}
		currentLocation++;

		Ref<Statement> newStatement = CreateRef<Statement>();
		newStatement->Value = newStatementBreak;
		Advance(currentLocation);
		return { true, newStatement };
	}
	std::tuple<bool, Ref<Statement>> ScriptTokenParser::ParseStatementContinue()
	{
		StatementContinue newStatementContinue{};
		uint32_t currentLocation{ 0 };

		// Check for a continue keyword
		if (GetCurrentToken(currentLocation).Type != ScriptTokenType::Keyword || GetCurrentToken(currentLocation).Value != "continue")
		{
			return { false, nullptr };
		}
		currentLocation++;

		// Ensure that current loop depth is greater than 0 (i.e. we are inside of a loop body)
		if (m_LoopDepth == 0)
		{
			StoreParseError(ParseErrorType::Statement, "Attempt to add continue statement when not inside of a loop", GetCurrentToken());
			return { false, nullptr };
		}

		// Check for a terminating semicolon
		if (GetCurrentToken(currentLocation).Type != ScriptTokenType::Semicolon)
		{
			StoreParseError(ParseErrorType::Statement, "Expecting a semicolon to complete continue statement", GetCurrentToken());
			return { false, nullptr };
		}
		currentLocation++;

		Ref<Statement> newStatement = CreateRef<Statement>();
		newStatement->Value = newStatementContinue;
		Advance(currentLocation);
		return { true, newStatement };
	}
	std::tuple<bool, Ref<Statement>> ScriptTokenParser::ParseStatementReturn()
	{
		StatementReturn newStatementReturn{};
		uint32_t currentLocation{ 0 };

		// Check for a return keyword
		if (GetCurrentToken(currentLocation).Type != ScriptTokenType::Keyword || GetCurrentToken(currentLocation).Value != "return")
		{
			return { false, nullptr };
		}
		currentLocation++;

		if (m_CurrentReturnType.Type == ScriptTokenType::None)
		{
			StoreParseError(ParseErrorType::Statement, "Attempt to add return statement for a function with a void return type", GetCurrentToken());
			return { false, nullptr };
		}

		// Check for an expression
		{
			auto [success, expression] = ParseExpressionNode(currentLocation);

			if (success)
			{
				newStatementReturn.ReturnValue = expression;
			}
			else
			{
				newStatementReturn.ReturnValue = nullptr;
			}
		}

		if (CheckForErrors())
		{
			StoreParseError(ParseErrorType::Statement, "Invalid expression provided in return statement", GetCurrentToken());
			return { false, nullptr };
		}

		// Check for context probe
		if (IsContextProbe(newStatementReturn.ReturnValue))
		{
			CursorContext newContext;
			ScriptToken newReturnType;
			if (m_CurrentReturnType.Type == ScriptTokenType::PrimitiveType && m_CurrentReturnType.Value != "None")
			{
				newReturnType = m_CurrentReturnType;
				newContext.AllReturnTypes.push_back(newReturnType);
			}
			else
			{
				newContext.m_Flags.SetFlag((uint8_t)CursorFlags::AllowAllVariableTypes);
			}
			newContext.StackVariables = m_StackVariables;
			m_CursorContext = newContext;
			StoreParseError(ParseErrorType::ContextProbe, "Found context probe in statement return", GetCurrentToken());
			return { false, nullptr };
		}

		// Check for a terminating semicolon
		if (GetCurrentToken(currentLocation).Type != ScriptTokenType::Semicolon)
		{
			return { false, nullptr };
		}
		currentLocation++;

		// Ensure return type of the function is equivalent to the returnExpression type
		if (newStatementReturn.ReturnValue && m_CurrentReturnType.Value != GetPrimitiveTypeFromToken(newStatementReturn.ReturnValue->GetReturnType()).Value)
		{
			StoreParseError(ParseErrorType::Statement, "Return statement expression type does not match function return type", GetCurrentToken());
			return { false, nullptr };
		}

		Ref<Statement> newStatement = CreateRef<Statement>();
		newStatement->Value = newStatementReturn;
		Advance(currentLocation);
		return { true, newStatement };
	}
	ScriptToken ScriptTokenParser::GetToken(int32_t location)
	{
		if (location >= (int32_t)m_Tokens.size())
		{
			return { ScriptTokenType::None, "End of File", InvalidLine, InvalidColumn };
		}
		
		return m_Tokens.at(location);
	}
	ScriptToken ScriptTokenParser::GetCurrentToken(int32_t offset)
	{
		// Return empty token if end of file reached or attempt to access token below 0
		if ((int32_t)m_TokenLocation + offset >= (int32_t)m_Tokens.size())
		{
			return { ScriptTokenType::None, "End of File", InvalidLine, InvalidColumn };
		}

		if ((int32_t)m_TokenLocation + offset < 0)
		{
			return { ScriptTokenType::None, "Index below 0", InvalidLine, InvalidColumn };
		}
		return m_Tokens.at(m_TokenLocation + offset);
	}
	Ref<Expression> ScriptTokenParser::TokenToExpression(ScriptToken token)
	{
		// Check for a single identifier
		TokenExpressionNode tokenNode;
		tokenNode.Value = token;
		if ((token.Type == ScriptTokenType::Identifier))
		{
			// Ensure identifier is a proper value
			if (!IsContextProbe(tokenNode.Value) && tokenNode.Value.Type == ScriptTokenType::Identifier && !CheckStackForIdentifier(tokenNode.Value))
			{
				StoreParseError(ParseErrorType::Expression, "Unknown variable", tokenNode.Value);
				return nullptr;
			}

			// Get return type from variable
			StackVariable currentVariable = GetStackVariable(tokenNode.Value);
			tokenNode.ReturnType = currentVariable.Type;
		}
		else if (ScriptCompilerService::IsLiteral(token))
		{
			// Get return type for literal
			tokenNode.ReturnType = GetPrimitiveTypeFromToken(tokenNode.Value);
		}
		else
		{
			StoreParseError(ParseErrorType::Expression, "Could not convert token to an expression", tokenNode.Value);
			return nullptr;
		}

		// Store new expression and return
		Ref<Expression> newExpression { CreateRef<Expression>()};
		newExpression->Value = tokenNode;
		return newExpression;
		
	}
	void ScriptTokenParser::Advance(uint32_t count)
	{
		m_TokenLocation += count;
	}

	void ScriptTokenParser::StoreStackVariable(ScriptToken type, ScriptToken identifier)
	{
		KG_ASSERT(m_StackVariables.size() > 0);

		std::vector<StackVariable>& currentStackFrame = m_StackVariables.back();
		StackVariable newStack{ type, identifier };
		currentStackFrame.push_back(newStack);
	}

	void ScriptTokenParser::AddStackFrame()
	{
		m_StackVariables.push_back({});
	}

	void ScriptTokenParser::PopStackFrame()
	{
		m_StackVariables.pop_back();
	}

	void ScriptTokenParser::IncrementLoopDepth()
	{
		m_LoopDepth++;
	}

	void ScriptTokenParser::DecrimentLoopDepth()
	{
		KG_ASSERT(m_LoopDepth > 0);
		m_LoopDepth--;
	}

	bool ScriptTokenParser::CheckStackForIdentifier(ScriptToken identifier)
	{
		// Search each stack frame in reverse to check most recent identifiers first
		for (auto stackIterator = m_StackVariables.rbegin(); stackIterator != m_StackVariables.rend(); ++stackIterator)
		{
			for (auto& stackVariable : *stackIterator)
			{
				if (stackVariable.Identifier.Value == identifier.Value)
				{
					return true;
				}
			}
		}

		// Return false if identifier could not be found
		return false;
	}

	bool ScriptTokenParser::CheckCurrentStackFrameForIdentifier(ScriptToken identifier)
	{
		// Get the current stack frame
		std::vector<StackVariable>& currentFrame = m_StackVariables.back();

		// Search through stack frame for identifier
		for (auto& stackVariable : currentFrame)
		{
			if (stackVariable.Identifier.Value == identifier.Value)
			{
				return true;
			}
		}
		// Return false if identifier could not be found
		return false;
	}

	StackVariable ScriptTokenParser::GetStackVariable(ScriptToken identifier)
	{
		// Search each stack frame in reverse to check most recent identifiers first
		for (auto stackIterator = m_StackVariables.rbegin(); stackIterator != m_StackVariables.rend(); ++stackIterator)
		{
			for (auto& stackVariable : *stackIterator)
			{
				if (stackVariable.Identifier.Value == identifier.Value)
				{
					return stackVariable;
				}
			}
		}
		// Return false if identifier could not be found
		return {};
	}

	void ScriptTokenParser::StoreParseError(ParseErrorType errorType, const std::string& message, ScriptToken errorToken)
	{
		m_Errors.push_back({ errorType , message, errorToken });
	}

	bool ScriptTokenParser::CheckForErrors()
	{
		return m_Errors.size() > 0;
	}

	bool ScriptTokenParser::IsContextProbe(ScriptToken token)
	{
		if ((token.Type == ScriptTokenType::Identifier ||
			token.Type == ScriptTokenType::CustomLiteral) &&
			token.Value == ContextProbe)
		{
			return true;
		}

		return false;
	}

	bool ScriptTokenParser::IsContextProbe(Ref<Expression> expression)
	{
		if (const TokenExpressionNode* token = std::get_if<TokenExpressionNode>(&expression->Value))
		{
			return IsContextProbe(token->Value);
		}
		return false;
	}

	bool ScriptTokenParser::PrimitiveTypeAcceptableToken(const std::string& queryType, Scripting::ScriptToken queryToken)
	{
		// Get primitive type associated with the parameter string type
		if (ScriptCompilerService::s_ActiveLanguageDefinition.PrimitiveTypes.contains(queryType))
		{
			PrimitiveType queryPrimitiveType = ScriptCompilerService::s_ActiveLanguageDefinition.PrimitiveTypes.at(queryType);
			// Check whether the provided token is literal (string literal, integer literal, etc...) that matches the primitiveType's requirements
			if (queryToken.Type == queryPrimitiveType.AcceptableLiteral)
			{
				return true;
			}

			// Check if token is an identifier
			if (queryToken.Type == ScriptTokenType::Identifier)
			{
				// Get associated stack variable
				StackVariable queryTokenVariable = GetStackVariable(queryToken);
				if (!queryTokenVariable)
				{
					return false;
				}
				// Ensure return type is PrimitiveType (it should always be, but you never know ^0^)
				if (queryTokenVariable.Type.Type != ScriptTokenType::PrimitiveType)
				{
					return false;
				}
				// Ensure token primitive type is valid
				if (!ScriptCompilerService::s_ActiveLanguageDefinition.PrimitiveTypes.contains(queryTokenVariable.Type.Value))
				{
					return false;
				}

				// Make sure that the literal types match
				PrimitiveType variablePrimitiveType = ScriptCompilerService::s_ActiveLanguageDefinition.PrimitiveTypes.at(queryTokenVariable.Type.Value);
				if (queryPrimitiveType.AcceptableLiteral == variablePrimitiveType.AcceptableLiteral)
				{
					return true;
				}
				
			}
			// If token is a primitive type, simply assure that the types match
			if (queryToken.Type == ScriptTokenType::PrimitiveType)
			{
				// Ensure token primitive type is valid
				if (!ScriptCompilerService::s_ActiveLanguageDefinition.PrimitiveTypes.contains(queryToken.Value))
				{
					return false;
				}

				// Make sure that the literal types match
				PrimitiveType tokenPrimitiveType = ScriptCompilerService::s_ActiveLanguageDefinition.PrimitiveTypes.at(queryToken.Value);
				if (queryPrimitiveType.AcceptableLiteral == tokenPrimitiveType.AcceptableLiteral)
				{
					// Check to ensure that asset value are distinct
					if (queryPrimitiveType.AcceptableLiteral == ScriptTokenType::CustomLiteral)
					{
						if (queryPrimitiveType.Name == queryToken.Value)
						{
							return true;
						}
						return false;
					}

					// Ensure types match if both do not accept a literal
					if (queryPrimitiveType.AcceptableLiteral == ScriptTokenType::None)
					{
						if (queryPrimitiveType.Name == tokenPrimitiveType.Name)
						{
							return true;
						}
						else
						{
							return false;
						}
					}
					// Otherwise, just return as valid
					return true;
				}
			}
		}

		return false;
	}

	ScriptToken ScriptTokenParser::GetPrimitiveTypeFromToken(Scripting::ScriptToken token)
	{
		if (ScriptCompilerService::IsLiteral(token))
		{
			for (auto& [name, primitiveType] : ScriptCompilerService::s_ActiveLanguageDefinition.PrimitiveTypes)
			{
				if (token.Type == primitiveType.AcceptableLiteral)
				{
					return { ScriptTokenType::PrimitiveType, primitiveType.Name };
				}
			}
		}

		if (token.Type == ScriptTokenType::Identifier)
		{
			StackVariable variable = GetStackVariable(token);
			if (!variable)
			{
				return {};
			}

			if (variable.Type.Type == ScriptTokenType::PrimitiveType)
			{
				return { ScriptTokenType::PrimitiveType, variable.Type.Value };
			}

			return {};
		}

		if (token.Type == ScriptTokenType::PrimitiveType)
		{
			return token;
		}
		return {};
	}
}
