#pragma once
#include "Kargono/Scripting/ScriptCompilerCommon.h"

namespace Kargono::Scripting
{
	class ScriptOutputGenerator
	{
	public:
		std::tuple<bool, std::string> GenerateOutput(ScriptAST&& ast);
	private:
		void GenerateStatement(Ref<Statement> expression);
		void GenerateExpression(Ref<Expression> expression);
		void AddIndentation();
	private:
		std::stringstream m_OutputText{};
		ScriptAST m_AST{};
		uint32_t m_IndentLevel{ 1 };
	};
}
