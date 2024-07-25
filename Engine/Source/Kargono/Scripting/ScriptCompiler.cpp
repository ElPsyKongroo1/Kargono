#include "kgpch.h"

#include "Kargono/Scripting/ScriptCompiler.h"

#include "Kargono/Utility/FileSystem.h"

namespace Kargono::Scripting
{
	std::string ScriptCompiler::CompileScriptFile(const std::filesystem::path& scriptLocation)
	{
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
				for (auto& keyword : m_Keywords)
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
				for (auto& primitiveType : m_PrimitiveTypes)
				{
					if (m_TextBuffer == primitiveType)
					{
						AddTokenAndClearBuffer(ScriptTokenType::PrimitiveType, {primitiveType});
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

	static void PrintStatement(const Statement& statement, uint32_t indentation = 0)
	{
		std::visit([&](auto&& state)
		{
			using type = std::decay_t<decltype(state)>;
			if constexpr (std::is_same_v<type, StatementEmpty>)
			{
				KG_INFO("{}Single Semicolon Statement", GetIndentation(indentation));
			}
			else if constexpr (std::is_same_v<type, StatementLiteral>)
			{
				KG_INFO("{}Literal Statement", GetIndentation(indentation));
				KG_INFO("{}Literal Value", GetIndentation(indentation + 1));
				PrintToken(state.ExpressionValue, indentation + 2);
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
				KG_INFO("{}Assignemnt Declared Type", GetIndentation(indentation + 1));
				PrintToken(state.Type, indentation + 2);
				KG_INFO("{}Declared Name/Identifier", GetIndentation(indentation + 1));
				PrintToken(state.Name, indentation + 2);
				KG_INFO("{}Assignment Value", GetIndentation(indentation + 1));
				PrintToken(state.Value, indentation + 2);
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
			PrintToken(parameter.ParameterType, indentation + 3);
			KG_INFO("{}ParameterName:", GetIndentation(indentation + 2));
			PrintToken(parameter.ParameterName, indentation + 3);
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
		Statement newStatementEmpty{};

		ScriptToken tokenBuffer = GetCurrentToken();
		if (tokenBuffer.Type == ScriptTokenType::Semicolon)
		{
			newStatementEmpty.emplace<StatementEmpty>();
			Advance();
			return { true, newStatementEmpty };
		}

		if ((tokenBuffer.Type == ScriptTokenType::StringLiteral || tokenBuffer.Type == ScriptTokenType::IntegerLiteral) 
			&& GetCurrentToken(1).Type == ScriptTokenType::Semicolon)
		{
			StatementLiteral newStatementLiteral{ tokenBuffer };
			newStatementEmpty.emplace<StatementLiteral>(newStatementLiteral);
			Advance(2);
			return { true, newStatementEmpty };
		}

		if (tokenBuffer.Type == ScriptTokenType::PrimitiveType && GetCurrentToken(1).Type == ScriptTokenType::Identifier
			&& GetCurrentToken(2).Type == ScriptTokenType::Semicolon)
		{
			StatementDeclaration newStatementDeclaration{ tokenBuffer, GetCurrentToken(1)};
			newStatementEmpty.emplace<StatementDeclaration>(newStatementDeclaration);
			Advance(3);
			return { true, newStatementEmpty };
		}

		if (tokenBuffer.Type == ScriptTokenType::PrimitiveType && GetCurrentToken(1).Type == ScriptTokenType::Identifier
			&& GetCurrentToken(2).Type == ScriptTokenType::AssignmentOperator
			&& (GetCurrentToken(3).Type == ScriptTokenType::StringLiteral || GetCurrentToken(3).Type == ScriptTokenType::IntegerLiteral)
			&& GetCurrentToken(4).Type == ScriptTokenType::Semicolon)
		{
			std::vector<ScriptTokenType> acceptableTypes = Utility::PrimitiveTypeAcceptableInput(GetCurrentToken().Value);
			bool success = false;
			for (ScriptTokenType type : acceptableTypes)
			{
				if (type == GetCurrentToken(3).Type)
				{
					success = true;
				}
			}
			if (!success)
			{
				StoreParseError(ParseErrorType::StateValue, "Invalid assignment statement. Value cannot be assigned to provided type");
				return { false, newStatementEmpty };
			}

			StatementAssignment newStatementAssignment{ tokenBuffer, GetCurrentToken(1), GetCurrentToken(3) };
			newStatementEmpty.emplace<StatementAssignment>(newStatementAssignment);
			Advance(5);
			return { true, newStatementEmpty };
		}



		return { false, newStatementEmpty };
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
		Advance();
		tokenBuffer = GetCurrentToken();
		while (tokenBuffer.Type == ScriptTokenType::PrimitiveType)
		{
			FunctionParameter newParameter{};
			// Store type of current parameter
			newParameter.ParameterType = tokenBuffer;

			// Check and store parameter name
			Advance();
			tokenBuffer = GetCurrentToken();
			if (tokenBuffer.Type != ScriptTokenType::Identifier)
			{
				StoreParseError(ParseErrorType::FuncParam, "Expecting an identifier for parameter in function signature");
				return { false, newFunctionNode };
			}
			newParameter.ParameterName = tokenBuffer;
			newFunctionNode.Parameters.push_back(newParameter);


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

		return { true, newFunctionNode };
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
	void TokenParser::StoreParseError(ParseErrorType errorType, const std::string& message)
	{
		m_Errors.push_back({ errorType , message, GetCurrentToken(), 
			GetCurrentToken(-1) });
	}

	bool TokenParser::CheckForErrors()
	{
		return m_Errors.size() > 0;
	}

}
