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
		draw_list->AddRectFilled(ImVec2(EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosOne - 5.0f, screenPosition.y),
			ImVec2(EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosOne + EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth, screenPosition.y + EditorUI::EditorUIContext::m_ConfigSpacing.m_TextBackgroundHeight),
			ImColor(EditorUI::EditorUIContext::m_ActiveWindowData.m_ActiveBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);


		// Display Item
		if (m_Flags & EditFloat_Indented)
		{
			ImGui::SetCursorPosX(EditorUIContext::m_ConfigSpacing.m_PrimaryTextIndent);
		}


		ImGui::PushStyleColor(ImGuiCol_Text, EditorUIContext::m_ConfigColors.m_PrimaryTextColor);
		int32_t labelPosition = ImGui::FindPositionAfterLength(m_Label.CString(),
			m_Flags & EditFloat_Indented ? EditorUIContext::m_ActiveWindowData.m_PrimaryTextIndentedWidth : EditorUIContext::m_ActiveWindowData.m_PrimaryTextWidth);
		EditorUIContext::RenderTruncatedText(m_Label.CString(), labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);
		ImGui::PopStyleColor();

		ImGui::SameLine(EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosOne);

		if (m_Editing)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
			// x value
			ImGui::PushStyleColor(ImGuiCol_Text, EditorUIContext::m_ConfigColors.m_HighlightColor1);
			ImGui::SetNextItemWidth(EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth);
			if (ImGui::DragFloat(("##" + std::to_string(m_WidgetID + EditorUIContext::GetNextChildID(widgetCount))).c_str(), &(m_CurrentFloat), m_ScrollSpeed,
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
				ImGui::TextColored(EditorUIContext::m_ConfigColors.m_HighlightColor1, "X-Value");
				ImGui::EndTooltip();
			}
			ImGui::PopStyleVar();

		}
		else
		{
			float yPosition = ImGui::GetCursorPosY();
			ImGui::SetCursorPos({ EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosOne, yPosition });
			ImGui::PushStyleColor(ImGuiCol_Text, EditorUIContext::m_ConfigColors.m_SecondaryTextColor);
			int32_t floatPosition = ImGui::FindPositionAfterLength(Utility::Conversions::FloatToString(m_CurrentFloat).c_str(), EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth);
			EditorUIContext::RenderTruncatedText(Utility::Conversions::FloatToString(m_CurrentFloat),
				floatPosition == -1 ? std::numeric_limits<int32_t>::max() : floatPosition);
			ImGui::PopStyleColor();
		}

		ImGui::SameLine();
		EditorUIContext::RenderInlineButton(m_WidgetID + EditorUIContext::GetNextChildID(widgetCount), [&]()
			{
				Utility::Operations::ToggleBoolean(m_Editing);
			},
			EditorUIContext::m_UIPresets.m_SmallEditButton,
			m_Editing, m_Editing ? EditorUIContext::m_ConfigColors.m_PrimaryTextColor : EditorUIContext::m_ConfigColors.m_DisabledColor);
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
		draw_list->AddRectFilled(ImVec2(EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosOne - 5.0f, screenPosition.y),
			ImVec2(EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosOne + EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth, screenPosition.y + EditorUI::EditorUIContext::m_ConfigSpacing.m_TextBackgroundHeight),
			ImColor(EditorUI::EditorUIContext::m_ActiveWindowData.m_ActiveBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);
		draw_list->AddRectFilled(ImVec2(EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosTwo - 5.0f, screenPosition.y),
			ImVec2(EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosTwo + EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth, screenPosition.y + EditorUI::EditorUIContext::m_ConfigSpacing.m_TextBackgroundHeight),
			ImColor(EditorUI::EditorUIContext::m_ActiveWindowData.m_ActiveBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);

		// Display Item
		if (m_Flags & EditVec2_Indented)
		{
			ImGui::SetCursorPosX(EditorUIContext::m_ConfigSpacing.m_PrimaryTextIndent);
		}
		ImGui::PushStyleColor(ImGuiCol_Text, EditorUIContext::m_ConfigColors.m_PrimaryTextColor);

		int32_t labelPosition = ImGui::FindPositionAfterLength(m_Label.CString(),
			m_Flags & EditVec2_Indented ? EditorUIContext::m_ActiveWindowData.m_PrimaryTextIndentedWidth : EditorUIContext::m_ActiveWindowData.m_PrimaryTextWidth);
		EditorUIContext::RenderTruncatedText(m_Label.CString(), labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);

		ImGui::PopStyleColor();
		ImGui::SameLine(EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosOne);

		if (m_Editing)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
			// x value
			ImGui::PushStyleColor(ImGuiCol_Text, EditorUIContext::m_ConfigColors.m_HighlightColor1);
			float yPosition = ImGui::GetCursorPosY();
			ImGui::SetNextItemWidth(EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth);
			if (ImGui::DragFloat(("##" + std::to_string(m_WidgetID + EditorUIContext::GetNextChildID(widgetCount))).c_str(), &(m_CurrentVec2.x), m_ScrollSpeed,
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
				ImGui::TextColored(EditorUIContext::m_ConfigColors.m_HighlightColor1, "X-Value");
				ImGui::EndTooltip();
			}

			// y value
			ImGui::PushStyleColor(ImGuiCol_Text, EditorUIContext::m_ConfigColors.m_HighlightColor2);
			ImGui::SetCursorPos({ EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosTwo, yPosition });
			ImGui::SetNextItemWidth(EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth);
			if (ImGui::DragFloat(("##" + std::to_string(m_WidgetID + EditorUIContext::GetNextChildID(widgetCount))).c_str(), &(m_CurrentVec2.y), m_ScrollSpeed,
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
				ImGui::TextColored(EditorUIContext::m_ConfigColors.m_HighlightColor2, "Y-Value");
				ImGui::EndTooltip();
			}
			ImGui::PopStyleVar();

		}
		else
		{
			float yPosition = ImGui::GetCursorPosY();
			ImGui::SetCursorPos({ EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosOne, yPosition });
			ImGui::PushStyleColor(ImGuiCol_Text, EditorUIContext::m_ConfigColors.m_SecondaryTextColor);
			int32_t floatPosition = ImGui::FindPositionAfterLength(
				Utility::Conversions::FloatToString(m_CurrentVec2.x).c_str(), EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth);
			EditorUIContext::RenderTruncatedText(Utility::Conversions::FloatToString(m_CurrentVec2.x),
				floatPosition == -1 ? std::numeric_limits<int32_t>::max() : floatPosition);
			ImGui::SetCursorPos({ EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosTwo, yPosition });
			floatPosition = ImGui::FindPositionAfterLength(
				Utility::Conversions::FloatToString(m_CurrentVec2.y).c_str(), EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth);
			EditorUIContext::RenderTruncatedText(Utility::Conversions::FloatToString(m_CurrentVec2.y),
				floatPosition == -1 ? std::numeric_limits<int32_t>::max() : floatPosition);
			ImGui::PopStyleColor();
		}

		ImGui::SameLine();
		EditorUIContext::RenderInlineButton(m_WidgetID + EditorUIContext::GetNextChildID(widgetCount), [&]()
			{
				Utility::Operations::ToggleBoolean(m_Editing);
			},
			EditorUIContext::m_UIPresets.m_SmallEditButton,
			m_Editing, m_Editing ? EditorUIContext::m_ConfigColors.m_PrimaryTextColor : EditorUIContext::m_ConfigColors.m_DisabledColor);
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
		draw_list->AddRectFilled(ImVec2(EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosOne - 5.0f, screenPosition.y),
			ImVec2(EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosOne + EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth, screenPosition.y + EditorUI::EditorUIContext::m_ConfigSpacing.m_TextBackgroundHeight),
			ImColor(EditorUI::EditorUIContext::m_ActiveWindowData.m_ActiveBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);
		draw_list->AddRectFilled(ImVec2(EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosTwo - 5.0f, screenPosition.y),
			ImVec2(EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosTwo + EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth, screenPosition.y + EditorUI::EditorUIContext::m_ConfigSpacing.m_TextBackgroundHeight),
			ImColor(EditorUI::EditorUIContext::m_ActiveWindowData.m_ActiveBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);
		draw_list->AddRectFilled(ImVec2(EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosThree - 5.0f, screenPosition.y),
			ImVec2(EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosThree + EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth, screenPosition.y + EditorUI::EditorUIContext::m_ConfigSpacing.m_TextBackgroundHeight),
			ImColor(EditorUI::EditorUIContext::m_ActiveWindowData.m_ActiveBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);

		// Display Item
		if (m_Flags & EditVec3_Indented)
		{
			ImGui::SetCursorPosX(EditorUIContext::m_ConfigSpacing.m_PrimaryTextIndent);
		}
		ImGui::PushStyleColor(ImGuiCol_Text, EditorUIContext::m_ConfigColors.m_PrimaryTextColor);
		int32_t labelPosition = ImGui::FindPositionAfterLength(m_Label.CString(),
			m_Flags & EditVec3_Indented ? EditorUIContext::m_ActiveWindowData.m_PrimaryTextIndentedWidth : EditorUIContext::m_ActiveWindowData.m_PrimaryTextWidth);
		EditorUIContext::RenderTruncatedText(m_Label.CString(), labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);
		ImGui::PopStyleColor();

		ImGui::SameLine(EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosOne);

		if (m_Editing)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
			// x value
			ImGui::PushStyleColor(ImGuiCol_Text, EditorUIContext::m_ConfigColors.m_HighlightColor1);
			float yPosition = ImGui::GetCursorPosY();
			ImGui::SetNextItemWidth(EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth);
			if (ImGui::DragFloat(("##" + std::to_string(m_WidgetID + EditorUIContext::GetNextChildID(widgetCount))).c_str(), &(m_CurrentVec3.x), m_ScrollSpeed,
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
				ImGui::TextColored(EditorUIContext::m_ConfigColors.m_HighlightColor1, "X-Value");
				ImGui::EndTooltip();
			}

			// y value
			ImGui::PushStyleColor(ImGuiCol_Text, EditorUIContext::m_ConfigColors.m_HighlightColor2);
			ImGui::SetCursorPos({ EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosTwo, yPosition });
			ImGui::SetNextItemWidth(EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth);
			if (ImGui::DragFloat(("##" + std::to_string(m_WidgetID + EditorUIContext::GetNextChildID(widgetCount))).c_str(), &(m_CurrentVec3.y), m_ScrollSpeed,
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
				ImGui::TextColored(EditorUIContext::m_ConfigColors.m_HighlightColor2, "Y-Value");
				ImGui::EndTooltip();
			}

			// z value
			ImGui::PushStyleColor(ImGuiCol_Text, EditorUIContext::m_ConfigColors.m_HighlightColor3);
			ImGui::SetCursorPos({ EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosThree, yPosition });
			ImGui::SetNextItemWidth(EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth);
			if (ImGui::DragFloat(("##" + std::to_string(m_WidgetID + EditorUIContext::GetNextChildID(widgetCount))).c_str(), &(m_CurrentVec3.z), m_ScrollSpeed,
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
				ImGui::TextColored(EditorUIContext::m_ConfigColors.m_HighlightColor3, "Z-Value");
				ImGui::EndTooltip();
			}
			ImGui::PopStyleVar();

		}
		else
		{
			float yPosition = ImGui::GetCursorPosY();
			ImGui::PushStyleColor(ImGuiCol_Text, EditorUIContext::m_ConfigColors.m_SecondaryTextColor);
			ImGui::SetCursorPos({ EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosOne, yPosition });
			int32_t floatPosition = ImGui::FindPositionAfterLength(
				Utility::Conversions::FloatToString(m_CurrentVec3.x).c_str(), EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth);
			EditorUIContext::RenderTruncatedText(Utility::Conversions::FloatToString(m_CurrentVec3.x),
				floatPosition == -1 ? std::numeric_limits<int32_t>::max() : floatPosition);
			ImGui::SetCursorPos({ EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosTwo, yPosition });
			floatPosition = ImGui::FindPositionAfterLength(
				Utility::Conversions::FloatToString(m_CurrentVec3.y).c_str(), EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth);
			EditorUIContext::RenderTruncatedText(Utility::Conversions::FloatToString(m_CurrentVec3.y),
				floatPosition == -1 ? std::numeric_limits<int32_t>::max() : floatPosition);
			ImGui::SetCursorPos({ EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosThree, yPosition });
			floatPosition = ImGui::FindPositionAfterLength(
				Utility::Conversions::FloatToString(m_CurrentVec3.z).c_str(), EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth);
			EditorUIContext::RenderTruncatedText(Utility::Conversions::FloatToString(m_CurrentVec3.z),
				floatPosition == -1 ? std::numeric_limits<int32_t>::max() : floatPosition);
			ImGui::PopStyleColor();
		}

		ImGui::SameLine();
		EditorUIContext::RenderInlineButton(m_WidgetID + EditorUIContext::GetNextChildID(widgetCount), [&]()
			{
				Utility::Operations::ToggleBoolean(m_Editing);
			},
			EditorUIContext::m_UIPresets.m_SmallEditButton,
			m_Editing, m_Editing ? EditorUIContext::m_ConfigColors.m_PrimaryTextColor : EditorUIContext::m_ConfigColors.m_DisabledColor);
	}
	void EditVec4Widget::RenderVec4()
	{
		// Local Variables
		FixedString<16> id{ "##" };
		id.AppendInteger(m_WidgetID);
		uint32_t widgetCount{ 0 };

		// Store the memento
		UndoMemento memento{ m_CurrentVec4, &*this };

		// Draw backgrounds
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec2 screenPosition = ImGui::GetCursorScreenPos();
		draw_list->AddRectFilled(ImVec2(EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosOne - 5.0f, screenPosition.y),
			ImVec2(EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosOne + EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth, screenPosition.y + EditorUI::EditorUIContext::m_ConfigSpacing.m_TextBackgroundHeight),
			ImColor(EditorUI::EditorUIContext::m_ActiveWindowData.m_ActiveBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);
		draw_list->AddRectFilled(ImVec2(EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosTwo - 5.0f, screenPosition.y),
			ImVec2(EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosTwo + EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth, screenPosition.y + EditorUI::EditorUIContext::m_ConfigSpacing.m_TextBackgroundHeight),
			ImColor(EditorUI::EditorUIContext::m_ActiveWindowData.m_ActiveBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);
		draw_list->AddRectFilled(ImVec2(EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosThree - 5.0f, screenPosition.y),
			ImVec2(EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosThree + EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth, screenPosition.y + EditorUI::EditorUIContext::m_ConfigSpacing.m_TextBackgroundHeight),
			ImColor(EditorUI::EditorUIContext::m_ActiveWindowData.m_ActiveBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);
		draw_list->AddRectFilled(ImVec2(EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosFour - 5.0f, screenPosition.y),
			ImVec2(EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosFour + ((m_Flags & EditVec4_RGBA) ?
				EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth - 25.0f : EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth), screenPosition.y + EditorUI::EditorUIContext::m_ConfigSpacing.m_TextBackgroundHeight),
			ImColor(EditorUI::EditorUIContext::m_ActiveWindowData.m_ActiveBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);

		if (m_Flags & EditVec4_RGBA)
		{
			draw_list->AddRectFilled(ImVec2(EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosFour + EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth - 23.0f, screenPosition.y),
				ImVec2(EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosFour + EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth, screenPosition.y + EditorUI::EditorUIContext::m_ConfigSpacing.m_TextBackgroundHeight),
				ImColor(EditorUI::EditorUIContext::m_ActiveWindowData.m_ActiveBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);
		}


		// Display Item
		if (m_Flags & EditVec4_Indented)
		{
			ImGui::SetCursorPosX(EditorUIContext::m_ConfigSpacing.m_PrimaryTextIndent);
		}
		ImGui::PushStyleColor(ImGuiCol_Text, EditorUIContext::m_ConfigColors.m_PrimaryTextColor);
		int32_t labelPosition = ImGui::FindPositionAfterLength(m_Label.CString(),
			m_Flags & EditVec4_Indented ? EditorUIContext::m_ActiveWindowData.m_PrimaryTextIndentedWidth : EditorUIContext::m_ActiveWindowData.m_PrimaryTextWidth);
		EditorUIContext::RenderTruncatedText(m_Label.CString(), labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);
		ImGui::PopStyleColor();

		ImGui::SameLine(EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosOne);

		if (m_Editing)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
			// x value
			ImGui::PushStyleColor(ImGuiCol_Text, (m_Flags & EditVec4_RGBA) ? k_Red : EditorUIContext::m_ConfigColors.m_HighlightColor1);
			static bool s_ModifyingX{ false };
			float yPosition = ImGui::GetCursorPosY();
			ImGui::SetNextItemWidth(EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth);
			if (ImGui::DragFloat(("##" + std::to_string(m_WidgetID + EditorUIContext::GetNextChildID(widgetCount))).c_str(), &(m_CurrentVec4.x), m_ScrollSpeed,
				m_Bounds[0], m_Bounds[1],
				"%.2f", ImGuiSliderFlags_AlwaysClamp))
			{
				if (!s_ModifyingX)
				{
					EditorUIContext::s_UndoStack.StoreUndo(memento);
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
				ImGui::TextColored(m_Flags & EditVec4_RGBA ? k_Red : EditorUIContext::m_ConfigColors.m_HighlightColor1,
					m_Flags & EditVec4_RGBA ? "Red Channel" : "X-Value");
				ImGui::EndTooltip();
			}

			// y value
			ImGui::PushStyleColor(ImGuiCol_Text, (m_Flags & EditVec4_RGBA) ? k_Green : EditorUIContext::m_ConfigColors.m_HighlightColor2);
			static bool s_ModifyingY{ false };
			ImGui::SetCursorPos({ EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosTwo, yPosition });
			ImGui::SetNextItemWidth(EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth);
			if (ImGui::DragFloat(("##" + std::to_string(m_WidgetID + EditorUIContext::GetNextChildID(widgetCount))).c_str(), &(m_CurrentVec4.y), m_ScrollSpeed,
				m_Bounds[0], m_Bounds[1],
				"%.2f", ImGuiSliderFlags_AlwaysClamp))
			{
				if (!s_ModifyingY)
				{
					EditorUIContext::s_UndoStack.StoreUndo(memento);
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
				ImGui::TextColored(m_Flags & EditVec4_RGBA ? k_Green : EditorUIContext::m_ConfigColors.m_HighlightColor2,
					m_Flags & EditVec4_RGBA ? "Green Channel" : "Y-Value");
				ImGui::EndTooltip();
			}

			// z value
			ImGui::PushStyleColor(ImGuiCol_Text, (m_Flags & EditVec4_RGBA) ? k_Blue : EditorUIContext::m_ConfigColors.m_HighlightColor3);
			static bool s_ModifyingZ{ false };
			ImGui::SetCursorPos({ EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosThree, yPosition });
			ImGui::SetNextItemWidth(EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth);
			if (ImGui::DragFloat(("##" + std::to_string(m_WidgetID + EditorUIContext::GetNextChildID(widgetCount))).c_str(), &(m_CurrentVec4.z), m_ScrollSpeed,
				m_Bounds[0], m_Bounds[1],
				"%.2f", ImGuiSliderFlags_AlwaysClamp))
			{
				if (!s_ModifyingZ)
				{
					EditorUIContext::s_UndoStack.StoreUndo(memento);
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
				ImGui::TextColored(m_Flags & EditVec4_RGBA ? k_Blue : EditorUIContext::m_ConfigColors.m_HighlightColor3,
					m_Flags & EditVec4_RGBA ? "Blue Channel" : "Z-Value");
				ImGui::EndTooltip();
			}


			// w value
			ImGui::PushStyleColor(ImGuiCol_Text, (m_Flags & EditVec4_RGBA) ? k_Alpha : EditorUIContext::m_ConfigColors.m_HighlightColor4);
			static bool s_ModifyingW{ false };
			ImGui::SetCursorPos({ EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosFour, yPosition });
			ImGui::SetNextItemWidth((m_Flags & EditVec4_RGBA) ? EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth - 28.0f : EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth);
			if (ImGui::DragFloat(("##" + std::to_string(m_WidgetID + EditorUIContext::GetNextChildID(widgetCount))).c_str(), &(m_CurrentVec4.w), m_ScrollSpeed,
				m_Bounds[0], m_Bounds[1],
				"%.2f", ImGuiSliderFlags_AlwaysClamp))
			{
				if (!s_ModifyingW)
				{
					EditorUIContext::s_UndoStack.StoreUndo(memento);
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
				ImGui::TextColored(m_Flags & EditVec4_RGBA ? k_Alpha : EditorUIContext::m_ConfigColors.m_HighlightColor4,
					m_Flags & EditVec4_RGBA ? "Alpha Channel" : "W-Value");
				ImGui::EndTooltip();
			}

			ImGui::PopStyleVar();

			if (m_Flags & EditVec4_RGBA)
			{
				ImVec4 colorPickerValue{ Utility::MathVec4ToImVec4(m_CurrentVec4) };
				static bool s_ModifyingColorButton{ false };
				ImGui::SetCursorPos({ EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosFour + EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth - 21.0f, yPosition + 1.0f });

				if (EditorUIContext::RenderColorPicker(("##" + std::to_string(m_WidgetID + EditorUIContext::GetNextChildID(widgetCount))).c_str(),
					colorPickerValue))
				{
					if (!s_ModifyingColorButton)
					{
						EditorUIContext::s_UndoStack.StoreUndo(memento);
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
			ImGui::PushStyleColor(ImGuiCol_Text, EditorUIContext::m_ConfigColors.m_SecondaryTextColor);
			ImGui::SetCursorPos({ EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosOne, yPosition });
			int32_t floatPosition = ImGui::FindPositionAfterLength(
				Utility::Conversions::FloatToString(m_CurrentVec4.x).c_str(), EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth);
			EditorUIContext::RenderTruncatedText(Utility::Conversions::FloatToString(m_CurrentVec4.x),
				floatPosition == -1 ? std::numeric_limits<int32_t>::max() : floatPosition);
			ImGui::SetCursorPos({ EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosTwo, yPosition });
			floatPosition = ImGui::FindPositionAfterLength(
				Utility::Conversions::FloatToString(m_CurrentVec4.y).c_str(), EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth);
			EditorUIContext::RenderTruncatedText(Utility::Conversions::FloatToString(m_CurrentVec4.y),
				floatPosition == -1 ? std::numeric_limits<int32_t>::max() : floatPosition);
			ImGui::SetCursorPos({ EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosThree, yPosition });
			floatPosition = ImGui::FindPositionAfterLength(
				Utility::Conversions::FloatToString(m_CurrentVec4.z).c_str(), EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth);
			EditorUIContext::RenderTruncatedText(Utility::Conversions::FloatToString(m_CurrentVec4.z),
				floatPosition == -1 ? std::numeric_limits<int32_t>::max() : floatPosition);
			ImGui::SetCursorPos({ EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosFour, yPosition });
			floatPosition = ImGui::FindPositionAfterLength(
				Utility::Conversions::FloatToString(m_CurrentVec4.w).c_str(), EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth -
				((m_Flags & EditVec4_RGBA) ? 24.0f : 0.0f));
			EditorUIContext::RenderTruncatedText(Utility::Conversions::FloatToString(m_CurrentVec4.w),
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
				ImGui::SetCursorPos({ EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosFour + EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth - 21.0f, yPosition + 1.0f });
				ImGui::ColorButton(("##" + std::to_string(m_WidgetID + EditorUIContext::GetNextChildID(widgetCount))).c_str(),
					colorPickerValue,
					misc_flags, ImVec2(18.0f, 18.0f));
			}
		}

		ImGui::SameLine();
		EditorUIContext::RenderInlineButton(m_WidgetID + EditorUIContext::GetNextChildID(widgetCount), [&]()
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
			EditorUIContext::m_UIPresets.m_SmallEditButton,
			m_Editing, m_Editing ? EditorUIContext::m_ConfigColors.m_PrimaryTextColor : EditorUIContext::m_ConfigColors.m_DisabledColor);
	}
}