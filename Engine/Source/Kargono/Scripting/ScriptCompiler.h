#pragma once
#include <string>
#include <vector>
#include <filesystem>

namespace Kargono::Scripting
{
	enum class ScriptTokenType
	{
		None = 0,
		Return,
		IntegerLiteral,
		Semicolon
	};

	struct ScriptToken
	{
		ScriptTokenType Type{ ScriptTokenType::None };
		std::string Value {};
	};

	//==============================
	// Script Compiler Class
	//==============================
	class ScriptCompiler
	{
	public:
		//==============================
		// External API
		//==============================
		static std::string CompileScriptFile(const std::filesystem::path& scriptLocation);
	private:
		//==============================
		// Internal Functionality
		//==============================
		static std::vector<ScriptToken> ConvertTextToTokens(const std::string& text);
	};
}

namespace Kargono::Utility
{
	inline std::string ScriptTokenTypeToString(Scripting::ScriptTokenType type)
	{
		switch (type)
		{
			case Scripting::ScriptTokenType::Return: return "Return";
			case Scripting::ScriptTokenType::IntegerLiteral: return "Integer Literal";
			case Scripting::ScriptTokenType::Semicolon: return "Semicolon";

			case Scripting::ScriptTokenType::None:
			default:
			{
				KG_CRITICAL("Unknown Type of ScriptTokenType.");
				return {};
				
			}
		}
	}
}
