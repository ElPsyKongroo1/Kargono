#include "kgpch.h"

#include "Modules/EditorUI/Widgets/EditorUICheckbox.h"
#include "Modules/EditorUI/EditorUIContext.h"
#include "EditorUIVariableWidget.h"

namespace Kargono::EditorUI
{
	void EditVariableWidget::RenderVariable()
	{
		// Local Variables
		uint32_t widgetCount{ 0 };
		FixedString<16> id{ "##" };
		id.AppendInteger(m_WidgetID);
		static ImGuiInputTextFlags inputFlags{};

		ImGui::PushStyleColor(ImGuiCol_Text, EditorUIService::m_ConfigColors.s_PrimaryTextColor);
		int32_t labelPosition = ImGui::FindPositionAfterLength(m_Label.CString(), EditorUIService::s_PrimaryTextWidth);
		EditorUIService::TruncateText(m_Label.CString(), labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);
		ImGui::PopStyleColor();

		ImGui::PushStyleColor(ImGuiCol_Text, EditorUIService::m_ConfigColors.s_SecondaryTextColor);
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
			ImGui::SameLine(EditorUIService::s_SecondaryTextPosOne);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, k_PureEmpty);
			ImGui::PushStyleColor(ImGuiCol_Button, k_PureEmpty);
			EditorUIService::TruncateText("True", 12);
			ImGui::SameLine();
			EditorUIService::CreateButton(m_WidgetID + EditorUIService::WidgetIterator(widgetCount), [&]()
				{
					if (FieldBuffer.As<char>() == "True")
					{
						FieldBuffer.SetString("False");
					}
					else
					{
						FieldBuffer.SetString("True");
					}
				}, EditorUIService::s_SmallCheckboxButton, std::string(FieldBuffer.As<char>()) == std::string("True"), EditorUIService::m_ConfigColors.s_HighlightColor1);

			ImGui::SameLine(300.0f);
			EditorUIService::TruncateText("False", 12);
			ImGui::SameLine();
			EditorUIService::CreateButton(m_WidgetID + EditorUIService::WidgetIterator(widgetCount), [&]()
				{
					if (FieldBuffer.As<char>() == "False")
					{
						FieldBuffer.SetString("True");
					}
					else
					{
						FieldBuffer.SetString("False");
					}
				}, EditorUIService::s_SmallCheckboxButton, std::string(FieldBuffer.As<char>()) == std::string("False"), EditorUIService::m_ConfigColors.s_HighlightColor1);
			ImGui::PopStyleColor(2);
			break;
		}
		case WrappedVarType::String:
		{
			inputFlags = ImGuiInputTextFlags_CallbackEdit;
			ImGui::SameLine();
			ImGui::SetCursorPosX(EditorUIService::s_SecondaryTextPosOne);
			ImGui::SetNextItemWidth(170.0f);
			ImGuiInputTextCallback typeCallback = [](ImGuiInputTextCallbackData* data)
				{
					UNREFERENCED_PARAMETER(data);
					return 0;
				};
			ImGui::InputText(("##" + std::to_string(m_WidgetID + EditorUIService::WidgetIterator(widgetCount))).c_str(),
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
			ImGui::SetCursorPosX(EditorUIService::s_SecondaryTextPosOne);
			ImGui::SetNextItemWidth(170.0f);
			ImGuiInputTextCallback typeCallback = [](ImGuiInputTextCallbackData* data)
				{
					UNREFERENCED_PARAMETER(data);
					return 0;
				};
			ImGui::InputText(("##" + std::to_string(m_WidgetID + EditorUIService::WidgetIterator(widgetCount))).c_str(),
				FieldBuffer.As<char>(), FieldBuffer.Size, inputFlags, typeCallback);
			break;
		}
		}
		ImGui::PopStyleColor();

	}
}

