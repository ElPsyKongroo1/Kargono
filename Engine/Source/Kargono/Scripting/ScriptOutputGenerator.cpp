#include "kgpch.h"

#include "Kargono/Scripting/ScriptOutputGenerator.h"
#include "Kargono/Scripting/ScriptCompilerService.h"

namespace Kargono::Scripting
{
	std::tuple<bool, std::string> ScriptOutputGenerator::GenerateOutput(ScriptAST&& ast)
	{
		m_AST = std::move(ast);
		m_OutputText = {};

		// Get Program Node
		if (!m_AST.ProgramNode)
		{
			return { false, {} };
		}

		FunctionNode& funcNode = m_AST.ProgramNode.FuncNode;
		// Emit Function Signature
		m_OutputText << funcNode.ReturnType.Value << " " << funcNode.Name.Value << '(';
		uint32_t iteration{ 0 };
		for (auto& [allTypes, identifier] : funcNode.Parameters)
		{
			PrimitiveType primitiveType = ScriptCompilerService::s_ActiveLanguageDefinition.GetPrimitiveTypeFromName(allTypes.at(0).Value);
			if (primitiveType.Name == "")
			{
				return { false, {} };
			}
			m_OutputText << primitiveType.EmittedParameter << ' ' << identifier.Value;
			if (iteration + 1 < funcNode.Parameters.size())
			{
				m_OutputText << ", ";
			}
			iteration++;
		}
		m_OutputText << ")\n";
		m_OutputText << "{\n";
		for (auto statement : funcNode.Statements)
		{
			GenerateStatement(statement);
		}
		m_OutputText << "}\n";

		return { true, m_OutputText.str() };
	}

	void ScriptOutputGenerator::GenerateStatement(Ref<Statement> statement)
	{
		std::visit([&](auto&& state)
			{
				using type = std::decay_t<decltype(state)>;
				if constexpr (std::is_same_v<type, StatementEmpty>)
				{
					// Do not display anything for an empty statement
				}
				else if constexpr (std::is_same_v<type, StatementExpression>)
				{
					m_OutputText << "  ";
					GenerateExpression(state.Value);
					m_OutputText << ";\n";
				}
				else if constexpr (std::is_same_v<type, StatementDeclaration>)
				{
					m_OutputText << "  ";
					PrimitiveType typeValue = ScriptCompilerService::s_ActiveLanguageDefinition.GetPrimitiveTypeFromName(state.Type.Value);
					if (typeValue.Name == "")
					{
						return;
					}
					m_OutputText << typeValue.EmittedDeclaration << " " << state.Name.Value << ";\n";

				}
				else if constexpr (std::is_same_v<type, StatementAssignment>)
				{
					m_OutputText << "  ";
					m_OutputText << state.Name.Value << " = ";
					GenerateExpression(state.Value);
					m_OutputText << ";\n";

				}
				else if constexpr (std::is_same_v<type, StatementDeclarationAssignment>)
				{
					m_OutputText << "  ";
					PrimitiveType typeValue = ScriptCompilerService::s_ActiveLanguageDefinition.GetPrimitiveTypeFromName(state.Type.Value);
					if (typeValue.Name == "")
					{
						return;
					}
					m_OutputText << typeValue.EmittedDeclaration << " " << state.Name.Value << " = ";
					GenerateExpression(state.Value);
					m_OutputText << ";\n";
				}

				else if constexpr (std::is_same_v<type, StatementConditional>)
				{
					m_OutputText << "  ";
					switch (state.Type)
					{
					case ConditionalType::IF:
						m_OutputText << "if (";
						break;
					case ConditionalType::ELSEIF:
						m_OutputText << "else if (";
						break;
					case ConditionalType::ELSE:
						m_OutputText << "else\n";
						break;
					case ConditionalType::None:
					default:
						KG_WARN("Invalid conditional type provided to ScriptOutputGenerator");
						return;
					}

					if (state.Type == ConditionalType::IF || state.Type == ConditionalType::ELSEIF)
					{
						GenerateExpression(state.ConditionExpression);
						m_OutputText << ")\n";
					}

					m_OutputText << "{\n";

					for (auto& statement : state.BodyStatements)
					{
						GenerateStatement(statement);
					}

					m_OutputText << "}\n";

					for (auto& statement : state.ChainedConditionals)
					{
						GenerateStatement(statement);
					}

				}
			}, statement->Value);
	}

	void ScriptOutputGenerator::GenerateExpression(Ref<Expression> expression)
	{
		bool success{ true };
		if (expression->GenerationAffixes)
		{
			m_OutputText << expression->GenerationAffixes->Prefix;
		}
		std::visit([&](auto&& expressionValue)
			{
				using type = std::decay_t<decltype(expressionValue)>;
				if constexpr (std::is_same_v<type, ScriptToken>)
				{
					m_OutputText << expressionValue.Value;
				}
				else if constexpr (std::is_same_v<type, FunctionCallNode>)
				{

					if (!ScriptCompilerService::s_ActiveLanguageDefinition.FunctionDefinitions.contains(expressionValue.Identifier.Value))
					{
						KG_WARN("Invalid function definition name provided when generating function call C++ code");
						return;
					}
					std::function<void(FunctionCallNode&)> onGenerateFunc =
						ScriptCompilerService::s_ActiveLanguageDefinition.FunctionDefinitions.at(expressionValue.Identifier.Value).OnGenerateFunction;

					if (onGenerateFunc)
					{
						onGenerateFunc(expressionValue);
					}

					if (expressionValue.Namespace)
					{
						m_OutputText << expressionValue.Namespace.Value << "::";
					}
					m_OutputText << expressionValue.Identifier.Value << '(';
					uint32_t iteration{ 0 };
					for (auto argument : expressionValue.Arguments)
					{
						GenerateExpression(argument);
						if (iteration + 1 < expressionValue.Arguments.size())
						{
							m_OutputText << ", ";
						}
						iteration++;
					}
					m_OutputText << ')';

				}
				else if constexpr (std::is_same_v<type, InitializationListNode>)
				{

					m_OutputText << '{';
					uint32_t iteration{ 0 };
					for (auto argument : expressionValue.Arguments)
					{
						GenerateExpression(argument);
						if (iteration + 1 < expressionValue.Arguments.size())
						{
							m_OutputText << ", ";
						}
						iteration++;
					}
					m_OutputText << '}';

				}
				else if constexpr (std::is_same_v<type, UnaryOperationNode>)
				{
					m_OutputText << expressionValue.Operator.Value << expressionValue.Operand.Value;
				}
				else if constexpr (std::is_same_v<type, BinaryOperationNode>)
				{
					GenerateExpression(expressionValue.LeftOperand);
					m_OutputText << " " << expressionValue.Operator.Value << " ";
					GenerateExpression(expressionValue.RightOperand);
				}
			}, expression->Value);

		if (expression->GenerationAffixes)
		{
			m_OutputText << expression->GenerationAffixes->Postfix;
		}

		if (!success)
		{
			return;
		}
	}
	std::string ScriptOutputGenerator::GetIndentation()
	{
		std::string outputIndentation{};
		for (uint32_t iteration{ 0 }; iteration < m_IndentLevel; iteration++)
		{
			outputIndentation += "  ";
		}
		return outputIndentation;
	}
}
