#include "kgpch.h"

#include "Modules/EditorUI/Widgets/EditorUIFloatWidget.h"
#include "Modules/EditorUI/EditorUIContext.h"

#include "Kargono/Utility/Operations.h"

namespace Kargono::EditorUI
{
	void EditFloatWidget::RenderFloat()
	{
		// Local Variables
		FixedString<16> id{ "##" };
		id.AppendInteger(m_WidgetID);
		uint32_t widgetCount{ 0 };

		// Draw background
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec2 screenPosition = ImGui::GetCursorScreenPos();
		draw_list->AddRectFilled(ImVec2(EditorUIService::s_WindowPosition.x + EditorUIService::s_SecondaryTextPosOne - 5.0f, screenPosition.y),
			ImVec2(EditorUIService::s_WindowPosition.x + EditorUIService::s_SecondaryTextPosOne + EditorUIService::s_SecondaryTextSmallWidth, screenPosition.y + EditorUI::EditorUIService::s_TextBackgroundHeight),
			ImColor(EditorUI::EditorUIService::s_ActiveBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);


		// Display Item
		if (m_Flags & EditFloat_Indented)
		{
			ImGui::SetCursorPosX(EditorUIService::s_TextLeftIndentOffset);
		}


		ImGui::PushStyleColor(ImGuiCol_Text, EditorUIService::m_ConfigColors.s_PrimaryTextColor);
		int32_t labelPosition = ImGui::FindPositionAfterLength(m_Label.CString(),
			m_Flags & EditFloat_Indented ? EditorUIService::s_PrimaryTextIndentedWidth : EditorUIService::s_PrimaryTextWidth);
		EditorUIService::TruncateText(m_Label.CString(), labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);
		ImGui::PopStyleColor();

		ImGui::SameLine(EditorUIService::s_SecondaryTextPosOne);

		if (m_Editing)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
			// x value
			ImGui::PushStyleColor(ImGuiCol_Text, EditorUIService::m_ConfigColors.s_HighlightColor1);
			ImGui::SetNextItemWidth(EditorUIService::s_SecondaryTextSmallWidth);
			if (ImGui::DragFloat(("##" + std::to_string(m_WidgetID + EditorUIService::WidgetIterator(widgetCount))).c_str(), &(m_CurrentFloat), m_ScrollSpeed,
				m_Bounds[0], m_Bounds[1],
				"%.2f", ImGuiSliderFlags_AlwaysClamp))
			{
				if (m_ConfirmAction)
				{
					m_ConfirmAction(*this);
				}
			}
			ImGui::PopStyleColor();
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::TextColored(EditorUIService::m_ConfigColors.s_HighlightColor1, "X-Value");
				ImGui::EndTooltip();
			}
			ImGui::PopStyleVar();

		}
		else
		{
			float yPosition = ImGui::GetCursorPosY();
			ImGui::SetCursorPos({ EditorUIService::s_SecondaryTextPosOne, yPosition });
			ImGui::PushStyleColor(ImGuiCol_Text, EditorUIService::m_ConfigColors.s_SecondaryTextColor);
			int32_t floatPosition = ImGui::FindPositionAfterLength(Utility::Conversions::FloatToString(m_CurrentFloat).c_str(), EditorUIService::s_SecondaryTextSmallWidth);
			EditorUIService::TruncateText(Utility::Conversions::FloatToString(m_CurrentFloat),
				floatPosition == -1 ? std::numeric_limits<int32_t>::max() : floatPosition);
			ImGui::PopStyleColor();
		}

