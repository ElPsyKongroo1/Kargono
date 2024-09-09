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
	private:
		std::stringstream m_OutputText{};
		ScriptAST m_AST{};
	};
}
