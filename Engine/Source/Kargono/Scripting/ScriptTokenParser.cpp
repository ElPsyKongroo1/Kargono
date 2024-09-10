#include "kgpch.h"
#include "Kargono/Scripting/ScriptTokenParser.h"
#include "Kargono/Scripting/ScriptCompilerService.h"

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
		std::visit([&](auto&& expressionValue)
			{
				using type = std::decay_t<decltype(expressionValue)>;
				if constexpr (std::is_same_v<type, Scripting::ScriptToken>)
				{
					KG_INFO("{}Expression Token", GetIndentation(indentation));
					KG_INFO("{}Expression Value", GetIndentation(indentation + 1));
					PrintToken(expressionValue, indentation + 2);
				}
				else if constexpr (std::is_same_v<type, Scripting::FunctionCallNode>)
				{
					KG_INFO("{}Expression Function Call", GetIndentation(indentation));
					KG_INFO("{}Namespace", GetIndentation(indentation + 1));
					PrintToken(expressionValue.Namespace, indentation + 2);
					KG_INFO("{}Identifier", GetIndentation(indentation + 1));
					PrintToken(expressionValue.Identifier, indentation + 2);
					KG_INFO("{}Return Type", GetIndentation(indentation + 1));
					PrintToken(expressionValue.ReturnType, indentation + 2);
					for (auto& argument : expressionValue.Arguments)
					{
						KG_INFO("{}Argument", GetIndentation(indentation + 1));
						PrintExpression(argument, indentation + 2);
					}
				}
				else if constexpr (std::is_same_v<type, Scripting::InitializationListNode>)
				{
					KG_INFO("{}Return Type", GetIndentation(indentation + 1));
					PrintToken(expressionValue.ReturnType, indentation + 2);
					for (auto& argument : expressionValue.Arguments)
					{
						KG_INFO("{}Argument", GetIndentation(indentation + 1));
						PrintExpression(argument, indentation + 2);
					}
				}
				else if constexpr (std::is_same_v<type, Scripting::UnaryOperationNode>)
				{
					KG_INFO("{}Expression Unary Operation", GetIndentation(indentation));
					KG_INFO("{}Operand", GetIndentation(indentation + 1));
					PrintToken(expressionValue.Operand, indentation + 2);
					KG_INFO("{}Operator", GetIndentation(indentation + 1));
					PrintToken(expressionValue.Operator, indentation + 2);
					KG_INFO("{}Return Type", GetIndentation(indentation + 1));
					PrintToken(expressionValue.ReturnType, indentation + 2);
				}
				else if constexpr (std::is_same_v<type, Scripting::BinaryOperationNode>)
				{
					KG_INFO("{}Expression Binary Operation", GetIndentation(indentation));
					KG_INFO("{}Operand 1", GetIndentation(indentation + 1));
					PrintExpression(expressionValue.LeftOperand, indentation + 2);
					KG_INFO("{}Operand 2", GetIndentation(indentation + 1));
					PrintExpression(expressionValue.RightOperand, indentation + 2);
					KG_INFO("{}Operator", GetIndentation(indentation + 1));
					PrintToken(expressionValue.Operator, indentation + 2);
					KG_INFO("{}Return Type", GetIndentation(indentation + 1));
					PrintToken(expressionValue.ReturnType, indentation + 2);
				}
			}, expression->Value);
	}

	static void PrintStatement(const Ref<Scripting::Statement> statement, uint32_t indentation = 0)
	{
		if (!statement)
		{
			return;
		}
		std::visit([&](auto&& state)
			{
				using type = std::decay_t<decltype(state)>;
				if constexpr (std::is_same_v<type, Scripting::StatementEmpty>)
				{
					KG_INFO("{}Single Semicolon Statement", GetIndentation(indentation));
				}
				else if constexpr (std::is_same_v<type, Scripting::StatementExpression>)
				{
					KG_INFO("{}Expression Statement", GetIndentation(indentation));
					KG_INFO("{}Expression Value", GetIndentation(indentation + 1));
					PrintExpression(state.Value, indentation + 2);
				}
				else if constexpr (std::is_same_v<type, Scripting::StatementDeclaration>)
				{
					KG_INFO("{}Declaration Statement", GetIndentation(indentation));
					KG_INFO("{}Declaration Type", GetIndentation(indentation + 1));
					PrintToken(state.Type, indentation + 2);
					KG_INFO("{}Declaration Name/Identifier", GetIndentation(indentation + 1));
					PrintToken(state.Name, indentation + 2);
				}
				else if constexpr (std::is_same_v<type, Scripting::StatementAssignment>)
				{
					KG_INFO("{}Assignment Statement", GetIndentation(indentation));
					KG_INFO("{}Assignment Identifier", GetIndentation(indentation + 1));
					PrintToken(state.Name, indentation + 2);
					KG_INFO("{}Assignment Value", GetIndentation(indentation + 1));
					PrintExpression(state.Value, indentation + 2);
				}
				else if constexpr (std::is_same_v<type, Scripting::StatementDeclarationAssignment>)
				{
					KG_INFO("{}Declaration/Assignment Statement", GetIndentation(indentation));
					KG_INFO("{}Declared Type", GetIndentation(indentation + 1));
					PrintToken(state.Type, indentation + 2);
					KG_INFO("{}Declared Identifier", GetIndentation(indentation + 1));
					PrintToken(state.Name, indentation + 2);
					KG_INFO("{}Assignment Value", GetIndentation(indentation + 1));
					PrintExpression(state.Value, indentation + 2);
				}
				else if constexpr (std::is_same_v<type, Scripting::StatementConditional>)
				{
					KG_INFO("{}Conditional Statement", GetIndentation(indentation));
					KG_INFO("{}Type", GetIndentation(indentation + 1));
					KG_INFO("{}{}", GetIndentation(indentation + 2), Utility::ConditionalTypeToString(state.Type));
					KG_INFO("{}Condition Expression", GetIndentation(indentation + 1));
					PrintExpression(state.ConditionExpression, indentation + 2);
					KG_INFO("{}Body Statements", GetIndentation(indentation + 1));
					for (auto bodyStatement : state.BodyStatements)
					{
						PrintStatement(bodyStatement, indentation + 2);
					}
					KG_INFO("{}Chained Conditional Statements", GetIndentation(indentation + 1));
					for (auto chainedStatement : state.ChainedConditionals)
					{
						PrintStatement(chainedStatement, indentation + 2);
					}
					
					
				}
			}, statement->Value);
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

		m_AST.ProgramNode = { newFunctionNode };

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
		if (m_AST.ProgramNode)
		{
			FunctionNode& funcNode = m_AST.ProgramNode.FuncNode;
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
		if (tokenBuffer.Type != ScriptTokenType::Keyword || tokenBuffer.Value != "void")
		{
			StoreParseError(ParseErrorType::Function, "Invalid/Empty return type provided for function signature", tokenBuffer);
			return { false, newFunctionNode };
		}
		newFunctionNode.ReturnType = tokenBuffer;

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
			ScriptToken newReturnType;
			newReturnType.Type = ScriptTokenType::PrimitiveType;
			newReturnType.Value = "None";
			newContext.AllReturnTypes.push_back(newReturnType);
			newContext.IsFunctionParameter = true;
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
					ScriptToken newReturnType;
					newReturnType.Type = ScriptTokenType::PrimitiveType;
					newReturnType.Value = "None";
					newContext.AllReturnTypes.push_back(newReturnType);
					newContext.IsFunctionParameter = true;
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

		// Check for addition / subtraction binary operations
		while (ScriptCompilerService::IsAdditionOrSubtractionOperator(GetCurrentToken(parentExpressionSize)))
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
				newContext.StackVariables = m_StackVariables;
				m_CursorContext = newContext;
				StoreParseError(ParseErrorType::ContextProbe, "Found context probe in left operand of addition/subtraction operation", newBinaryOperation.Operator);
				return { false, {} };
			}

			if (IsContextProbe(newBinaryOperation.RightOperand))
			{
				CursorContext newContext;
				newContext.AllReturnTypes.push_back(GetPrimitiveTypeFromToken(newBinaryOperation.LeftOperand->GetReturnType()));
				newContext.StackVariables = m_StackVariables;
				m_CursorContext = newContext;
				StoreParseError(ParseErrorType::ContextProbe, "Found context probe in right operand of addition/subtraction operation", newBinaryOperation.Operator);
				return { false, {} };
			}

			// Ensure the return type of both operands is identical
			if (GetPrimitiveTypeFromToken(newBinaryOperation.LeftOperand->GetReturnType()).Value !=
				GetPrimitiveTypeFromToken(newBinaryOperation.RightOperand->GetReturnType()).Value)
			{
				std::string errorMessage = fmt::format("Return types do not match in binary expression\n Left operand return type: {}\n Right operand return type: {}",
					GetPrimitiveTypeFromToken(newBinaryOperation.LeftOperand->GetReturnType()).Value,
					GetPrimitiveTypeFromToken(newBinaryOperation.RightOperand->GetReturnType()).Value);
				StoreParseError(ParseErrorType::Expression, errorMessage, newBinaryOperation.Operator);
				return { false, {} };
			}

			// Store Return Type
			newBinaryOperation.ReturnType = GetPrimitiveTypeFromToken(newBinaryOperation.LeftOperand->GetReturnType());

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
					newContext.StackVariables = m_StackVariables;
					m_CursorContext = newContext;
					StoreParseError(ParseErrorType::ContextProbe, "Found context probe in left operand of multiplication/division/comparison operation", newBinaryOperation.Operator);
					return { false, {} };
				}

				if (IsContextProbe(newBinaryOperation.RightOperand))
				{
					CursorContext newContext;
					newContext.AllReturnTypes.push_back(GetPrimitiveTypeFromToken(newBinaryOperation.LeftOperand->GetReturnType()));
					newContext.StackVariables = m_StackVariables;
					m_CursorContext = newContext;
					StoreParseError(ParseErrorType::ContextProbe, "Found context probe in right operand of multiplication/division/comparison operation", newBinaryOperation.Operator);
					return { false, {} };
				}

				// Ensure the return type of both operands is identical
				if (GetPrimitiveTypeFromToken(newBinaryOperation.LeftOperand->GetReturnType()).Value !=
					GetPrimitiveTypeFromToken(newBinaryOperation.RightOperand->GetReturnType()).Value)
				{
					std::string errorMessage = fmt::format("Return types do not match in binary expression\n Left operand return type: {}\n Right operand return type: {}",
						GetPrimitiveTypeFromToken(newBinaryOperation.LeftOperand->GetReturnType()).Value,
						GetPrimitiveTypeFromToken(newBinaryOperation.RightOperand->GetReturnType()).Value);
					StoreParseError(ParseErrorType::Expression, errorMessage, newBinaryOperation.Operator);
					return { false, {} };
				}

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

				newBinaryExpression->Value = newBinaryOperation;
				newExpression = newBinaryExpression;
			}
		}

		return { true, newExpression };
	}
	std::tuple<bool, Ref<Expression>> ScriptTokenParser::ParseExpressionLiteral(uint32_t& parentExpressionSize)
	{
		Ref<Expression> newExpression {CreateRef<Expression>()};
		// Check for a single literal/identifier
		ScriptToken tokenBuffer = GetCurrentToken(parentExpressionSize);
		if (!ScriptCompilerService::IsLiteral(tokenBuffer))
		{
			return { false, {} };
		}
		newExpression->Value = tokenBuffer;
		parentExpressionSize++;
		return { true, newExpression };
	}
	std::tuple<bool, Ref<Expression>> ScriptTokenParser::ParseExpressionIdentifier(uint32_t& parentExpressionSize)
	{
		Ref<Expression> newExpression{ CreateRef<Expression>() };
		// Check for a single literal/identifier
		ScriptToken tokenBuffer = GetCurrentToken(parentExpressionSize);
		if ((tokenBuffer.Type != ScriptTokenType::Identifier))
		{
			return { false, {} };
		}

		if (!IsContextProbe(tokenBuffer) && tokenBuffer.Type == ScriptTokenType::Identifier && !CheckStackForIdentifier(tokenBuffer))
		{
			StoreParseError(ParseErrorType::Expression, "Unknown variable", tokenBuffer);
			return { false, {} };
		}

		newExpression->Value = tokenBuffer;
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
		newUnaryOperation.ReturnType = GetPrimitiveTypeFromToken(newUnaryOperation.Operand);

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
						ScriptToken newReturnType;
						newReturnType.Type = ScriptTokenType::PrimitiveType;
						newReturnType.Value = "None";
						newContext.AllReturnTypes.push_back(newReturnType);
						newContext.StackVariables = m_StackVariables;
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
			ScriptToken newReturnType;
			newReturnType.Type = ScriptTokenType::PrimitiveType;
			newReturnType.Value = "None";
			newContext.AllReturnTypes.push_back(newReturnType);
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
		Ref<Expression> newExpression{ CreateRef<Expression>() };
		ScriptToken tokenBuffer = GetCurrentToken();

		// Check for Type, Identifier, and Assignment Operator
		if (tokenBuffer.Type != ScriptTokenType::Identifier ||
			GetCurrentToken(1).Type != ScriptTokenType::AssignmentOperator)
		{
			return { false, {} };
		}

		// Check for an expression
		uint32_t expressionSize{ 0 };
		{
			Advance(2);
			auto [success, expression] = ParseExpressionNode(expressionSize);
			Advance(-2);
			if (!success)
			{
				return { false, {} };
			}
			newExpression = expression;
		}

		// Check for context probe
		if (IsContextProbe(newExpression) && CheckStackForIdentifier(tokenBuffer))
		{
			StackVariable currentIdentifierVariable = GetStackVariable(tokenBuffer);
			CursorContext newContext;
			newContext.AllReturnTypes.push_back(currentIdentifierVariable.Type);
			newContext.StackVariables = m_StackVariables;
			m_CursorContext = newContext;
			StoreParseError(ParseErrorType::ContextProbe, "Found context probe in statement assignment", tokenBuffer);
			return { false, {} };
		}

		// Check for semicolon
		if (GetCurrentToken(2 + expressionSize).Type != ScriptTokenType::Semicolon)
		{
			return { false, {} };
		}

		if (CheckForErrors())
		{
			StoreParseError(ParseErrorType::Statement, "Invalid expression provided in assignment statement", tokenBuffer);
			return { false, {} };
		}

		// Ensure identifer is a valid StackVariable
		if (!CheckStackForIdentifier(tokenBuffer))
		{
			StoreParseError(ParseErrorType::Statement, "Unknown variable found in assignment statement", tokenBuffer);
			return { false, {} };
		}

		// Ensure expression value is a valid type to be assigned to identifier
		StackVariable currentIdentifierVariable = GetStackVariable(tokenBuffer);
		bool success = PrimitiveTypeAcceptableToken(currentIdentifierVariable.Type.Value, newExpression->GetReturnType());
		if (!success)
		{
			std::string errorMessage =
				fmt::format("Invalid expression return type provided in assignment statement\n Declared type: {}\n ExpressionType: {}",
					currentIdentifierVariable.Type.Value, GetPrimitiveTypeFromToken(newExpression->GetReturnType()).Value);
			StoreParseError(ParseErrorType::Statement, errorMessage, GetCurrentToken(1));
			return { false, {} };
		}

		Ref<Statement> newStatement = CreateRef<Statement>();
		StatementAssignment newStatementAssignment{ tokenBuffer, newExpression };
		newStatement->Value = newStatementAssignment;
		Advance(3 + expressionSize);
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

		if (token.Type == ScriptTokenType::Identifier && token.Value == ContextProbe)
		{
			return true;
		}

		return false;
	}

	bool ScriptTokenParser::IsContextProbe(Ref<Expression> expression)
	{
		if (const ScriptToken* token = std::get_if<ScriptToken>(&expression->Value))
		{
			if (token->Type == ScriptTokenType::Identifier && token->Value == ContextProbe)
			{
				return true;
			}
		}

		return false;
	}

	bool ScriptTokenParser::PrimitiveTypeAcceptableToken(const std::string& type, Scripting::ScriptToken token)
	{
		// Search all primitive types to check if token is acceptable
		for (auto& primitiveType : ScriptCompilerService::s_ActiveLanguageDefinition.PrimitiveTypes)
		{
			if (type == primitiveType.Name)
			{
				if (token.Type == primitiveType.AcceptableLiteral)
				{
					return true;
				}
				if (token.Type == ScriptTokenType::Identifier)
				{
					StackVariable variable = GetStackVariable(token);
					if (!variable)
					{
						return false;
					}

					if (variable.Type.Type == ScriptTokenType::PrimitiveType && variable.Type.Value == primitiveType.Name)
					{
						return true;
					}

					return false;
				}
				if (token.Type == ScriptTokenType::PrimitiveType && token.Value == type)
				{
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
			for (auto& primitiveType : ScriptCompilerService::s_ActiveLanguageDefinition.PrimitiveTypes)
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