		ImGui::SameLine();
		EditorUIService::CreateButton(m_WidgetID + EditorUIService::WidgetIterator(widgetCount), [&]()
			{
				Utility::Operations::ToggleBoolean(m_Editing);
			},
			EditorUIService::s_SmallEditButton,
			m_Editing, m_Editing ? EditorUIService::m_ConfigColors.s_PrimaryTextColor : EditorUIService::m_ConfigColors.s_DisabledColor);
	}
	void EditVec2Widget::RenderVec2()
	{
		// Local Variables
		FixedString<16> id{ "##" };
		id.AppendInteger(m_WidgetID);
		uint32_t widgetCount{ 0 };

		// Draw backgrounds
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec2 screenPosition = ImGui::GetCursorScreenPos();
		draw_list->AddRectFilled(ImVec2(EditorUIService::s_WindowPosition.x + EditorUIService::s_SecondaryTextPosOne - 5.0f, screenPosition.y),
			ImVec2(EditorUIService::s_WindowPosition.x + EditorUIService::s_SecondaryTextPosOne + EditorUIService::s_SecondaryTextSmallWidth, screenPosition.y + EditorUI::EditorUIService::s_TextBackgroundHeight),
			ImColor(EditorUI::EditorUIService::s_ActiveBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);
		draw_list->AddRectFilled(ImVec2(EditorUIService::s_WindowPosition.x + EditorUIService::s_SecondaryTextPosTwo - 5.0f, screenPosition.y),
			ImVec2(EditorUIService::s_WindowPosition.x + EditorUIService::s_SecondaryTextPosTwo + EditorUIService::s_SecondaryTextSmallWidth, screenPosition.y + EditorUI::EditorUIService::s_TextBackgroundHeight),
			ImColor(EditorUI::EditorUIService::s_ActiveBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);

		// Display Item
		if (m_Flags & EditVec2_Indented)
		{
			ImGui::SetCursorPosX(EditorUIService::s_TextLeftIndentOffset);
		}
		ImGui::PushStyleColor(ImGuiCol_Text, EditorUIService::m_ConfigColors.s_PrimaryTextColor);

		int32_t labelPosition = ImGui::FindPositionAfterLength(m_Label.CString(),
			m_Flags & EditVec2_Indented ? EditorUIService::s_PrimaryTextIndentedWidth : EditorUIService::s_PrimaryTextWidth);
		EditorUIService::TruncateText(m_Label.CString(), labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);

		ImGui::PopStyleColor();
		ImGui::SameLine(EditorUIService::s_SecondaryTextPosOne);

		if (m_Editing)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
			// x value
			ImGui::PushStyleColor(ImGuiCol_Text, EditorUIService::m_ConfigColors.s_HighlightColor1);
			float yPosition = ImGui::GetCursorPosY();
			ImGui::SetNextItemWidth(EditorUIService::s_SecondaryTextSmallWidth);
			if (ImGui::DragFloat(("##" + std::to_string(m_WidgetID + EditorUIService::WidgetIterator(widgetCount))).c_str(), &(m_CurrentVec2.x), m_ScrollSpeed,
				m_Bounds[0], m_Bounds[1],
				"%.2f", ImGuiSliderFlags_AlwaysClamp))
			{
				if (m_ConfirmAction)
				{
					m_ConfirmAction(*this);
				}
			}
			ImGui::PopStyleColor();
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::TextColored(EditorUIService::m_ConfigColors.s_HighlightColor1, "X-Value");
				ImGui::EndTooltip();
			}

			// y value
			ImGui::PushStyleColor(ImGuiCol_Text, EditorUIService::m_ConfigColors.s_HighlightColor2);
			ImGui::SetCursorPos({ EditorUIService::s_SecondaryTextPosTwo, yPosition });
			ImGui::SetNextItemWidth(EditorUIService::s_SecondaryTextSmallWidth);
			if (ImGui::DragFloat(("##" + std::to_string(m_WidgetID + EditorUIService::WidgetIterator(widgetCount))).c_str(), &(m_CurrentVec2.y), m_ScrollSpeed,
				m_Bounds[0], m_Bounds[1],
				"%.2f", ImGuiSliderFlags_AlwaysClamp))
			{
				if (m_ConfirmAction)
				{
					m_ConfirmAction(*this);
				}
			}
			ImGui::PopStyleColor();
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::TextColored(EditorUIService::m_ConfigColors.s_HighlightColor2, "Y-Value");
				ImGui::EndTooltip();
			}
			ImGui::PopStyleVar();

		}
		else
		{
			float yPosition = ImGui::GetCursorPosY();
			ImGui::SetCursorPos({ EditorUIService::s_SecondaryTextPosOne, yPosition });
			ImGui::PushStyleColor(ImGuiCol_Text, EditorUIService::m_ConfigColors.s_SecondaryTextColor);
			int32_t floatPosition = ImGui::FindPositionAfterLength(
				Utility::Conversions::FloatToString(m_CurrentVec2.x).c_str(), EditorUIService::s_SecondaryTextSmallWidth);
			EditorUIService::TruncateText(Utility::Conversions::FloatToString(m_CurrentVec2.x),
				floatPosition == -1 ? std::numeric_limits<int32_t>::max() : floatPosition);
			ImGui::SetCursorPos({ EditorUIService::s_SecondaryTextPosTwo, yPosition });
			floatPosition = ImGui::FindPositionAfterLength(
				Utility::Conversions::FloatToString(m_CurrentVec2.y).c_str(), EditorUIService::s_SecondaryTextSmallWidth);
			EditorUIService::TruncateText(Utility::Conversions::FloatToString(m_CurrentVec2.y),
				floatPosition == -1 ? std::numeric_limits<int32_t>::max() : floatPosition);
			ImGui::PopStyleColor();
		}

		ImGui::SameLine();
		EditorUIService::CreateButton(m_WidgetID + EditorUIService::WidgetIterator(widgetCount), [&]()
			{
				Utility::Operations::ToggleBoolean(m_Editing);
			},
			EditorUIService::s_SmallEditButton,
			m_Editing, m_Editing ? EditorUIService::m_ConfigColors.s_PrimaryTextColor : EditorUIService::m_ConfigColors.s_DisabledColor);
	}
	void EditVec3Widget::RenderVec3()
	{
		// Local Variables
		FixedString<16> id{ "##" };
		id.AppendInteger(m_WidgetID);
		uint32_t widgetCount{ 0 };

		// Draw backgrounds
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec2 screenPosition = ImGui::GetCursorScreenPos();
		draw_list->AddRectFilled(ImVec2(EditorUIService::s_WindowPosition.x + EditorUIService::s_SecondaryTextPosOne - 5.0f, screenPosition.y),
			ImVec2(EditorUIService::s_WindowPosition.x + EditorUIService::s_SecondaryTextPosOne + EditorUIService::s_SecondaryTextSmallWidth, screenPosition.y + EditorUI::EditorUIService::s_TextBackgroundHeight),
			ImColor(EditorUI::EditorUIService::s_ActiveBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);
		draw_list->AddRectFilled(ImVec2(EditorUIService::s_WindowPosition.x + EditorUIService::s_SecondaryTextPosTwo - 5.0f, screenPosition.y),
			ImVec2(EditorUIService::s_WindowPosition.x + EditorUIService::s_SecondaryTextPosTwo + EditorUIService::s_SecondaryTextSmallWidth, screenPosition.y + EditorUI::EditorUIService::s_TextBackgroundHeight),
			ImColor(EditorUI::EditorUIService::s_ActiveBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);
		draw_list->AddRectFilled(ImVec2(EditorUIService::s_WindowPosition.x + EditorUIService::s_SecondaryTextPosThree - 5.0f, screenPosition.y),
			ImVec2(EditorUIService::s_WindowPosition.x + EditorUIService::s_SecondaryTextPosThree + EditorUIService::s_SecondaryTextSmallWidth, screenPosition.y + EditorUI::EditorUIService::s_TextBackgroundHeight),
			ImColor(EditorUI::EditorUIService::s_ActiveBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);

		// Display Item
		if (m_Flags & EditVec3_Indented)
		{
			ImGui::SetCursorPosX(EditorUIService::s_TextLeftIndentOffset);
		}
		ImGui::PushStyleColor(ImGuiCol_Text, EditorUIService::m_ConfigColors.s_PrimaryTextColor);
		int32_t labelPosition = ImGui::FindPositionAfterLength(m_Label.CString(),
			m_Flags & EditVec3_Indented ? EditorUIService::s_PrimaryTextIndentedWidth : EditorUIService::s_PrimaryTextWidth);
		EditorUIService::TruncateText(m_Label.CString(), labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);
		ImGui::PopStyleColor();

		ImGui::SameLine(EditorUIService::s_SecondaryTextPosOne);

		if (m_Editing)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
			// x value
			ImGui::PushStyleColor(ImGuiCol_Text, EditorUIService::m_ConfigColors.s_HighlightColor1);
			float yPosition = ImGui::GetCursorPosY();
			ImGui::SetNextItemWidth(EditorUIService::s_SecondaryTextSmallWidth);
			if (ImGui::DragFloat(("##" + std::to_string(m_WidgetID + EditorUIService::WidgetIterator(widgetCount))).c_str(), &(m_CurrentVec3.x), m_ScrollSpeed,
				m_Bounds[0], m_Bounds[1],
				"%.2f", ImGuiSliderFlags_AlwaysClamp))
			{
				if (m_ConfirmAction)
				{
					m_ConfirmAction(*this);
				}
			}
			ImGui::PopStyleColor();
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::TextColored(EditorUIService::m_ConfigColors.s_HighlightColor1, "X-Value");
				ImGui::EndTooltip();
			}

			// y value
			ImGui::PushStyleColor(ImGuiCol_Text, EditorUIService::m_ConfigColors.s_HighlightColor2);
			ImGui::SetCursorPos({ EditorUIService::s_SecondaryTextPosTwo, yPosition });
			ImGui::SetNextItemWidth(EditorUIService::s_SecondaryTextSmallWidth);
			if (ImGui::DragFloat(("##" + std::to_string(m_WidgetID + EditorUIService::WidgetIterator(widgetCount))).c_str(), &(m_CurrentVec3.y), m_ScrollSpeed,
				m_Bounds[0], m_Bounds[1],
				"%.2f", ImGuiSliderFlags_AlwaysClamp))
			{
				if (m_ConfirmAction)
				{
					m_ConfirmAction(*this);
				}
			}
			ImGui::PopStyleColor();
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::TextColored(EditorUIService::m_ConfigColors.s_HighlightColor2, "Y-Value");
				ImGui::EndTooltip();
			}

			// z value
			ImGui::PushStyleColor(ImGuiCol_Text, EditorUIService::m_ConfigColors.s_HighlightColor3);
			ImGui::SetCursorPos({ EditorUIService::s_SecondaryTextPosThree, yPosition });
			ImGui::SetNextItemWidth(EditorUIService::s_SecondaryTextSmallWidth);
			if (ImGui::DragFloat(("##" + std::to_string(m_WidgetID + EditorUIService::WidgetIterator(widgetCount))).c_str(), &(m_CurrentVec3.z), m_ScrollSpeed,
				m_Bounds[0], m_Bounds[1],
				"%.2f", ImGuiSliderFlags_AlwaysClamp))
			{
				if (m_ConfirmAction)
				{
					m_ConfirmAction(*this);
				}
			}
			ImGui::PopStyleColor();
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::TextColored(EditorUIService::m_ConfigColors.s_HighlightColor3, "Z-Value");
				ImGui::EndTooltip();
			}
			ImGui::PopStyleVar();

		}
		else
		{
			float yPosition = ImGui::GetCursorPosY();
			ImGui::PushStyleColor(ImGuiCol_Text, EditorUIService::m_ConfigColors.s_SecondaryTextColor);
			ImGui::SetCursorPos({ EditorUIService::s_SecondaryTextPosOne, yPosition });
			int32_t floatPosition = ImGui::FindPositionAfterLength(
				Utility::Conversions::FloatToString(m_CurrentVec3.x).c_str(), EditorUIService::s_SecondaryTextSmallWidth);
			EditorUIService::TruncateText(Utility::Conversions::FloatToString(m_CurrentVec3.x),
				floatPosition == -1 ? std::numeric_limits<int32_t>::max() : floatPosition);
			ImGui::SetCursorPos({ EditorUIService::s_SecondaryTextPosTwo, yPosition });
			floatPosition = ImGui::FindPositionAfterLength(
				Utility::Conversions::FloatToString(m_CurrentVec3.y).c_str(), EditorUIService::s_SecondaryTextSmallWidth);
			EditorUIService::TruncateText(Utility::Conversions::FloatToString(m_CurrentVec3.y),
				floatPosition == -1 ? std::numeric_limits<int32_t>::max() : floatPosition);
			ImGui::SetCursorPos({ EditorUIService::s_SecondaryTextPosThree, yPosition });
			floatPosition = ImGui::FindPositionAfterLength(
				Utility::Conversions::FloatToString(m_CurrentVec3.z).c_str(), EditorUIService::s_SecondaryTextSmallWidth);
			EditorUIService::TruncateText(Utility::Conversions::FloatToString(m_CurrentVec3.z),
				floatPosition == -1 ? std::numeric_limits<int32_t>::max() : floatPosition);
			ImGui::PopStyleColor();
		}

		ImGui::SameLine();
		EditorUIService::CreateButton(m_WidgetID + EditorUIService::WidgetIterator(widgetCount), [&]()
			{
				Utility::Operations::ToggleBoolean(m_Editing);
			},
			EditorUIService::s_SmallEditButton,
			m_Editing, m_Editing ? EditorUIService::m_ConfigColors.s_PrimaryTextColor : EditorUIService::m_ConfigColors.s_DisabledColor);
	}
	void EditVec4Widget::RenderVec4()
	{
		// Local Variables
		FixedString<16> id{ "##" };
		id.AppendInteger(m_WidgetID);
		uint32_t widgetCount{ 0 };

		// Store the memento
		EditorMemento memento{ m_CurrentVec4, &*this };

		// Draw backgrounds
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec2 screenPosition = ImGui::GetCursorScreenPos();
		draw_list->AddRectFilled(ImVec2(EditorUIService::s_WindowPosition.x + EditorUIService::s_SecondaryTextPosOne - 5.0f, screenPosition.y),
			ImVec2(EditorUIService::s_WindowPosition.x + EditorUIService::s_SecondaryTextPosOne + EditorUIService::s_SecondaryTextSmallWidth, screenPosition.y + EditorUI::EditorUIService::s_TextBackgroundHeight),
			ImColor(EditorUI::EditorUIService::s_ActiveBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);
		draw_list->AddRectFilled(ImVec2(EditorUIService::s_WindowPosition.x + EditorUIService::s_SecondaryTextPosTwo - 5.0f, screenPosition.y),
			ImVec2(EditorUIService::s_WindowPosition.x + EditorUIService::s_SecondaryTextPosTwo + EditorUIService::s_SecondaryTextSmallWidth, screenPosition.y + EditorUI::EditorUIService::s_TextBackgroundHeight),
			ImColor(EditorUI::EditorUIService::s_ActiveBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);
		draw_list->AddRectFilled(ImVec2(EditorUIService::s_WindowPosition.x + EditorUIService::s_SecondaryTextPosThree - 5.0f, screenPosition.y),
			ImVec2(EditorUIService::s_WindowPosition.x + EditorUIService::s_SecondaryTextPosThree + EditorUIService::s_SecondaryTextSmallWidth, screenPosition.y + EditorUI::EditorUIService::s_TextBackgroundHeight),
			ImColor(EditorUI::EditorUIService::s_ActiveBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);
		draw_list->AddRectFilled(ImVec2(EditorUIService::s_WindowPosition.x + EditorUIService::s_SecondaryTextPosFour - 5.0f, screenPosition.y),
			ImVec2(EditorUIService::s_WindowPosition.x + EditorUIService::s_SecondaryTextPosFour + ((m_Flags & EditVec4_RGBA) ?
				EditorUIService::s_SecondaryTextSmallWidth - 25.0f : EditorUIService::s_SecondaryTextSmallWidth), screenPosition.y + EditorUI::EditorUIService::s_TextBackgroundHeight),
			ImColor(EditorUI::EditorUIService::s_ActiveBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);

		if (m_Flags & EditVec4_RGBA)
		{
			draw_list->AddRectFilled(ImVec2(EditorUIService::s_WindowPosition.x + EditorUIService::s_SecondaryTextPosFour + EditorUIService::s_SecondaryTextSmallWidth - 23.0f, screenPosition.y),
				ImVec2(EditorUIService::s_WindowPosition.x + EditorUIService::s_SecondaryTextPosFour + EditorUIService::s_SecondaryTextSmallWidth, screenPosition.y + EditorUI::EditorUIService::s_TextBackgroundHeight),
				ImColor(EditorUI::EditorUIService::s_ActiveBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);
		}


		// Display Item
		if (m_Flags & EditVec4_Indented)
		{
			ImGui::SetCursorPosX(EditorUIService::s_TextLeftIndentOffset);
		}
		ImGui::PushStyleColor(ImGuiCol_Text, EditorUIService::m_ConfigColors.s_PrimaryTextColor);
		int32_t labelPosition = ImGui::FindPositionAfterLength(m_Label.CString(),
			m_Flags & EditVec4_Indented ? EditorUIService::s_PrimaryTextIndentedWidth : EditorUIService::s_PrimaryTextWidth);
		EditorUIService::TruncateText(m_Label.CString(), labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);
		ImGui::PopStyleColor();

		ImGui::SameLine(EditorUIService::s_SecondaryTextPosOne);

		if (m_Editing)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
			// x value
			ImGui::PushStyleColor(ImGuiCol_Text, (m_Flags & EditVec4_RGBA) ? k_Red : EditorUIService::m_ConfigColors.s_HighlightColor1);
			static bool s_ModifyingX{ false };
			float yPosition = ImGui::GetCursorPosY();
			ImGui::SetNextItemWidth(EditorUIService::s_SecondaryTextSmallWidth);
			if (ImGui::DragFloat(("##" + std::to_string(m_WidgetID + EditorUIService::WidgetIterator(widgetCount))).c_str(), &(m_CurrentVec4.x), m_ScrollSpeed,
				m_Bounds[0], m_Bounds[1],
				"%.2f", ImGuiSliderFlags_AlwaysClamp))
			{
				if (!s_ModifyingX)
				{
					EditorUIService::StoreUndoMemento(memento);
					s_ModifyingX = true;
				}

				if (m_ConfirmAction)
				{
					m_ConfirmAction(*this);
				}
			}
			else
			{
				if (!ImGui::IsItemActive())
				{
					s_ModifyingX = false;
				}
			}
			ImGui::PopStyleColor();
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::TextColored(m_Flags & EditVec4_RGBA ? k_Red : EditorUIService::m_ConfigColors.s_HighlightColor1,
					m_Flags & EditVec4_RGBA ? "Red Channel" : "X-Value");
				ImGui::EndTooltip();
			}

			// y value
			ImGui::PushStyleColor(ImGuiCol_Text, (m_Flags & EditVec4_RGBA) ? k_Green : EditorUIService::m_ConfigColors.s_HighlightColor2);
			static bool s_ModifyingY{ false };
			ImGui::SetCursorPos({ EditorUIService::s_SecondaryTextPosTwo, yPosition });
			ImGui::SetNextItemWidth(EditorUIService::s_SecondaryTextSmallWidth);
			if (ImGui::DragFloat(("##" + std::to_string(m_WidgetID + EditorUIService::WidgetIterator(widgetCount))).c_str(), &(m_CurrentVec4.y), m_ScrollSpeed,
				m_Bounds[0], m_Bounds[1],
				"%.2f", ImGuiSliderFlags_AlwaysClamp))
			{
				if (!s_ModifyingY)
				{
					EditorUIService::StoreUndoMemento(memento);
					s_ModifyingY = true;
				}
				if (m_ConfirmAction)
				{
					m_ConfirmAction(*this);
				}
			}
			else
			{
				if (!ImGui::IsItemActive())
				{
					s_ModifyingY = false;
				}
			}
			ImGui::PopStyleColor();
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::TextColored(m_Flags & EditVec4_RGBA ? k_Green : EditorUIService::m_ConfigColors.s_HighlightColor2,
					m_Flags & EditVec4_RGBA ? "Green Channel" : "Y-Value");
				ImGui::EndTooltip();
			}

			// z value
			ImGui::PushStyleColor(ImGuiCol_Text, (m_Flags & EditVec4_RGBA) ? k_Blue : EditorUIService::m_ConfigColors.s_HighlightColor3);
			static bool s_ModifyingZ{ false };
			ImGui::SetCursorPos({ EditorUIService::s_SecondaryTextPosThree, yPosition });
			ImGui::SetNextItemWidth(EditorUIService::s_SecondaryTextSmallWidth);
			if (ImGui::DragFloat(("##" + std::to_string(m_WidgetID + EditorUIService::WidgetIterator(widgetCount))).c_str(), &(m_CurrentVec4.z), m_ScrollSpeed,
				m_Bounds[0], m_Bounds[1],
				"%.2f", ImGuiSliderFlags_AlwaysClamp))
			{
				if (!s_ModifyingZ)
				{
					EditorUIService::StoreUndoMemento(memento);
					s_ModifyingZ = true;
				}
				if (m_ConfirmAction)
				{
					m_ConfirmAction(*this);
				}
			}
			else
			{
				if (!ImGui::IsItemActive())
				{
					s_ModifyingZ = false;
				}
			}
			ImGui::PopStyleColor();
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::TextColored(m_Flags & EditVec4_RGBA ? k_Blue : EditorUIService::m_ConfigColors.s_HighlightColor3,
					m_Flags & EditVec4_RGBA ? "Blue Channel" : "Z-Value");
				ImGui::EndTooltip();
			}


			// w value
			ImGui::PushStyleColor(ImGuiCol_Text, (m_Flags & EditVec4_RGBA) ? k_Alpha : EditorUIService::m_ConfigColors.s_HighlightColor4);
			static bool s_ModifyingW{ false };
			ImGui::SetCursorPos({ EditorUIService::s_SecondaryTextPosFour, yPosition });
			ImGui::SetNextItemWidth((m_Flags & EditVec4_RGBA) ? EditorUIService::s_SecondaryTextSmallWidth - 28.0f : EditorUIService::s_SecondaryTextSmallWidth);
			if (ImGui::DragFloat(("##" + std::to_string(m_WidgetID + EditorUIService::WidgetIterator(widgetCount))).c_str(), &(m_CurrentVec4.w), m_ScrollSpeed,
				m_Bounds[0], m_Bounds[1],
				"%.2f", ImGuiSliderFlags_AlwaysClamp))
			{
				if (!s_ModifyingW)
				{
					EditorUIService::StoreUndoMemento(memento);
					s_ModifyingW = true;
				}
				if (m_ConfirmAction)
				{
					m_ConfirmAction(*this);
				}
			}
			else
			{
				if (!ImGui::IsItemActive())
				{
					s_ModifyingW = false;
				}
			}
			ImGui::PopStyleColor();
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::TextColored(m_Flags & EditVec4_RGBA ? k_Alpha : EditorUIService::m_ConfigColors.s_HighlightColor4,
					m_Flags & EditVec4_RGBA ? "Alpha Channel" : "W-Value");
				ImGui::EndTooltip();
			}

			ImGui::PopStyleVar();

			if (m_Flags & EditVec4_RGBA)
			{
				ImVec4 colorPickerValue{ Utility::MathVec4ToImVec4(m_CurrentVec4) };
				static bool s_ModifyingColorButton{ false };
				ImGui::SetCursorPos({ EditorUIService::s_SecondaryTextPosFour + EditorUIService::s_SecondaryTextSmallWidth - 21.0f, yPosition + 1.0f });

				if (EditorUIService::DrawColorPickerButton(("##" + std::to_string(m_WidgetID + EditorUIService::WidgetIterator(widgetCount))).c_str(),
					colorPickerValue))
				{
					if (!s_ModifyingColorButton)
					{
						EditorUIService::StoreUndoMemento(memento);
						s_ModifyingColorButton = true;
					}
					m_CurrentVec4 = Utility::ImVec4ToMathVec4(colorPickerValue);
					if (m_ConfirmAction)
					{
						m_ConfirmAction(*this);
					}
				}
				else
				{
					if (!ImGui::IsItemActive())
					{
						s_ModifyingColorButton = false;
					}
				}
			}
		}
		else
		{
			float yPosition = ImGui::GetCursorPosY();
			ImGui::PushStyleColor(ImGuiCol_Text, EditorUIService::m_ConfigColors.s_SecondaryTextColor);
			ImGui::SetCursorPos({ EditorUIService::s_SecondaryTextPosOne, yPosition });
			int32_t floatPosition = ImGui::FindPositionAfterLength(
				Utility::Conversions::FloatToString(m_CurrentVec4.x).c_str(), EditorUIService::s_SecondaryTextSmallWidth);
			EditorUIService::TruncateText(Utility::Conversions::FloatToString(m_CurrentVec4.x),
				floatPosition == -1 ? std::numeric_limits<int32_t>::max() : floatPosition);
			ImGui::SetCursorPos({ EditorUIService::s_SecondaryTextPosTwo, yPosition });
			floatPosition = ImGui::FindPositionAfterLength(
				Utility::Conversions::FloatToString(m_CurrentVec4.y).c_str(), EditorUIService::s_SecondaryTextSmallWidth);
			EditorUIService::TruncateText(Utility::Conversions::FloatToString(m_CurrentVec4.y),
				floatPosition == -1 ? std::numeric_limits<int32_t>::max() : floatPosition);
			ImGui::SetCursorPos({ EditorUIService::s_SecondaryTextPosThree, yPosition });
			floatPosition = ImGui::FindPositionAfterLength(
				Utility::Conversions::FloatToString(m_CurrentVec4.z).c_str(), EditorUIService::s_SecondaryTextSmallWidth);
			EditorUIService::TruncateText(Utility::Conversions::FloatToString(m_CurrentVec4.z),
				floatPosition == -1 ? std::numeric_limits<int32_t>::max() : floatPosition);
			ImGui::SetCursorPos({ EditorUIService::s_SecondaryTextPosFour, yPosition });
			floatPosition = ImGui::FindPositionAfterLength(
				Utility::Conversions::FloatToString(m_CurrentVec4.w).c_str(), EditorUIService::s_SecondaryTextSmallWidth -
				((m_Flags & EditVec4_RGBA) ? 24.0f : 0.0f));
			EditorUIService::TruncateText(Utility::Conversions::FloatToString(m_CurrentVec4.w),
				floatPosition == -1 ? std::numeric_limits<int32_t>::max() : floatPosition);
			ImGui::PopStyleColor();

			if (m_Flags & EditVec4_RGBA)
			{
				ImVec4 colorPickerValue{ Utility::MathVec4ToImVec4(m_CurrentVec4) };
				ImGuiColorEditFlags misc_flags = ImGuiColorEditFlags_NoInputs |
					ImGuiColorEditFlags_NoLabel |
					ImGuiColorEditFlags_AlphaPreviewHalf |
					ImGuiColorEditFlags_NoSidePreview |
					ImGuiColorEditFlags_NoPicker;
				ImGui::SetCursorPos({ EditorUIService::s_SecondaryTextPosFour + EditorUIService::s_SecondaryTextSmallWidth - 21.0f, yPosition + 1.0f });
				ImGui::ColorButton(("##" + std::to_string(m_WidgetID + EditorUIService::WidgetIterator(widgetCount))).c_str(),
					colorPickerValue,
					misc_flags, ImVec2(18.0f, 18.0f));
			}
		}

		ImGui::SameLine();
		EditorUIService::CreateButton(m_WidgetID + EditorUIService::WidgetIterator(widgetCount), [&]()
			{
				if (m_Flags & EditorUI::EditVec4_HandleEditButtonExternally)
				{
					if (m_OnEdit)
					{
						m_OnEdit(*this);
					}
				}
				else
				{
					Utility::Operations::ToggleBoolean(m_Editing);
				}
			},
			EditorUIService::s_SmallEditButton,
			m_Editing, m_Editing ? EditorUIService::m_ConfigColors.s_PrimaryTextColor : EditorUIService::m_ConfigColors.s_DisabledColor);
	}
}