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
		ScriptAST syntaxTree = tokenParser.ParseTokens(std::move(tokens));


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

	ScriptAST TokenParser::ParseTokens(std::vector<ScriptToken> tokens)
	{
		m_Tokens = std::move(tokens);

		// Return Value
		ScriptToken& tokenBuffer = GetCurrentToken();
		if (tokenBuffer.Type != ScriptTokenType::Keyword || tokenBuffer.Value != "void")
		{
			// TODO: Error state
		}

		// Function Name
		Advance();
		tokenBuffer = GetCurrentToken();
		if (tokenBuffer.Type == ScriptTokenType::Identifier)
		{
			// TODO: Error state
		}

		// Parameter Open Parentheses
		Advance();
		tokenBuffer = GetCurrentToken();
		if (tokenBuffer.Type != ScriptTokenType::OpenParentheses)
		{
			// TODO: Error state
		}

		// Parameter List
		Advance();
		tokenBuffer = GetCurrentToken();
		// TODO: Check for parameters
		/*while (tokenBuffer.Type == ScriptTokenType::PrimitiveType)
		{

		}*/


		
		tokenBuffer = GetCurrentToken();
		if (tokenBuffer.Type != ScriptTokenType::CloseParentheses)
		{
			// TODO: Error state
		}

		Advance();
		tokenBuffer = GetCurrentToken();

		//m_AST.ProgramNode

		


		return m_AST;
	}
	ScriptToken& TokenParser::GetCurrentToken(int32_t offset)
	{
		return m_Tokens.at(m_TokenLocation + offset);
	}
	void TokenParser::Advance(uint32_t count)
	{
		m_TokenLocation += count;
	}
}
