#pragma once
#include "ScriptingPlugin/ScriptCompilerCommon.h"

namespace Kargono::Scripting
{
	class ScriptTokenizer
	{
	public:
		std::vector<ScriptToken> TokenizeString(std::string m_ScriptText);
	private:
		char GetCurrentChar(int32_t offset = 0);
		bool CurrentLocationValid(int32_t offset = 0);
		void AddCurrentCharToBuffer();
		void Advance(uint32_t count = 1);
		void AddTokenAndClearBuffer(ScriptTokenType type, const std::string& value);
		ScriptToken CreateTokenExplicit(ScriptTokenType type, const std::string& value);
		void AddTokenExplicit(const ScriptToken& token);
		void ClearBuffer();
	private:
		std::string m_ScriptText{};
		std::string m_TextBuffer{};
		std::vector<ScriptToken> m_Tokens {};
		uint32_t m_TextLocation{ 0 };
		uint32_t m_LineCount{ 1 };
		uint32_t m_ColumnCount{ 0 };
	};
}
