#include "kgpch.h"

#include "Modules/EditorUI/Widgets/EditorUICheckbox.h"
#include "Modules/EditorUI/EditorUIContext.h"
#include "EditorUIVariableWidget.h"

namespace Kargono::EditorUI
{
	void EditVariableWidget::RenderVariable()
	{
		ResetChildID();

		static ImGuiInputTextFlags inputFlags{};

		ImGui::PushStyleColor(ImGuiCol_Text, EditorUIContext::m_ConfigColors.m_PrimaryTextColor);
		int32_t labelPosition = ImGui::FindPositionAfterLength(m_Label.CString(), EditorUIContext::m_ActiveWindowData.m_PrimaryTextWidth);
		EditorUIContext::RenderTruncatedText(m_Label.CString(), labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);
		ImGui::PopStyleColor();

		ImGui::PushStyleColor(ImGuiCol_Text, EditorUIContext::m_ConfigColors.m_SecondaryTextColor);
		switch (VariableType)
		{
		case WrappedVarType::None:
		case WrappedVarType::Void:
		{
			KG_WARN("Invalid Type used for Edit Variable");
			return;
		}

		case WrappedVarType::Bool:
		{
			ImGui::SameLine(EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosOne);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, k_PureEmpty);
			ImGui::PushStyleColor(ImGuiCol_Button, k_PureEmpty);
			EditorUIContext::RenderTruncatedText("True", 12);
			ImGui::SameLine();
			EditorUIContext::RenderInlineButton(GetNextChildID(), [&]()
				{
					if (FieldBuffer.As<char>() == "True")
					{
						FieldBuffer.SetString("False");
					}
					else
					{
						FieldBuffer.SetString("True");
					}
				}, EditorUIContext::m_UIPresets.m_SmallCheckboxButton, std::string(FieldBuffer.As<char>()) == std::string("True"), EditorUIContext::m_ConfigColors.m_HighlightColor1);

			ImGui::SameLine(300.0f);
			EditorUIContext::RenderTruncatedText("False", 12);
			ImGui::SameLine();
			EditorUIContext::RenderInlineButton(GetNextChildID(), [&]()
				{
					if (FieldBuffer.As<char>() == "False")
					{
						FieldBuffer.SetString("True");
					}
					else
					{
						FieldBuffer.SetString("False");
					}
				}, EditorUIContext::m_UIPresets.m_SmallCheckboxButton, std::string(FieldBuffer.As<char>()) == std::string("False"), EditorUIContext::m_ConfigColors.m_HighlightColor1);
			ImGui::PopStyleColor(2);
			break;
		}
		case WrappedVarType::String:
		{
			inputFlags = ImGuiInputTextFlags_CallbackEdit;
			ImGui::SameLine();
			ImGui::SetCursorPosX(EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosOne);
			ImGui::SetNextItemWidth(170.0f);
			ImGuiInputTextCallback typeCallback = [](ImGuiInputTextCallbackData* data)
				{
					UNREFERENCED_PARAMETER(data);
					return 0;
				};
			ImGui::InputText(("##" + std::to_string(GetNextChildID())).c_str(),
				FieldBuffer.As<char>(), FieldBuffer.Size, inputFlags, typeCallback);
			break;
		}
		case WrappedVarType::Float:
		case WrappedVarType::Integer32:
		case WrappedVarType::UInteger16:
		case WrappedVarType::UInteger32:
		case WrappedVarType::UInteger64:
		default:
		{
			inputFlags = ImGuiInputTextFlags_CallbackEdit | ImGuiInputTextFlags_CharsDecimal;
			ImGui::SameLine();
			ImGui::SetCursorPosX(EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosOne);
			ImGui::SetNextItemWidth(170.0f);
			ImGuiInputTextCallback typeCallback = [](ImGuiInputTextCallbackData* data)
				{
					UNREFERENCED_PARAMETER(data);
					return 0;
				};
			ImGui::InputText(("##" + std::to_string(GetNextChildID())).c_str(),
				FieldBuffer.As<char>(), FieldBuffer.Size, inputFlags, typeCallback);
			break;
		}
		}
		ImGui::PopStyleColor();

	}
}

