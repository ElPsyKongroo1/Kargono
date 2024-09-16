#pragma once
#include "Kargono/Scripting/ScriptCompilerCommon.h"

namespace Kargono::Scripting
{
	class ScriptOutputGenerator
	{
	public:
		std::tuple<bool, std::string> GenerateOutput(ScriptAST&& ast);
		void GenerateStatement(Ref<Statement> expression);
		void GenerateExpression(Ref<Expression> expression);
		void AddIndentation();
	public:
		std::stringstream m_OutputText{};
	private:
		ScriptAST m_AST{};
		uint32_t m_IndentLevel{ 1 };
	};
}
