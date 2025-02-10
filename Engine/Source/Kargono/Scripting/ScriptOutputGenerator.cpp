#include "kgpch.h"

#include "Kargono/Scripting/ScriptOutputGenerator.h"
#include "Kargono/Scripting/ScriptCompilerService.h"

#include "Kargono/Core/Resolution.h"
#include "Kargono/Core/KeyCodes.h"
#include "Kargono/Utility/FileSystem.h"

namespace Kargono::Scripting
{
	std::tuple<bool, std::string> ScriptOutputGenerator::GenerateOutput(ScriptAST&& ast)
	{
		m_AST = std::move(ast);
		m_OutputText = {};

		// Get Program Node
		if (!m_AST.m_ProgramNode)
		{
			return { false, {} };
		}

		FunctionNode& funcNode = m_AST.m_ProgramNode.FuncNode;
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
		if (StatementEmpty* emptyStatement = std::get_if<StatementEmpty>(&statement->Value))
		{
			// Do not display anything for an empty statement
		}
		else if (StatementExpression* expressionStatement = std::get_if<StatementExpression>(&statement->Value))
		{
			AddIndentation();
			GenerateExpression(expressionStatement->Value);
			m_OutputText << ";\n";
		}
		else if (StatementReturn* returnStatement = std::get_if<StatementReturn>(&statement->Value))
		{
			AddIndentation();

			if (returnStatement->ReturnValue)
			{
				m_OutputText << "return ";
				GenerateExpression(returnStatement->ReturnValue);
				m_OutputText << ";\n";
			}
			else
			{
				
				m_OutputText << "return;\n";
			}
			
		}
		else if (StatementDeclaration* declarationStatement = std::get_if<StatementDeclaration>(&statement->Value))
		{
			AddIndentation();
			PrimitiveType typeValue = ScriptCompilerService::s_ActiveLanguageDefinition.GetPrimitiveTypeFromName(declarationStatement->Type.Value);
			if (typeValue.Name == "")
			{
				return;
			}
			m_OutputText << typeValue.EmittedDeclaration << " " << declarationStatement->Name.Value << ";\n";

		}
		else if (StatementAssignment* assignmentStatement = std::get_if<StatementAssignment>(&statement->Value))
		{
			// Check if an OnGenerateSetter override is appropriate
			if (MemberNode* memberNode = std::get_if<MemberNode>(&assignmentStatement->Name->Value))
			{
				// Get terminal node of MemberNode list
				Ref<MemberNode> terminalNode = memberNode->ChildMemberNode;
				{
					while (terminalNode->ChildMemberNode)
					{
						terminalNode = terminalNode->ChildMemberNode;
					}
				}

				// Check if terminal node uses an OnGenerateSetter override
				if (DataMember* currentDataMember = std::get_if<DataMember>(&terminalNode->m_MemberType->Value))
				{
					if (currentDataMember->OnGenerateSetter)
					{
						AddIndentation();
						currentDataMember->OnGenerateSetter(*this, *assignmentStatement);
						m_OutputText << ";\n";
						return;
					}
				}
			}

			// If not, simply output the assignment statement normally
			AddIndentation();
			GenerateExpression(assignmentStatement->Name);
			m_OutputText << " = ";
			GenerateExpression(assignmentStatement->Value);
			m_OutputText << ";\n";

		}
		else if (StatementDeclarationAssignment* declarationAssignmentStatement = std::get_if<StatementDeclarationAssignment>(&statement->Value))
		{
			AddIndentation();
			PrimitiveType typeValue = ScriptCompilerService::s_ActiveLanguageDefinition.GetPrimitiveTypeFromName(declarationAssignmentStatement->Type.Value);
			if (typeValue.Name == "")
			{
				return;
			}
			m_OutputText << typeValue.EmittedDeclaration << " " << declarationAssignmentStatement->Name.Value << " = ";
			GenerateExpression(declarationAssignmentStatement->Value);
			m_OutputText << ";\n";
		}

		else if (StatementConditional* conditionalStatement = std::get_if<StatementConditional>(&statement->Value))
		{
			AddIndentation();
			switch (conditionalStatement->Type)
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

			if (conditionalStatement->Type == ConditionalType::IF || conditionalStatement->Type == ConditionalType::ELSEIF)
			{
				GenerateExpression(conditionalStatement->ConditionExpression);
				m_OutputText << ")\n";
			}
			AddIndentation();
			m_OutputText << "{\n";

			m_IndentLevel++;
			for (Ref<Statement> bodyStatement : conditionalStatement->BodyStatements)
			{
				GenerateStatement(bodyStatement);
			}
			m_IndentLevel--;
			AddIndentation();
			m_OutputText << "}\n";

			for (Ref<Statement> chainedConditional : conditionalStatement->ChainedConditionals)
			{
				GenerateStatement(chainedConditional);
			}

		}

		else if (StatementWhileLoop* whileLoopStatement = std::get_if<StatementWhileLoop>(&statement->Value))
		{
			AddIndentation();

			m_OutputText << "while (";
			GenerateExpression(whileLoopStatement->ConditionExpression);
			m_OutputText << ")\n";
			
			AddIndentation();
			m_OutputText << "{\n";

			m_IndentLevel++;
			for (Ref<Statement> bodyStatement : whileLoopStatement->BodyStatements)
			{
				GenerateStatement(bodyStatement);
			}
			m_IndentLevel--;
			AddIndentation();
			m_OutputText << "}\n";
		}

		else if (StatementBreak* breakStatement = std::get_if<StatementBreak>(&statement->Value))
		{
			AddIndentation();
			m_OutputText << "break;";
		}

		else if (StatementContinue* continueStatement = std::get_if<StatementContinue>(&statement->Value))
		{
			AddIndentation();
			m_OutputText << "continue;";
		}

	}

