#include "kgpch.h"

#include "Kargono/Scripting/ScriptCompiler.h"

#include "Kargono/Utility/FileSystem.h"

namespace Kargono::Scripting
{
	LanguageDefinition ScriptCompiler::s_ActiveLanguageDefinition {};

	std::string ScriptCompiler::CompileScriptFile(const std::filesystem::path& scriptLocation)
	{
		if (!s_ActiveLanguageDefinition)
		{
			CreateKGScriptLanguageDefinition();
		}

		// Check for invalid input
		if (!std::filesystem::exists(scriptLocation))
		{
			KG_WARN("Failed to compile .kgscript. File does not exist at specified location!");
			return {};
		}

		if (scriptLocation.extension() != ".kgscript")
		{
			KG_WARN("Failed to compile .kgscript. File uses incorrect extension!");
			return {};
		}

		// Load in script file from disk
		std::string scriptFile = Utility::FileSystem::ReadFileString(scriptLocation);

		ScriptTokenizer scriptTokenizer{};
		// Get Tokens from Text
		std::vector<ScriptToken> tokens = scriptTokenizer.TokenizeString(std::move(scriptFile));

		for (auto& token : tokens)
		{
			KG_TRACE_CRITICAL("Token: Type ({}) Value ({}) at {}:{}", 
				Utility::ScriptTokenTypeToString(token.Type), token.Value, token.Line, token.Column );
		}

		TokenParser tokenParser{};
		auto [success, newAST] = tokenParser.ParseTokens(std::move(tokens));


		if (!success)
		{
			KG_WARN("Token parsing failed");
			// Print out error messages
			tokenParser.PrintErrors();
		}

		tokenParser.PrintAST();

		return "Hello";
	}
	void ScriptCompiler::CreateKGScriptLanguageDefinition()
	{
		s_ActiveLanguageDefinition = {};
		s_ActiveLanguageDefinition.Keywords = 
		{ 
			"return", 
			"void" 
		};

		s_ActiveLanguageDefinition.PrimitiveTypes = 
		{
			{"string", ScriptTokenType::StringLiteral},
			{ "uint16", ScriptTokenType::IntegerLiteral }
		};

		FunctionNode newFunctionNode{};
		FunctionParameter newParameter{};

		newFunctionNode.Name = { ScriptTokenType::Identifier, "str" };
		newFunctionNode.ReturnType = { ScriptTokenType::PrimitiveType, "string" };
		newParameter.Type = { ScriptTokenType::PrimitiveType, "uint16"};
		newFunctionNode.Parameters.push_back(newParameter);

		s_ActiveLanguageDefinition.FunctionDefinitions.insert_or_assign(newFunctionNode.Name.Value, newFunctionNode);

		newFunctionNode = {};
		newParameter = {};

	}
	std::vector<ScriptToken> ScriptTokenizer::TokenizeString(std::string text)
	{
		// Initialize Variables
		m_ScriptText = std::move(text);

		while (CurrentLocationValid())
		{
			if (std::isalpha(GetCurrentChar()) || GetCurrentChar() == '_')
			{
				// Add first character to buffer
				AddCurrentCharToBuffer();
				Advance();

				// Fill remainder of buffer
				while (CurrentLocationValid() && (std::isalnum(GetCurrentChar()) || GetCurrentChar() == '_'))
				{
					AddCurrentCharToBuffer();
					Advance();
				}

				// Check for keywords
				bool foundKeyword = false;
				for (auto& keyword : ScriptCompiler::s_ActiveLanguageDefinition.Keywords)
				{
					if (m_TextBuffer == keyword)
					{
						AddTokenAndClearBuffer(ScriptTokenType::Keyword, {keyword});
						foundKeyword = true;
						break;
					}
				}
				if (foundKeyword)
				{
					continue;
				}

				// Check for primitive types
				bool foundPrimitiveType = false;
				for (auto& primitiveType : ScriptCompiler::s_ActiveLanguageDefinition.PrimitiveTypes)
				{
					if (m_TextBuffer == primitiveType.Name)
					{
						AddTokenAndClearBuffer(ScriptTokenType::PrimitiveType, {primitiveType.Name});
						foundPrimitiveType = true;
						break;
					}
				}
				if (foundPrimitiveType)
				{
					continue;
				}

				AddTokenAndClearBuffer(ScriptTokenType::Identifier, m_TextBuffer);
				continue;
			}

			if (std::isdigit(GetCurrentChar()))
			{
				// Add first digit to buffer
				AddCurrentCharToBuffer();
				Advance();

				// Fill remainder of buffer
				while (CurrentLocationValid() && std::isdigit(GetCurrentChar()))
				{
					AddCurrentCharToBuffer();
					Advance();
				}

				// Fill in integer literal
				AddTokenAndClearBuffer(ScriptTokenType::IntegerLiteral, m_TextBuffer);
				continue;
			}

			if (std::isspace(GetCurrentChar()))
			{
				Advance();
				continue;
			}

			if (GetCurrentChar() == '\"')
			{
				// Skip first quotation
				Advance();

				// Fill buffer
				while (CurrentLocationValid() && GetCurrentChar() != '\"')
				{
					AddCurrentCharToBuffer();
					Advance();
				}

				// Move past second quotation
				Advance();

				// Fill in String literal
				AddTokenAndClearBuffer(ScriptTokenType::StringLiteral, m_TextBuffer);
				continue;
			}

			if (GetCurrentChar() == ';')
			{
				AddTokenAndClearBuffer(ScriptTokenType::Semicolon, {});
				Advance();
				continue;
			}

			if (GetCurrentChar() == '(')
			{
				AddTokenAndClearBuffer(ScriptTokenType::OpenParentheses, {});
				Advance();
				continue;
			}

			if (GetCurrentChar() == ')')
			{
				AddTokenAndClearBuffer(ScriptTokenType::CloseParentheses, {});
				Advance();
				continue;
			}

			if (GetCurrentChar() == '{')
			{
				AddTokenAndClearBuffer(ScriptTokenType::OpenCurlyBrace, {});
				Advance();
				continue;
			}

			if (GetCurrentChar() == '}')
			{
				AddTokenAndClearBuffer(ScriptTokenType::CloseCurlyBrace, {});
				Advance();
				continue;
			}

			if (GetCurrentChar() == '=')
			{
				AddTokenAndClearBuffer(ScriptTokenType::AssignmentOperator, {});
				Advance();
				continue;
			}

			if (GetCurrentChar() == '+')
			{
				AddTokenAndClearBuffer(ScriptTokenType::AdditionOperator, {});
				Advance();
				continue;
			}

			if (GetCurrentChar() == '-')
			{
				AddTokenAndClearBuffer(ScriptTokenType::SubtractionOperator, {});
				Advance();
				continue;
			}

			if (GetCurrentChar() == '/')
			{
				AddTokenAndClearBuffer(ScriptTokenType::DivisionOperator, {});
				Advance();
				continue;
			}

			if (GetCurrentChar() == '*')
			{
				AddTokenAndClearBuffer(ScriptTokenType::MultiplicationOperator, {});
				Advance();
				continue;
			}

			if (GetCurrentChar() == ',')
			{
				AddTokenAndClearBuffer(ScriptTokenType::Comma, {} );
				Advance();
				continue;
			}

			if (GetCurrentChar() == ':' && GetCurrentChar(1) == ':')
			{
				AddTokenAndClearBuffer(ScriptTokenType::NamespaceResolver, {});
				Advance(2);
				continue;
			}

			KG_CRITICAL("Could not identify character!");
			Advance();
		}
		return m_Tokens;
	}

