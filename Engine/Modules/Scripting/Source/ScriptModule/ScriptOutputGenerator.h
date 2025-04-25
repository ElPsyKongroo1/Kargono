#pragma once
#include "ScriptModule/ScriptCompilerCommon.h"

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
		ScriptAST m_AST{};
	private:
		uint32_t m_IndentLevel{ 1 };
	};
}
