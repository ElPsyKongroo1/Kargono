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

		// Get Tokens from Text
		std::vector<ScriptToken> allTokens = ConvertTextToTokens(scriptFile);

		//TODO: Testing
		std::stringstream outputStream {};
		for (auto& token : allTokens)
		{
			outputStream << "TokenType: " << Utility::ScriptTokenTypeToString(token.Type) << " | Value: " << token.Value << "\n";
		}

		return outputStream.str();
	}
	std::vector<ScriptToken> ScriptCompiler::ConvertTextToTokens(const std::string& text)
	{
		std::vector<ScriptToken> newTokens {};
		std::string tokenBuffer {};
		uint32_t textLocation {0};

		while (textLocation < text.size())
		{
			if (std::isalpha(text.at(textLocation)))
			{
				// Add first character to buffer
				tokenBuffer.push_back(text.at(textLocation));
				textLocation++;

				// Fill remainder of buffer
				while (textLocation < text.size() && std::isalnum(text.at(textLocation)))
				{
					tokenBuffer.push_back(text.at(textLocation));
					textLocation++;
				}

				// Check what type of token is provided
				if (tokenBuffer == "return")
				{
					newTokens.push_back({ ScriptTokenType::Return, {} });
					tokenBuffer.clear();
					continue;
				}
				if (tokenBuffer == "void")
				{
					newTokens.push_back({ ScriptTokenType::Void, {} });
					tokenBuffer.clear();
					continue;
				}
				if (tokenBuffer == "UInt16")
				{
					newTokens.push_back({ ScriptTokenType::UInt16, {} });
					tokenBuffer.clear();
					continue;
				}

				if (tokenBuffer == "String")
				{
					newTokens.push_back({ ScriptTokenType::String, {} });
					tokenBuffer.clear();
					continue;
				}

				newTokens.push_back({ ScriptTokenType::Identifier, tokenBuffer });
				tokenBuffer.clear();
				continue;
			}

			if (std::isdigit(text.at(textLocation)))
			{
				// Add first digit to buffer
				tokenBuffer.push_back(text.at(textLocation));
				textLocation++;

				// Fill remainder of buffer
				while (textLocation < text.size() && std::isdigit(text.at(textLocation)))
				{
					tokenBuffer.push_back(text.at(textLocation));
					textLocation++;
				}

				// Fill in integer literal
				newTokens.push_back({ ScriptTokenType::IntegerLiteral, tokenBuffer });
				tokenBuffer.clear();
				continue;
			}

			if (std::isspace(text.at(textLocation)))
			{
				textLocation++;
				continue;
			}

			if (text.at(textLocation) == '\"')
			{
				// Skip first quotation
				textLocation++;

				// Fill buffer
				while (textLocation < text.size() && text.at(textLocation) != '\"')
				{
					tokenBuffer.push_back(text.at(textLocation));
					textLocation++;
				}

				// Move past second quotation
				textLocation++;

				// Fill in String literal
				newTokens.push_back({ ScriptTokenType::StringLiteral, tokenBuffer });
				tokenBuffer.clear();
				continue;
			}

			if (text.at(textLocation) == ';')
			{
				newTokens.push_back({ ScriptTokenType::Semicolon, {} });
				textLocation++;
				continue;
			}

			if (text.at(textLocation) == '(')
			{
				newTokens.push_back({ ScriptTokenType::OpenParentheses, {} });
				textLocation++;
				continue;
			}

			if (text.at(textLocation) == ')')
			{
				newTokens.push_back({ ScriptTokenType::CloseParentheses, {} });
				textLocation++;
				continue;
			}

			if (text.at(textLocation) == '{')
			{
				newTokens.push_back({ ScriptTokenType::OpenCurlyBrace, {} });
				textLocation++;
				continue;
			}

			if (text.at(textLocation) == '}')
			{
				newTokens.push_back({ ScriptTokenType::CloseCurlyBrace, {} });
				textLocation++;
				continue;
			}

			if (text.at(textLocation) == '=')
			{
				newTokens.push_back({ ScriptTokenType::AssignmentOperator, {} });
				textLocation++;
				continue;
			}

			if (text.at(textLocation) == '+')
			{
				newTokens.push_back({ ScriptTokenType::AdditionOperator, {} });
				textLocation++;
				continue;
			}

			if (text.at(textLocation) == ',')
			{
				newTokens.push_back({ ScriptTokenType::Comma, {} });
				textLocation++;
				continue;
			}

			if (text.at(textLocation) == ':' && text.at(textLocation + 1) == ':')
			{
				newTokens.push_back({ ScriptTokenType::NamespaceResolver, {} });
				textLocation+= 2;
				continue;
			}



			KG_ERROR("Could not identify character!");
		}
		return newTokens;
	}
}