	char ScriptTokenizer::GetCurrentChar(int32_t offset)
	{
		return m_ScriptText.at(m_TextLocation + offset);
	}

	bool ScriptTokenizer::CurrentLocationValid()
	{
		return m_TextLocation < m_ScriptText.size();
	}


	void ScriptTokenizer::AddCurrentCharToBuffer()
	{
		m_TextBuffer.push_back(m_ScriptText.at(m_TextLocation));
	}

	void ScriptTokenizer::Advance(uint32_t count)
	{
		for (uint32_t iteration {0}; iteration < count; iteration++)
		{
			if (!CurrentLocationValid())
			{
				return;
			}
			if (m_ScriptText.at(m_TextLocation) == '\n')
			{
				m_ColumnCount = 0;
				m_LineCount++;
			}
			else if (m_ScriptText.at(m_TextLocation) == '\r')
			{
				// Do nothing
			}
			else
			{
				m_ColumnCount++;
			}

			m_TextLocation++;
		}
	}

	void ScriptTokenizer::AddTokenAndClearBuffer(ScriptTokenType type, const std::string& value)
	{
		ScriptToken newToken{ type, value, m_LineCount, m_ColumnCount - (uint32_t)m_TextBuffer.size()};
		m_Tokens.push_back(newToken);
		m_TextBuffer.clear();
	}

