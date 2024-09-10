#include "kgpch.h"

#include "Kargono/Scripting/ScriptTokenizer.h"
#include "Kargono/Scripting/ScriptCompilerService.h"

namespace Kargono::Scripting
{

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
				for (auto& keyword : ScriptCompilerService::s_ActiveLanguageDefinition.Keywords)
				{
					if (m_TextBuffer == keyword)
					{
						AddTokenAndClearBuffer(ScriptTokenType::Keyword, { keyword });
						foundKeyword = true;
						break;
					}
				}
				if (foundKeyword)
				{
					continue;
				}

				// Check for boolean literals
				if (m_TextBuffer == "true" || m_TextBuffer == "false")
				{
					AddTokenAndClearBuffer(ScriptTokenType::BooleanLiteral, { m_TextBuffer });
					continue;
				}

				// Check for primitive types
				bool foundPrimitiveType = false;
				for (auto& primitiveType : ScriptCompilerService::s_ActiveLanguageDefinition.PrimitiveTypes)
				{
					if (m_TextBuffer == primitiveType.Name)
					{
						AddTokenAndClearBuffer(ScriptTokenType::PrimitiveType, { primitiveType.Name });
						foundPrimitiveType = true;
						break;
					}
				}
				if (foundPrimitiveType)
				{
					continue;
				}

