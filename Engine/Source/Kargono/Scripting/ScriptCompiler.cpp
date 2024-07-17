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
		return scriptFile;
	}
	std::vector<ScriptToken> ScriptCompiler::ConvertTextToTokens(const std::string& text)
	{
		uint32_t iteration = 0;
		for (auto character : text)
		{

			KG_TRACE_CRITICAL(character);
			iteration++;
		}
		return {};
	}
}
