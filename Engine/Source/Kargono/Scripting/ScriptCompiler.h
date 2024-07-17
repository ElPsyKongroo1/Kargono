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
		IntLiteral,
		SemiColon,

	};

	struct ScriptToken
	{
		ScriptTokenType TokenType{ ScriptTokenType::None };
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
