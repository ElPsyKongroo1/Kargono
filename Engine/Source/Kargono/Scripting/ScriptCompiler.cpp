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

		// Compile Script

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

		uint32_t textLocation = 0;
		while (textLocation < text.size())
		{
			if (std::isalpha(text.at(textLocation)))
			{
				// Add first character to buffer
				tokenBuffer.push_back(text.at(textLocation));
				textLocation++;

				// Fill remainder of buffer
				while (std::isalnum(text.at(textLocation)))
				{
					tokenBuffer.push_back(text.at(textLocation));
					textLocation++;
					if (textLocation >= text.size())
					{
						break;
					}
				}

				// Check what type of token is provided
				if (tokenBuffer == "return")
				{
					newTokens.push_back({ ScriptTokenType::Return, {} });
					tokenBuffer.clear();
					continue;
				}
				KG_WARN("Invalid token provided");
			}

			if (std::isdigit(text.at(textLocation)))
			{
				// Add first digit to buffer
				tokenBuffer.push_back(text.at(textLocation));
				textLocation++;

				// Fill remainder of buffer
				while (std::isdigit(text.at(textLocation)))
				{
					tokenBuffer.push_back(text.at(textLocation));
					textLocation++;
					if (textLocation >= text.size())
					{
						break;
					}
				}

				// Fill in integer literal
				newTokens.push_back({ ScriptTokenType::IntLiteral, tokenBuffer });
				tokenBuffer.clear();
				continue;
			}

			if (std::isspace(text.at(textLocation)))
			{
				textLocation++;
				continue;
			}

			if (text.at(textLocation) == ';')
			{
				newTokens.push_back({ ScriptTokenType::SemiColon, {} });
				textLocation++;
				continue;
			}

			KG_ERROR("Could not identify character!");
		}
		return newTokens;
	}
}