	void ScriptOutputGenerator::GenerateExpression(Ref<Expression> expression)
	{
		if (expression->GenerationAffixes)
		{
			m_OutputText << expression->GenerationAffixes->Prefix;
		}

		if (TokenExpressionNode* token = std::get_if<TokenExpressionNode>(&expression->Value))
		{
			KG_ASSERT(token->Value.Type != ScriptTokenType::CustomLiteral);
			m_OutputText << token->Value.Value;
			
		}
		else if (FunctionCallNode* funcNode = std::get_if<FunctionCallNode>(&expression->Value))
		{
			if (!ScriptCompilerService::s_ActiveLanguageDefinition.FunctionDefinitions.contains(funcNode->Identifier.Value))
			{
				KG_WARN("Invalid function definition name provided when generating function call C++ code");
				return;
			}
			std::function<void(ScriptOutputGenerator& generator, FunctionCallNode&)> onGenerateFunc =
				ScriptCompilerService::s_ActiveLanguageDefinition.FunctionDefinitions.at(funcNode->Identifier.Value).OnGenerateFunction;

			if (onGenerateFunc)
			{
				onGenerateFunc(*this, *funcNode);
			}

			if (funcNode->Namespace)
			{
				m_OutputText << funcNode->Namespace.Value << "::";
			}
			m_OutputText << funcNode->Identifier.Value << '(';
			uint32_t iteration{ 0 };
			for (auto& argument : funcNode->Arguments)
			{
				GenerateExpression(argument);
				if (iteration + 1 < funcNode->Arguments.size())
				{
					m_OutputText << ", ";
				}
				iteration++;
			}
			m_OutputText << ')';
		}

		else if (CustomLiteralNode* customLiteralNode = std::get_if<CustomLiteralNode>(&expression->Value))
		{
			// Output ID of the customLiteral
			m_OutputText << customLiteralNode->OutputValue;
		}

		else if (InitializationListNode* initListNode = std::get_if<InitializationListNode>(&expression->Value))
		{
			m_OutputText << '{';
			uint32_t iteration{ 0 };
			for (Ref<Expression> argument : initListNode->Arguments)
			{
				GenerateExpression(argument);
				if (iteration + 1 < initListNode->Arguments.size())
				{
					m_OutputText << ", ";
				}
				iteration++;
			}
			m_OutputText << '}';
		}
		else if (UnaryOperationNode* unaryOperationNode = std::get_if<UnaryOperationNode>(&expression->Value))
		{
			m_OutputText << unaryOperationNode->Operator.Value << unaryOperationNode->Operand.Value;
		}
		else if (BinaryOperationNode* binaryOperationNode = std::get_if<BinaryOperationNode>(&expression->Value))
		{
			GenerateExpression(binaryOperationNode->LeftOperand);
			m_OutputText << " " << binaryOperationNode->Operator.Value << " ";
			GenerateExpression(binaryOperationNode->RightOperand);
		}
		else if (TernaryOperationNode* ternaryOperationNode = std::get_if<TernaryOperationNode>(&expression->Value))
		{
			GenerateExpression(ternaryOperationNode->Conditional);
			m_OutputText << " ? ";
			GenerateExpression(ternaryOperationNode->OptionOne);
			m_OutputText << " : ";
			GenerateExpression(ternaryOperationNode->OptionTwo);
		}
		else if (MemberNode* memberNode = std::get_if<MemberNode>(&expression->Value))
		{
			// Find terminal node
			Ref<MemberNode> currentNode = memberNode->ChildMemberNode;
			{
				while (currentNode->ChildMemberNode)
				{
					currentNode = currentNode->ChildMemberNode;
				}
			}

			// Check if member node uses an OnGenerateGetter function
			bool useOnGenerate = false;
			if (FunctionCallNode* funcCallNode = std::get_if<FunctionCallNode>(&currentNode->CurrentNodeExpression->Value))
			{
				if (funcCallNode->m_FunctionNode && funcCallNode->m_FunctionNode->OnGenerateGetter)
				{
					funcCallNode->m_FunctionNode->OnGenerateGetter(*this, *memberNode);
					useOnGenerate = true;
				}
			}
			else if (TokenExpressionNode* tokenNode = std::get_if<TokenExpressionNode>(&currentNode->CurrentNodeExpression->Value))
			{
				std::variant<FunctionNode, DataMember> Value {};
				DataMember* currentDataMember = std::get_if<DataMember>(&currentNode->m_MemberType->Value);
				KG_ASSERT(currentDataMember);

				if (tokenNode &&  currentDataMember->OnGenerateGetter)
				{
					currentDataMember->OnGenerateGetter(*this, *memberNode);
					useOnGenerate = true;
				}
			}

			// If on generate is not available, then generate the output as it appears (Ex: entity.Transform.GetTranslation())
			if (!useOnGenerate)
			{
				// Generate text for member node
				GenerateExpression(memberNode->CurrentNodeExpression);
				currentNode = memberNode->ChildMemberNode;
				while (currentNode)
				{
					// Generate expression code for each MemberNode in linkedlist
					m_OutputText << '.';
					if (TokenExpressionNode* expressionToken = std::get_if<TokenExpressionNode>(&currentNode->CurrentNodeExpression->Value))
					{
						GenerateExpression(currentNode->CurrentNodeExpression);
					}
					else if (FunctionCallNode* expressionFunctionCall = std::get_if<FunctionCallNode>(&currentNode->CurrentNodeExpression->Value))
					{
						m_OutputText << expressionFunctionCall->Identifier.Value << '(';
						uint32_t iteration{ 0 };
						for (auto argument : expressionFunctionCall->Arguments)
						{
							GenerateExpression(argument);
							if (iteration + 1 < expressionFunctionCall->Arguments.size())
							{
								m_OutputText << ", ";
							}
							iteration++;
						}
						m_OutputText << ')';
					}
					else
					{
						KG_WARN("Invalid current expression inside MemberNode when generating output");
					}
					currentNode = currentNode->ChildMemberNode;
				}
			}

			
		}

		if (expression->GenerationAffixes)
		{
			m_OutputText << expression->GenerationAffixes->Postfix;
		}
	}
	void ScriptOutputGenerator::AddIndentation()
	{
		std::string outputIndentation{};
		for (uint32_t iteration{ 0 }; iteration < m_IndentLevel; iteration++)
		{
			m_OutputText << "  ";
		}
	}
}