				// If all else fails, store buffer as identifier
				AddTokenAndClearBuffer(ScriptTokenType::Identifier, m_TextBuffer);
				continue;
			}

			if (std::isdigit(GetCurrentChar()))
			{
				// Add first digit to buffer
				AddCurrentCharToBuffer();
				Advance();

				// Fill buffer
				while (CurrentLocationValid() && std::isdigit(GetCurrentChar()))
				{
					AddCurrentCharToBuffer();
					Advance();
				}

				// Check for a float's decimal
				if (GetCurrentChar() == '.' && std::isdigit(GetCurrentChar(1)))
				{
					AddCurrentCharToBuffer();
					Advance();
					// Fill exponent of float
					while (CurrentLocationValid() && std::isdigit(GetCurrentChar()))
					{
						AddCurrentCharToBuffer();
						Advance();
					}

					if (GetCurrentChar() == 'f')
					{
						AddCurrentCharToBuffer();
						Advance();
					}

					AddTokenAndClearBuffer(ScriptTokenType::FloatLiteral, m_TextBuffer);
					continue;
				}
				// Fill in integer literal
				else
				{
					AddTokenAndClearBuffer(ScriptTokenType::IntegerLiteral, m_TextBuffer);
					continue;
				}
			}

			if (std::isspace(GetCurrentChar()))
			{
				Advance();
				continue;
			}

			if (GetCurrentChar() == '\"')
			{
				// Skip first quotation
				AddCurrentCharToBuffer();
				Advance();

				// Fill buffer
				while (CurrentLocationValid() && GetCurrentChar() != '\"')
				{
					AddCurrentCharToBuffer();
					Advance();
				}

				if (!CurrentLocationValid())
				{
					continue;
				}

				// Move past second quotation
				AddCurrentCharToBuffer();
				Advance();

				// Fill in String literal
				AddTokenAndClearBuffer(ScriptTokenType::StringLiteral, m_TextBuffer);
				continue;
			}

			if (GetCurrentChar() == ';')
			{
				AddTokenAndClearBuffer(ScriptTokenType::Semicolon, { ';' });
				Advance();
				continue;
			}

			if (GetCurrentChar() == '(')
			{
				AddTokenAndClearBuffer(ScriptTokenType::OpenParentheses, { '(' });
				Advance();
				continue;
			}

			if (GetCurrentChar() == ')')
			{
				AddTokenAndClearBuffer(ScriptTokenType::CloseParentheses, { ')' });
				Advance();
				continue;
			}

			if (GetCurrentChar() == '{')
			{
				AddTokenAndClearBuffer(ScriptTokenType::OpenCurlyBrace, { '{' });
				Advance();
				continue;
			}

			if (GetCurrentChar() == '}')
			{
				AddTokenAndClearBuffer(ScriptTokenType::CloseCurlyBrace, { '}' });
				Advance();
				continue;
			}

			if (GetCurrentChar() == '=')
			{
				if (!CurrentLocationValid(1))
				{
					AddTokenAndClearBuffer(ScriptTokenType::AssignmentOperator, { '=' });
					Advance();
					continue;
				}

				if (GetCurrentChar(1) == '=')
				{
					AddTokenAndClearBuffer(ScriptTokenType::EqualToOperator, { "==" });
					Advance(2);
					continue;
				}

				AddTokenAndClearBuffer(ScriptTokenType::AssignmentOperator, { '=' });
				Advance();
				continue;
			}

			if (GetCurrentChar() == '>')
			{
				if (!CurrentLocationValid(1))
				{
					AddTokenAndClearBuffer(ScriptTokenType::GreaterThan, { '>' });
					Advance();
					continue;
				}

				if (GetCurrentChar(1) == '=')
				{
					AddTokenAndClearBuffer(ScriptTokenType::GreaterThanOrEqual, { ">=" });
					Advance(2);
					continue;
				}

				AddTokenAndClearBuffer(ScriptTokenType::GreaterThan, { '>' });
				Advance();
				continue;
			}

			if (GetCurrentChar() == '<')
			{
				if (!CurrentLocationValid(1))
				{
					AddTokenAndClearBuffer(ScriptTokenType::LessThan, { '<' });
					Advance();
					continue;
				}

				if (GetCurrentChar(1) == '=')
				{
					AddTokenAndClearBuffer(ScriptTokenType::LessThanOrEqual, { "<=" });
					Advance(2);
					continue;
				}

				AddTokenAndClearBuffer(ScriptTokenType::LessThan, { '<' });
				Advance();
				continue;
			}

			if (GetCurrentChar() == '!')
			{
				if (!CurrentLocationValid(1))
				{
					KG_WARN("Invalid token provided to kargono script compiler");
					Advance();
					continue;
				}

				if (GetCurrentChar(1) == '=')
				{
					AddTokenAndClearBuffer(ScriptTokenType::NotEqualToOperator, { "!=" });
					Advance(2);
					continue;
				}

				KG_WARN("Invalid token provided to kargono script compiler");
				Advance();
				continue;
			}

			if (GetCurrentChar() == '+')
			{
				AddTokenAndClearBuffer(ScriptTokenType::AdditionOperator, { '+' });
				Advance();
				continue;
			}

			if (GetCurrentChar() == '-')
			{
				AddTokenAndClearBuffer(ScriptTokenType::SubtractionOperator, { '-' });
				Advance();
				continue;
			}

			if (GetCurrentChar() == '/')
			{
				// Check for single line comment
				if (GetCurrentChar(1) == '/')
				{
					Advance(2);
					m_TextBuffer.clear();
					while (CurrentLocationValid() && GetCurrentChar() != '\n')
					{
						Advance();
					}
				}
				// Check for multi-line comment
				else if (GetCurrentChar(1) == '*')
				{
					Advance(2);
					m_TextBuffer.clear();
					while (CurrentLocationValid(1) && !(GetCurrentChar() == '*' && GetCurrentChar(1) == '/'))
					{
						Advance();
					}
					// Move past multi-line comment ending if valid
					if (CurrentLocationValid(1))
					{
						Advance(2);
					}
					else
					{
						// Clear remaining character before end of file, since it is still technically inside the comment
						if (CurrentLocationValid())
						{
							Advance();
						}
					}

				}
				// Store division operator
				else
				{
					AddTokenAndClearBuffer(ScriptTokenType::DivisionOperator, { '/' });
					Advance();
				}

				continue;
			}

			if (GetCurrentChar() == '*')
			{
				AddTokenAndClearBuffer(ScriptTokenType::MultiplicationOperator, { '*' });
				Advance();
				continue;
			}

			if (GetCurrentChar() == ',')
			{
				AddTokenAndClearBuffer(ScriptTokenType::Comma, { ',' });
				Advance();
				continue;
			}

			if (GetCurrentChar() == ':' && GetCurrentChar(1) == ':')
			{
				AddTokenAndClearBuffer(ScriptTokenType::NamespaceResolver, { "::" });
				Advance(2);
				continue;
			}

			KG_CRITICAL("Could not identify token!");
			Advance();
		}
		return m_Tokens;
	}

	char ScriptTokenizer::GetCurrentChar(int32_t offset)
	{
		if (m_TextLocation + offset >= m_ScriptText.size())
		{
			return '\0';
		}
		return m_ScriptText.at(m_TextLocation + offset);
	}

	bool ScriptTokenizer::CurrentLocationValid(int32_t offset)
	{
		return m_TextLocation + offset < m_ScriptText.size();
	}


	void ScriptTokenizer::AddCurrentCharToBuffer()
	{
		m_TextBuffer.push_back(m_ScriptText.at(m_TextLocation));
	}

	void ScriptTokenizer::Advance(uint32_t count)
	{
		for (uint32_t iteration{ 0 }; iteration < count; iteration++)
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
		ScriptToken newToken{ type, value, m_LineCount, m_ColumnCount - (uint32_t)m_TextBuffer.size() };
		m_Tokens.push_back(newToken);
		m_TextBuffer.clear();
	}
}
