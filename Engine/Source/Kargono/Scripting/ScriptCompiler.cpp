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
			if (std::isalpha(GetCurrentChar()))
			{
				// Add first character to buffer
				AddCurrentCharToBuffer();
				Advance();

				// Fill remainder of buffer
				while (CurrentLocationValid() && std::isalnum(GetCurrentChar()))
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
						AddTokenAndClearBuffer({ ScriptTokenType::Keyword, {keyword} });
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
						AddTokenAndClearBuffer({ ScriptTokenType::PrimitiveType, {primitiveType} });
						foundPrimitiveType = true;
						break;
					}
				}
				if (foundPrimitiveType)
				{
					continue;
				}

				AddTokenAndClearBuffer({ ScriptTokenType::Identifier, m_TextBuffer });
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
				AddTokenAndClearBuffer({ ScriptTokenType::IntegerLiteral, m_TextBuffer });
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
				AddTokenAndClearBuffer({ ScriptTokenType::StringLiteral, m_TextBuffer });
				continue;
			}

			if (GetCurrentChar() == ';')
			{
				AddTokenAndClearBuffer({ ScriptTokenType::Semicolon, {} });
				Advance();
				continue;
			}

			if (GetCurrentChar() == '(')
			{
				AddTokenAndClearBuffer({ ScriptTokenType::OpenParentheses, {} });
				Advance();
				continue;
			}

			if (GetCurrentChar() == ')')
			{
				AddTokenAndClearBuffer({ ScriptTokenType::CloseParentheses, {} });
				Advance();
				continue;
			}

			if (GetCurrentChar() == '{')
			{
				AddTokenAndClearBuffer({ ScriptTokenType::OpenCurlyBrace, {} });
				Advance();
				continue;
			}

			if (GetCurrentChar() == '}')
			{
				AddTokenAndClearBuffer({ ScriptTokenType::CloseCurlyBrace, {} });
				Advance();
				continue;
			}

			if (GetCurrentChar() == '=')
			{
				AddTokenAndClearBuffer({ ScriptTokenType::AssignmentOperator, {} });
				Advance();
				continue;
			}

			if (GetCurrentChar() == '+')
			{
				AddTokenAndClearBuffer({ ScriptTokenType::AdditionOperator, {} });
				Advance();
				continue;
			}

			if (GetCurrentChar() == ',')
			{
				AddTokenAndClearBuffer({ ScriptTokenType::Comma, {} });
				Advance();
				continue;
			}

			if (GetCurrentChar() == ':' && GetCurrentChar(1) == ':')
			{
				AddTokenAndClearBuffer({ ScriptTokenType::NamespaceResolver, {} });
				Advance(2);
				continue;
			}

			KG_ERROR("Could not identify character!");
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
		m_TextLocation += count;
	}

	void ScriptTokenizer::AddTokenAndClearBuffer(const ScriptToken& token)
	{
		m_Tokens.push_back(token);
		m_TextBuffer.clear();
	}

	std::tuple<bool, ScriptAST> TokenParser::ParseTokens(std::vector<ScriptToken> tokens)
	{
		m_Tokens = std::move(tokens);

		FunctionNode newFunctionNode{};
		// Store return value
		ScriptToken& tokenBuffer = GetCurrentToken();
		if (tokenBuffer.Type != ScriptTokenType::Keyword || tokenBuffer.Value != "void")
		{
			KG_WARN("Expecting a return type in function signature. Got a {} with a value of {}",
				Utility::ScriptTokenTypeToString(tokenBuffer.Type), tokenBuffer.Value);
			return {false, m_AST};
		}
		// TODO: Store return type if applicable

		// Get Function Name
		Advance();
		tokenBuffer = GetCurrentToken();
		if (tokenBuffer.Type != ScriptTokenType::Identifier)
		{
			KG_WARN("Expecting a function name in function signature. Got a {} with a value of {}",
				Utility::ScriptTokenTypeToString(tokenBuffer.Type), tokenBuffer.Value);
			return { false, m_AST };
		}
		newFunctionNode.Name = {tokenBuffer};


		// Parameter Open Parentheses
		Advance();
		tokenBuffer = GetCurrentToken();
		if (tokenBuffer.Type != ScriptTokenType::OpenParentheses)
		{
			KG_WARN("Expecting an open parentheses in function signature (\"(\"). Got a {} with a value of {}",
				Utility::ScriptTokenTypeToString(tokenBuffer.Type), tokenBuffer.Value);
			return { false, m_AST };
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
				KG_WARN("Expecting an identifier for parameter in function signature. Got a {} with a value of {}",
					Utility::ScriptTokenTypeToString(tokenBuffer.Type), tokenBuffer.Value);
				return { false, m_AST };
			}
			newParameter.Identifier = tokenBuffer;
			newFunctionNode.Parameters.push_back(newParameter);


			// Check for comma
			Advance();
			tokenBuffer = GetCurrentToken();
			if (tokenBuffer.Type == ScriptTokenType::Comma)
			{
				if (GetCurrentToken(1).Type != ScriptTokenType::Identifier)
				{
					KG_WARN("Expecting an identifier after comma separator for parameter list in function signature. Got a {} with a value of {}",
						Utility::ScriptTokenTypeToString(tokenBuffer.Type), tokenBuffer.Value);
					return { false, m_AST };
				}

				// Move to next token if comma is present
				Advance();
			}
		}
		// Check for close parentheses
		tokenBuffer = GetCurrentToken();
		if (tokenBuffer.Type != ScriptTokenType::CloseParentheses)
		{
			KG_WARN("Expecting an closing parentheses in function signature (\")\"). Got a {} with a value of {}",
				Utility::ScriptTokenTypeToString(tokenBuffer.Type), tokenBuffer.Value);
			return { false, m_AST };
		}


		// Check for open curly braces
		Advance();
		tokenBuffer = GetCurrentToken();
		if (tokenBuffer.Type != ScriptTokenType::OpenCurlyBrace)
		{
			KG_WARN("Expecting an opening curly brace in function signature. Got a {} with a value of {}",
				Utility::ScriptTokenTypeToString(tokenBuffer.Type), tokenBuffer.Value);
			return { false, m_AST };
		}

		// TODO: Process list of statements

		// Check for close curly braces
		Advance();
		tokenBuffer = GetCurrentToken();
		if (tokenBuffer.Type != ScriptTokenType::CloseCurlyBrace)
		{
			KG_WARN("Expecting an closing curly brace in function signature. Got a {} with a value of {}",
				Utility::ScriptTokenTypeToString(tokenBuffer.Type), tokenBuffer.Value);
			return { false, m_AST };
		}

		m_AST.ProgramNode = { newFunctionNode };

		return { true, m_AST };
	}
	void TokenParser::PrintAST()
	{
		if (m_AST.ProgramNode)
		{
			FunctionNode& funcNode = m_AST.ProgramNode.FuncNode;
			KG_WARN("Function Node");
			KG_WARN(" Name:");
			KG_WARN("  Type: {}", Utility::ScriptTokenTypeToString(funcNode.Name.Type));
			KG_WARN("  Value: {}", funcNode.Name.Value);
			for (auto& parameter : funcNode.Parameters)
			{
				KG_WARN(" Parameter:");
				KG_WARN("  ParameterType:");
				KG_WARN("   Type: {}", Utility::ScriptTokenTypeToString(parameter.ParameterType.Type));
				KG_WARN("   Value: {}", parameter.ParameterType.Value);
				KG_WARN("  Identifier:");
				KG_WARN("   Type: {}", Utility::ScriptTokenTypeToString(parameter.Identifier.Type));
				KG_WARN("   Value: {}", parameter.Identifier.Value);
			}
		}
	}
	ScriptToken& TokenParser::GetCurrentToken(int32_t offset)
	{
		// Return empty token if end of file reached
		if (m_TokenLocation + offset >= m_Tokens.size())
		{
			static ScriptToken s_EmptyToken{};
			s_EmptyToken = {};
			return s_EmptyToken;
		}
		return m_Tokens.at(m_TokenLocation + offset);
	}
	void TokenParser::Advance(uint32_t count)
	{
		m_TokenLocation += count;
	}
}