	std::tuple<bool, ScriptAST> TokenParser::ParseTokens(std::vector<ScriptToken> tokens)
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
			StoreParseError(ParseErrorType::ProgEnd, "Expecting end of file token");
			return { false, m_AST};
		}

		return { true, m_AST };
	}

	static std::string GetIndentation(uint32_t count)
	{
		std::string outputIndentation {};
		for (uint32_t iteration {0}; iteration < count; iteration++)
		{
			outputIndentation += " ";
		}
		return outputIndentation;
	}

	static void PrintToken(const ScriptToken& token, uint32_t indentation = 0)
	{
		KG_INFO("{}Type: {}", GetIndentation(indentation), Utility::ScriptTokenTypeToString(token.Type));
		KG_INFO("{}Value: {}", GetIndentation(indentation), token.Value);
	}

	static void PrintExpression(const Expression& expression, uint32_t indentation = 0)
	{
		std::visit([&](auto&& expressionValue)
			{
				using type = std::decay_t<decltype(expressionValue)>;
				if constexpr (std::is_same_v<type, ScriptToken>)
				{
					KG_INFO("{}Expression Token", GetIndentation(indentation));
					KG_INFO("{}Expression Value", GetIndentation(indentation + 1));
					PrintToken(expressionValue, indentation + 2);
				}
				else if constexpr (std::is_same_v<type, FunctionCallNode>)
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
						PrintToken(argument, indentation + 2);
					}
				}
				else if constexpr (std::is_same_v<type, UnaryOperationNode>)
				{
					KG_INFO("{}Expression Unary Operation", GetIndentation(indentation));
					KG_INFO("{}Operand", GetIndentation(indentation + 1));
					PrintToken(expressionValue.Operand, indentation + 2);
					KG_INFO("{}Operator", GetIndentation(indentation + 1));
					PrintToken(expressionValue.Operator, indentation + 2);
					KG_INFO("{}Return Type", GetIndentation(indentation + 1));
					PrintToken(expressionValue.ReturnType, indentation + 2);
				}
				else if constexpr (std::is_same_v<type, BinaryOperationNode>)
				{
					KG_INFO("{}Expression Binary Operation", GetIndentation(indentation));
					KG_INFO("{}Operand 1", GetIndentation(indentation + 1));
					PrintToken(expressionValue.Operand1, indentation + 2);
					KG_INFO("{}Operand 2", GetIndentation(indentation + 1));
					PrintToken(expressionValue.Operand2, indentation + 2);
					KG_INFO("{}Operator", GetIndentation(indentation + 1));
					PrintToken(expressionValue.Operator, indentation + 2);
					KG_INFO("{}Return Type", GetIndentation(indentation + 1));
					PrintToken(expressionValue.ReturnType, indentation + 2);
				}
			}, expression.Value);
	}

	static void PrintStatement(const Statement& statement, uint32_t indentation = 0)
	{
		std::visit([&](auto&& state)
		{
			using type = std::decay_t<decltype(state)>;
			if constexpr (std::is_same_v<type, StatementEmpty>)
			{
				KG_INFO("{}Single Semicolon Statement", GetIndentation(indentation));
			}
			else if constexpr (std::is_same_v<type, StatementExpression>)
			{
				KG_INFO("{}Expression Statement", GetIndentation(indentation));
				KG_INFO("{}Expression Value", GetIndentation(indentation + 1));
				PrintExpression(state.Value, indentation + 2);
			}
			else if constexpr (std::is_same_v<type, StatementDeclaration>)
			{
				KG_INFO("{}Declaration Statement", GetIndentation(indentation));
				KG_INFO("{}Declaration Type", GetIndentation(indentation + 1));
				PrintToken(state.Type, indentation + 2);
				KG_INFO("{}Declaration Name/Identifier", GetIndentation(indentation + 1));
				PrintToken(state.Name, indentation + 2);
			}
			else if constexpr (std::is_same_v<type, StatementAssignment>)
			{
				KG_INFO("{}Assignment Statement", GetIndentation(indentation));
				KG_INFO("{}Assignment Identifier", GetIndentation(indentation + 1));
				PrintToken(state.Name, indentation + 2);
				KG_INFO("{}Assignment Value", GetIndentation(indentation + 1));
				PrintExpression(state.Value, indentation + 2);
			}
			else if constexpr (std::is_same_v<type, StatementDeclarationAssignment>)
			{
				KG_INFO("{}Declaration/Assignment Statement", GetIndentation(indentation));
				KG_INFO("{}Declared Type", GetIndentation(indentation + 1));
				PrintToken(state.Type, indentation + 2);
				KG_INFO("{}Declared Identifier", GetIndentation(indentation + 1));
				PrintToken(state.Name, indentation + 2);
				KG_INFO("{}Assignment Value", GetIndentation(indentation + 1));
				PrintExpression(state.Value, indentation + 2);
			}
		}, statement);
	}

	static void PrintFunction(const FunctionNode& funcNode, uint32_t indentation = 0)
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
			PrintToken(parameter.Type, indentation + 3);
			KG_INFO("{}ParameterName:", GetIndentation(indentation + 2));
			PrintToken(parameter.Identifier, indentation + 3);
		}
		for (auto& statement : funcNode.Statements)
		{
			KG_INFO("{}Statement:", GetIndentation(indentation + 1));
			PrintStatement(statement, indentation + 2);
		}
	}

	void TokenParser::PrintAST()
	{
		if (m_AST.ProgramNode)
		{
			FunctionNode& funcNode = m_AST.ProgramNode.FuncNode;
			if (funcNode)
			{
				PrintFunction(funcNode);
			}
		}
	}
	void TokenParser::PrintErrors()
	{
		for (auto& error : m_Errors)
		{
			KG_WARN(error.ToString());
		}
	}


	std::tuple<bool, Statement> TokenParser::ParseStatementNode()
	{
		Statement newStatement{};

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
			return { false, {} };
		}

		return { false, newStatement };
	}

	std::tuple<bool, FunctionNode> TokenParser::ParseFunctionNode()
	{
		FunctionNode newFunctionNode{};
		// Store return value
		ScriptToken tokenBuffer = GetCurrentToken();
		if (tokenBuffer.Type != ScriptTokenType::Keyword || tokenBuffer.Value != "void")
		{
			StoreParseError(ParseErrorType::FuncReturn, "Expecting a return type for function signature");
			return { false, newFunctionNode };
		}
		newFunctionNode.ReturnType = tokenBuffer;

		// Get Function Name
		Advance();
		tokenBuffer = GetCurrentToken();
		if (tokenBuffer.Type != ScriptTokenType::Identifier)
		{
			StoreParseError(ParseErrorType::FuncName, "Expecting a function name in function signature");
			return { false, newFunctionNode };
		}
		newFunctionNode.Name = { tokenBuffer };

		// Parameter Open Parentheses
		Advance();
		tokenBuffer = GetCurrentToken();
		if (tokenBuffer.Type != ScriptTokenType::OpenParentheses)
		{
			StoreParseError(ParseErrorType::FuncPunc, "Expecting an open parentheses in function signature");
			return { false, newFunctionNode };
		}

		// Parameter List
		AddStackFrame();
		Advance();
		tokenBuffer = GetCurrentToken();
		while (tokenBuffer.Type == ScriptTokenType::PrimitiveType)
		{
			FunctionParameter newParameter{};
			// Store type of current parameter
			newParameter.Type = tokenBuffer;

			// Check and store parameter name
			Advance();
			tokenBuffer = GetCurrentToken();
			if (tokenBuffer.Type != ScriptTokenType::Identifier)
			{
				StoreParseError(ParseErrorType::FuncParam, "Expecting an identifier for parameter in function signature");
				return { false, newFunctionNode };
			}
			newParameter.Identifier = tokenBuffer;
			newFunctionNode.Parameters.push_back(newParameter);
			StoreStackVariable(newParameter.Type, newParameter.Identifier);

			// Check for comma
			Advance();
			tokenBuffer = GetCurrentToken();
			if (tokenBuffer.Type == ScriptTokenType::Comma)
			{
				if (GetCurrentToken(1).Type != ScriptTokenType::Identifier)
				{
					StoreParseError(ParseErrorType::FuncParam,
						"Expecting an identifier after comma separator for parameter list in function signature");
					return { false, newFunctionNode };
				}

				// Move to next token if comma is present
				Advance();
			}
		}
		// Check for close parentheses
		tokenBuffer = GetCurrentToken();
		if (tokenBuffer.Type != ScriptTokenType::CloseParentheses)
		{
			StoreParseError(ParseErrorType::FuncPunc, "Expecting an closing parentheses in function signature");
			return { false, newFunctionNode };
		}

		// Check for open curly braces
		Advance();
		tokenBuffer = GetCurrentToken();
		if (tokenBuffer.Type != ScriptTokenType::OpenCurlyBrace)
		{
			StoreParseError(ParseErrorType::FuncPunc, "Expecting an opening curly brace in function signature");
			return { false, newFunctionNode };
		}

		// Parse Statements
		Advance();
		bool success = false;
		Statement statement;
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
			StoreParseError(ParseErrorType::FuncBody, "Invalid function statement");
			return { false, newFunctionNode };
		}

		// Check for close curly braces
		tokenBuffer = GetCurrentToken();
		if (tokenBuffer.Type != ScriptTokenType::CloseCurlyBrace)
		{
			StoreParseError(ParseErrorType::FuncPunc, "Expecting an closing curly brace in function signature");
			return { false, newFunctionNode };
		}

		// Remove current stack variables
		PopStackFrame();

		return { true, newFunctionNode };
	}
	std::tuple<bool, Expression> TokenParser::ParseExpressionNode(uint32_t& expressionSize)
	{
		// Parse Expression Binary Operation
		{
			auto [success, statement] = ParseExpressionBinaryOperation(expressionSize);
			if (success)
			{
				return { success, statement };
			}
		}

		if (CheckForErrors())
		{
			return { false, {} };
		}

		// Parse Expression Unary Operation
		{
			auto [success, statement] = ParseExpressionUnaryOperation(expressionSize);
			if (success)
			{
				return { success, statement };
			}
		}

		if (CheckForErrors())
		{
			return { false, {} };
		}

		// Parse Expression Function Call
		{
			auto [success, statement] = ParseExpressionFunctionCall(expressionSize);
			if (success)
			{
				return { success, statement };
			}
		}

		if (CheckForErrors())
		{
			return { false, {} };
		}

		// Parse Expression Literal
		{
			auto [success, statement] = ParseExpressionLiteral(expressionSize);
			if (success)
			{
				return { success, statement };
			}
		}

		if (CheckForErrors())
		{
			return { false, {} };
		}

		// Parse Expression Identifier
		{
			auto [success, statement] = ParseExpressionIdentifier(expressionSize);
			if (success)
			{
				return { success, statement };
			}
		}

		if (CheckForErrors())
		{
			return { false, {} };
		}

		return { false, {} };
	}
	std::tuple<bool, Expression> TokenParser::ParseExpressionLiteral(uint32_t& expressionSize)
	{
		Expression newExpression{};
		// Check for a single literal/identifier
		ScriptToken tokenBuffer = GetCurrentToken();
		if ((tokenBuffer.Type != ScriptTokenType::StringLiteral &&
			tokenBuffer.Type != ScriptTokenType::IntegerLiteral))
		{
			return { false, {} };
		}

		newExpression.Value = tokenBuffer;
		expressionSize++;
		return { true, newExpression };
	}
	std::tuple<bool, Expression> TokenParser::ParseExpressionIdentifier(uint32_t& expressionSize)
	{
		Expression newExpression{};
		// Check for a single literal/identifier
		ScriptToken tokenBuffer = GetCurrentToken();
		if ((tokenBuffer.Type != ScriptTokenType::Identifier))
		{
			return { false, {} };
		}

		if (tokenBuffer.Type == ScriptTokenType::Identifier && !CheckStackForIdentifier(tokenBuffer))
		{
			StoreParseError(ParseErrorType::StateValue, "Could not locate identifier in current stack!");
			return { false, {} };
		}

		newExpression.Value = tokenBuffer;
		expressionSize++;
		return { true, newExpression };
	}
	std::tuple<bool, Expression> TokenParser::ParseExpressionFunctionCall(uint32_t& expressionSize)
	{
		Expression newExpression{};
		FunctionCallNode newFunctionCallNode{};

		// Check for function namespace, namespace resolver symbol, function identifier, and open parentheses
		ScriptToken tokenBuffer = GetCurrentToken();
		int32_t initialAdvance{ 0 };
		if (tokenBuffer.Type == ScriptTokenType::Identifier &&
			GetCurrentToken(1).Type == ScriptTokenType::NamespaceResolver &&
			GetCurrentToken(2).Type == ScriptTokenType::Identifier &&
			GetCurrentToken(3).Type == ScriptTokenType::OpenParentheses)
		{
			newFunctionCallNode.Namespace = tokenBuffer;
			newFunctionCallNode.Identifier = GetCurrentToken(2);
			initialAdvance = 4;
		}
		else if (tokenBuffer.Type == ScriptTokenType::Identifier &&
			GetCurrentToken(1).Type == ScriptTokenType::OpenParentheses)
		{
			newFunctionCallNode.Identifier = tokenBuffer;
			initialAdvance = 2;
		}
		else
		{
			return { false, {} };
		}

		// Check for arguments and close parentheses
		int32_t argumentTokens{ 0 };
		bool validSyntax = true;
		Advance(initialAdvance);
		while (IsLiteralOrIdentifier(GetCurrentToken()))
		{
			newFunctionCallNode.Arguments.push_back(GetCurrentToken());
			if (GetCurrentToken(1).Type != ScriptTokenType::Comma &&
				GetCurrentToken(1).Type != ScriptTokenType::CloseParentheses)
			{
				validSyntax = false;
				break;
			}

			if (GetCurrentToken(1).Type == ScriptTokenType::CloseParentheses)
			{
				break;
			}
			Advance(2);
			argumentTokens += 2;
		}
		Advance(-initialAdvance + (-argumentTokens));

		if (!validSyntax)
		{
			return { false, {} };
		}

		// Ensure function identifier exists and get function node
		if (!ScriptCompiler::s_ActiveLanguageDefinition.FunctionDefinitions.contains(newFunctionCallNode.Identifier.Value))
		{
			StoreParseError(ParseErrorType::ExpressionValue, "Could not locate function identifier!");
			return { false, {} };
		}
		FunctionNode& functionNode = ScriptCompiler::s_ActiveLanguageDefinition.FunctionDefinitions.at(newFunctionCallNode.Identifier.Value);

		// Ensure number of arguments in function call match the number of arguments in the function
		if (functionNode.Parameters.size() != newFunctionCallNode.Arguments.size())
		{
			StoreParseError(ParseErrorType::ExpressionValue, "Number of arguments in function call does not match the function parameter size");
			return { false, {} };
		}

		// Ensure each argument type matches the parameter type
		uint32_t parameterIteration{ 0 };
		for (auto& parameter : functionNode.Parameters)
		{
			bool valid = PrimitiveTypeAcceptableToken(parameter.Type.Value, newFunctionCallNode.Arguments.at(parameterIteration));
			if (!valid)
			{
				StoreParseError(ParseErrorType::ExpressionValue, "Argument type is not acceptable for function parameter");
				return { false, {} };
			}
			parameterIteration++;
		}

		// Get return type from function node and emplace it into the functionCallNode
		newFunctionCallNode.ReturnType = functionNode.ReturnType;

		// Fill the expression buffer and exit
		newExpression.Value = newFunctionCallNode;
		expressionSize += initialAdvance + argumentTokens + 2;
		return { true, newExpression };
	}
	std::tuple<bool, Expression> TokenParser::ParseExpressionBinaryOperation(uint32_t& expressionSize)
	{
		Expression newExpression{};
		BinaryOperationNode newBinaryOperation{};

		// Check for first operand
		ScriptToken tokenBuffer = GetCurrentToken();
		if (!IsLiteralOrIdentifier(tokenBuffer))
		{
			return { false, {} };
		}
		newBinaryOperation.Operand1 = tokenBuffer;

		// Check for operator (+,-,*,/)
		if (!IsBinaryOperator(GetCurrentToken(1)))
		{
			return { false, {} };
		}
		newBinaryOperation.Operator = GetCurrentToken(1);

		// Check for second operand
		if (!IsLiteralOrIdentifier(GetCurrentToken(2)))
		{
			return { false, {} };
		}
		newBinaryOperation.Operand2 = GetCurrentToken(2);

		// Ensure the return type of both operands is identical
		if (GetPrimitiveTypeFromToken(newBinaryOperation.Operand1).Value != GetPrimitiveTypeFromToken(newBinaryOperation.Operand2).Value)
		{
			StoreParseError(ParseErrorType::ExpressionValue, "Operand return types do not match in binary expression");
			return { false, {} };
		}

		newBinaryOperation.ReturnType = GetPrimitiveTypeFromToken(newBinaryOperation.Operand1);

		// Fill the expression buffer and exit
		newExpression.Value = newBinaryOperation;
		expressionSize += 3;
		return { true, newExpression };
	}
	std::tuple<bool, Expression> TokenParser::ParseExpressionUnaryOperation(uint32_t& expressionSize)
	{
		Expression newExpression{};
		UnaryOperationNode newUnaryOperation{};

		// Check for operator
		if (!IsUnaryOperator(GetCurrentToken()))
		{
			return { false, {} };
		}
		newUnaryOperation.Operator = GetCurrentToken();

		// Check for operand
		if (!IsLiteralOrIdentifier(GetCurrentToken(1)))
		{
			return { false, {} };
		}
		newUnaryOperation.Operand = GetCurrentToken(1);

		// Fill return value
		newUnaryOperation.ReturnType = GetPrimitiveTypeFromToken(newUnaryOperation.Operand);

		// Fill the expression buffer and exit
		newExpression.Value = newUnaryOperation;
		expressionSize += 2;
		return { true, newExpression };
	}
	std::tuple<bool, Statement> TokenParser::ParseStatementEmpty()
	{
		Statement newStatement{};
		ScriptToken tokenBuffer = GetCurrentToken();

		if (tokenBuffer.Type != ScriptTokenType::Semicolon)
		{
			return { false, {} };
		}

		newStatement.emplace<StatementEmpty>();
		Advance();
		return { true, newStatement };
	}
	std::tuple<bool, Statement> TokenParser::ParseStatementExpression()
	{
		Statement newStatement{};
		Expression newExpression{};
		ScriptToken tokenBuffer = GetCurrentToken();

		// Check for an expression
		uint32_t expressionSize{ 0 };
		{
			auto [success, expression] = ParseExpressionNode(expressionSize);

			if (!success)
			{
				return { false, {} };
			}
			newExpression = expression;
		}

		if (CheckForErrors())
		{
			return { false, {} };
		}

		// Check for a terminating semicolon
		if (GetCurrentToken(expressionSize).Type != ScriptTokenType::Semicolon)
		{
			return { false, {} };
		}

		StatementExpression newStatementExpression{ newExpression };
		newStatement.emplace<StatementExpression>(newStatementExpression);
		Advance(1 + expressionSize);
		return { true, newStatement };
	}
	std::tuple<bool, Statement> TokenParser::ParseStatementDeclaration()
	{
		Statement newStatement{};
		ScriptToken tokenBuffer = GetCurrentToken();

		if (tokenBuffer.Type != ScriptTokenType::PrimitiveType || 
			GetCurrentToken(1).Type != ScriptTokenType::Identifier ||
			GetCurrentToken(2).Type != ScriptTokenType::Semicolon)
		{
			return { false, {} };
		}

		if (CheckCurrentStackFrameForIdentifier(GetCurrentToken(1)))
		{
			StoreParseError(ParseErrorType::StateValue, "Duplicate identifier found during declaration");
			return { false, newStatement };
		}

		StatementDeclaration newStatementDeclaration{ tokenBuffer, GetCurrentToken(1) };
		newStatement.emplace<StatementDeclaration>(newStatementDeclaration);
		StoreStackVariable(tokenBuffer, GetCurrentToken(1));
		Advance(3);
		return { true, newStatement };
	}
	std::tuple<bool, Statement> TokenParser::ParseStatementAssignment()
	{
		Statement newStatement{};
		Expression newExpression{};
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

		// Check for semicolon
		if (GetCurrentToken(2 + expressionSize).Type != ScriptTokenType::Semicolon)
		{
			return { false, {} };
		}

		// Ensure identifer is a valid StackVariable
		if (!CheckStackForIdentifier(tokenBuffer))
		{
			StoreParseError(ParseErrorType::StateValue, "Invalid identifier when attempting to parse assignment statement");
			return { false, {} };
		}

		// Ensure expression value is a valid type to be assigned to identifier
		StackVariable currentIdentifierVariable = GetStackVariable(tokenBuffer);
		bool success = PrimitiveTypeAcceptableToken(currentIdentifierVariable.Type.Value, newExpression.GetReturnType());
		if (!success)
		{
			StoreParseError(ParseErrorType::StateValue, "Invalid assignment statement. Value cannot be assigned to provided type");
			return { false, {} };
		}

		StatementAssignment newStatementAssignment{ tokenBuffer, newExpression };
		newStatement.emplace<StatementAssignment>(newStatementAssignment);
		Advance(3 + expressionSize);
		return { true, newStatement };
	}
	std::tuple<bool, Statement> TokenParser::ParseStatementDeclarationAssignment()
	{
		Statement newStatement{};
		Expression newExpression{};
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

		// Check for semicolon
		if (GetCurrentToken(3 + expressionSize).Type != ScriptTokenType::Semicolon)
		{
			return { false, {} };
		}

		// Ensure expression value is a valid type to be assigned to new identifier
		bool success = PrimitiveTypeAcceptableToken(GetCurrentToken().Value, newExpression.GetReturnType());
		if (!success)
		{
			StoreParseError(ParseErrorType::StateValue, "Invalid assignment statement. Value cannot be assigned to provided type");
			return { false, newStatement };
		}

		// Ensure identifer is not being declared twice in the current stack frame
		if (CheckCurrentStackFrameForIdentifier(GetCurrentToken(1)))
		{
			StoreParseError(ParseErrorType::StateValue, "Duplicate identifier found during declaration");
			return { false, newStatement };
		}
		StatementDeclarationAssignment newStatementAssignment{ tokenBuffer, GetCurrentToken(1), newExpression };
		newStatement.emplace<StatementDeclarationAssignment>(newStatementAssignment);
		StoreStackVariable(tokenBuffer, GetCurrentToken(1));
		Advance(4 + expressionSize);
		return { true, newStatement };
	}
	ScriptToken TokenParser::GetCurrentToken(int32_t offset)
	{
		// Return empty token if end of file reached or attempt to access token below 0
		if ((int32_t)m_TokenLocation + offset >= (int32_t)m_Tokens.size())
		{
			return {ScriptTokenType::None, "End of File", InvalidLine, InvalidColumn};
		}

		if ((int32_t)m_TokenLocation + offset < 0)
		{
			return {ScriptTokenType::None, "Index below 0", InvalidLine, InvalidColumn};
		}
		return m_Tokens.at(m_TokenLocation + offset);
	}
	void TokenParser::Advance(uint32_t count)
	{
		m_TokenLocation += count;
	}

	void TokenParser::StoreStackVariable(ScriptToken type, ScriptToken identifier)
	{
		KG_ASSERT(m_StackVariables.size() > 0);

		std::vector<StackVariable>& currentStackFrame = m_StackVariables.back();
		StackVariable newStack{ type, identifier };
		currentStackFrame.push_back(newStack);
	}

	void TokenParser::AddStackFrame()
	{
		m_StackVariables.push_back({});
	}

	void TokenParser::PopStackFrame()
	{
		m_StackVariables.pop_back();
	}

	bool TokenParser::CheckStackForIdentifier(ScriptToken identifier)
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

	bool TokenParser::CheckCurrentStackFrameForIdentifier(ScriptToken identifier)
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

	StackVariable TokenParser::GetStackVariable(ScriptToken identifier)
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
	
	void TokenParser::StoreParseError(ParseErrorType errorType, const std::string& message)
	{
		m_Errors.push_back({ errorType , message, GetCurrentToken(), 
			GetCurrentToken(-1) });
	}

	bool TokenParser::CheckForErrors()
	{
		return m_Errors.size() > 0;
	}

	bool TokenParser::IsLiteralOrIdentifier(ScriptToken token)
	{
		if (IsLiteral(token) || token.Type == ScriptTokenType::Identifier)
		{
			return true;
		}
		return false;
	}

	bool TokenParser::IsLiteral(ScriptToken token)
	{
		if (token.Type == ScriptTokenType::IntegerLiteral ||
			token.Type == ScriptTokenType::StringLiteral)
		{
			return true;
		}
		return false;
	}

	bool TokenParser::IsUnaryOperator(ScriptToken token)
	{
		if (token.Type == ScriptTokenType::SubtractionOperator)
		{
			return true;
		}
		return false;
	}

	bool TokenParser::IsBinaryOperator(ScriptToken token)
	{
		if (token.Type == ScriptTokenType::AdditionOperator ||
			token.Type == ScriptTokenType::SubtractionOperator ||
			token.Type == ScriptTokenType::MultiplicationOperator ||
			token.Type == ScriptTokenType::DivisionOperator)
		{
			return true;
		}
		return false;
	}

	bool TokenParser::PrimitiveTypeAcceptableToken(const std::string& type, Scripting::ScriptToken token)
	{
		// Search all primitive types to check if token is acceptable
		for (auto& primitiveType : ScriptCompiler::s_ActiveLanguageDefinition.PrimitiveTypes)
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
						StoreParseError(ParseErrorType::PrimTypeAccep, "Could not find StackVariable with specified identifier");
						return false;
					}

					if (variable.Type.Type == ScriptTokenType::PrimitiveType && variable.Type.Value == primitiveType.Name)
					{
						return true;
					}

					StoreParseError(ParseErrorType::PrimTypeAccep, "Invalid stack variable type/value provided");
					return false;
				}
				if (token.Type == ScriptTokenType::PrimitiveType && token.Value == type)
				{
					return true;
				}
			}
		}

		StoreParseError(ParseErrorType::PrimTypeAccep, "Invalid primitive type provided");
		return false;
	}

	ScriptToken TokenParser::GetPrimitiveTypeFromToken(Scripting::ScriptToken token)
	{
		if (IsLiteral(token))
		{
			for (auto& primitiveType : ScriptCompiler::s_ActiveLanguageDefinition.PrimitiveTypes)
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
				StoreParseError(ParseErrorType::PrimTypeAccep, "Could not find StackVariable with specified identifier");
				return {};
			}

			if (variable.Type.Type == ScriptTokenType::PrimitiveType)
			{
				return { ScriptTokenType::PrimitiveType, variable.Type.Value };
			}

			StoreParseError(ParseErrorType::PrimTypeAccep, "Invalid stack variable type/value provided");
			return {};
		}
		return {};
	}

}
