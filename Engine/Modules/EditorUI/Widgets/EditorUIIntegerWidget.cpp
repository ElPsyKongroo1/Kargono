#include "kgpch.h"

#include "Modules/EditorUI/Widgets/EditorUIIntegerWidget.h"
#include "Modules/EditorUI/EditorUIContext.h"

#include "Kargono/Utility/Operations.h"

namespace Kargono::EditorUI
{
	void EditIntegerWidget::RenderInteger()
	{
		ResetChildID();

		// Get relevant context data
		ActiveWindowData& windowData{ EditorUIContext::m_ActiveWindowData };
		ConfigColors& configColors{ EditorUIContext::m_ConfigColors};

		// Draw background
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec2 screenPosition = ImGui::GetCursorScreenPos();
		draw_list->AddRectFilled(ImVec2(windowData.m_WindowPosition.x + windowData.m_SecondaryTextPosOne - 5.0f, screenPosition.y),
			ImVec2(windowData.m_WindowPosition.x + windowData.m_SecondaryTextPosOne + windowData.m_SecondaryTextSmallWidth, screenPosition.y + EditorUIContext::m_ConfigSpacing.m_TextBackgroundHeight),
			ImColor(windowData.m_ActiveBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);

		// Display Item
		if (m_Flags & EditInteger_Indented)
		{
			ImGui::SetCursorPosX(EditorUIContext::m_ConfigSpacing.m_PrimaryTextIndent);
		}

		ImGui::PushStyleColor(ImGuiCol_Text, configColors.m_PrimaryTextColor);
		int32_t labelPosition = ImGui::FindPositionAfterLength(m_Label.CString(),
			m_Flags & EditInteger_Indented ? windowData.m_PrimaryTextIndentedWidth : windowData.m_PrimaryTextWidth);
		EditorUIContext::RenderTruncatedText(m_Label.CString(), labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);
		ImGui::PopStyleColor();

		ImGui::SameLine(windowData.m_SecondaryTextPosOne);

		if (m_Editing)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
			// x value
			ImGui::PushStyleColor(ImGuiCol_Text, configColors.m_HighlightColor1);
			ImGui::SetNextItemWidth(windowData.m_SecondaryTextSmallWidth);
			if (ImGui::DragInt(("##" + std::to_string(GetNextChildID())).c_str(), &(m_CurrentInteger), (float)m_ScrollSpeed,
				m_Bounds[0], m_Bounds[1],
				"%d", ImGuiSliderFlags_AlwaysClamp))
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
				ImGui::TextColored(configColors.m_HighlightColor1, "X-Value");
				ImGui::EndTooltip();
			}
			ImGui::PopStyleVar();

		}
		else
		{
			float yPosition = ImGui::GetCursorPosY();
			ImGui::SetCursorPos({ windowData.m_SecondaryTextPosOne, yPosition });
			ImGui::PushStyleColor(ImGuiCol_Text, configColors.m_SecondaryTextColor);
			int32_t integerPosition = ImGui::FindPositionAfterLength(std::to_string(m_CurrentInteger).c_str(), windowData.m_SecondaryTextSmallWidth);
			EditorUIContext::RenderTruncatedText(std::to_string(m_CurrentInteger),
				integerPosition == -1 ? std::numeric_limits<int32_t>::max() : integerPosition);
			ImGui::PopStyleColor();
		}

		ImGui::SameLine();
		EditorUIContext::RenderInlineButton(GetNextChildID(), [&]()
		{
			Utility::Operations::ToggleBoolean(m_Editing);
		},
		EditorUIContext::m_UIPresets.m_SmallEditButton,
		m_Editing, m_Editing ? configColors.m_PrimaryTextColor : configColors.m_DisabledColor);
	}
	void EditIVec2Widget::RenderIVec2()
	{
		ResetChildID();

		// Draw backgrounds
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec2 screenPosition = ImGui::GetCursorScreenPos();
		draw_list->AddRectFilled(ImVec2(EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosOne - 5.0f, screenPosition.y),
			ImVec2(EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosOne + EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth, screenPosition.y + EditorUIContext::m_ConfigSpacing.m_TextBackgroundHeight),
			ImColor(EditorUIContext::m_ActiveWindowData.m_ActiveBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);
		draw_list->AddRectFilled(ImVec2(EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosTwo - 5.0f, screenPosition.y),
			ImVec2(EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosTwo + EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth, screenPosition.y + EditorUIContext::m_ConfigSpacing.m_TextBackgroundHeight),
			ImColor(EditorUIContext::m_ActiveWindowData.m_ActiveBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);

		// Display Item
		if (m_Flags & EditIVec2_Indented)
		{
			ImGui::SetCursorPosX(EditorUIContext::m_ConfigSpacing.m_PrimaryTextIndent);
		}
		ImGui::PushStyleColor(ImGuiCol_Text, EditorUIContext::m_ConfigColors.m_PrimaryTextColor);
		int32_t labelPosition = ImGui::FindPositionAfterLength(m_Label.CString(),
			m_Flags & EditIVec2_Indented ? EditorUIContext::m_ActiveWindowData.m_PrimaryTextIndentedWidth : EditorUIContext::m_ActiveWindowData.m_PrimaryTextWidth);
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
			if (ImGui::DragInt(("##" + std::to_string(GetNextChildID())).c_str(), &(m_CurrentIVec2.x), (float)m_ScrollSpeed,
				m_Bounds[0], m_Bounds[1],
				"%d", ImGuiSliderFlags_AlwaysClamp))
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
			if (ImGui::DragInt(("##" + std::to_string(GetNextChildID())).c_str(), &(m_CurrentIVec2.y), (float)m_ScrollSpeed,
				m_Bounds[0], m_Bounds[1],
				"%d", ImGuiSliderFlags_AlwaysClamp))
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
			int32_t integerPosition = ImGui::FindPositionAfterLength(
				std::to_string(m_CurrentIVec2.x).c_str(), EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth);
			EditorUIContext::RenderTruncatedText(std::to_string(m_CurrentIVec2.x),
				integerPosition == -1 ? std::numeric_limits<int32_t>::max() : integerPosition);
			ImGui::SetCursorPos({ EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosTwo, yPosition });
			integerPosition = ImGui::FindPositionAfterLength(
				std::to_string(m_CurrentIVec2.y).c_str(), EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth);
			EditorUIContext::RenderTruncatedText(std::to_string(m_CurrentIVec2.y),
				integerPosition == -1 ? std::numeric_limits<int32_t>::max() : integerPosition);
			ImGui::PopStyleColor();
		}

		ImGui::SameLine();
		EditorUIContext::RenderInlineButton(GetNextChildID(), [&]()
			{
				Utility::Operations::ToggleBoolean(m_Editing);
			},
			EditorUIContext::m_UIPresets.m_SmallEditButton,
			m_Editing, m_Editing ? EditorUIContext::m_ConfigColors.m_PrimaryTextColor : EditorUIContext::m_ConfigColors.m_DisabledColor);
	}
	void EditIVec3Widget::RenderIVec3()
	{
		ResetChildID();

		// Draw backgrounds
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec2 screenPosition = ImGui::GetCursorScreenPos();
		draw_list->AddRectFilled(ImVec2(EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosOne - 5.0f, screenPosition.y),
			ImVec2(EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosOne + EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth, screenPosition.y + EditorUIContext::m_ConfigSpacing.m_TextBackgroundHeight),
			ImColor(EditorUIContext::m_ActiveWindowData.m_ActiveBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);
		draw_list->AddRectFilled(ImVec2(EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosTwo - 5.0f, screenPosition.y),
			ImVec2(EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosTwo + EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth, screenPosition.y + EditorUIContext::m_ConfigSpacing.m_TextBackgroundHeight),
			ImColor(EditorUIContext::m_ActiveWindowData.m_ActiveBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);
		draw_list->AddRectFilled(ImVec2(EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosThree - 5.0f, screenPosition.y),
			ImVec2(EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosThree + EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth, screenPosition.y + EditorUIContext::m_ConfigSpacing.m_TextBackgroundHeight),
			ImColor(EditorUIContext::m_ActiveWindowData.m_ActiveBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);

		// Display Item
		if (m_Flags & EditIVec3_Indented)
		{
			ImGui::SetCursorPosX(EditorUIContext::m_ConfigSpacing.m_PrimaryTextIndent);
		}
		ImGui::PushStyleColor(ImGuiCol_Text, EditorUIContext::m_ConfigColors.m_PrimaryTextColor);
		int32_t labelPosition = ImGui::FindPositionAfterLength(m_Label.CString(),
			m_Flags & EditIVec3_Indented ? EditorUIContext::m_ActiveWindowData.m_PrimaryTextIndentedWidth : EditorUIContext::m_ActiveWindowData.m_PrimaryTextWidth);
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
			if (ImGui::DragInt(("##" + std::to_string(GetNextChildID())).c_str(), &(m_CurrentIVec3.x), (float)m_ScrollSpeed,
				m_Bounds[0], m_Bounds[1],
				"%d", ImGuiSliderFlags_AlwaysClamp))
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
			if (ImGui::DragInt(("##" + std::to_string(GetNextChildID())).c_str(), &(m_CurrentIVec3.y), (float)m_ScrollSpeed,
				m_Bounds[0], m_Bounds[1],
				"%d", ImGuiSliderFlags_AlwaysClamp))
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
			if (ImGui::DragInt(("##" + std::to_string(GetNextChildID())).c_str(), &(m_CurrentIVec3.z), (float)m_ScrollSpeed,
				m_Bounds[0], m_Bounds[1],
				"%d", ImGuiSliderFlags_AlwaysClamp))
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
				ImGui::TextColored(EditorUIContext::m_ConfigColors.m_HighlightColor2, "Z-Value");
				ImGui::EndTooltip();
			}
			ImGui::PopStyleVar();

		}
		else
		{
			float yPosition = ImGui::GetCursorPosY();
			ImGui::SetCursorPos({ EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosOne, yPosition });
			ImGui::PushStyleColor(ImGuiCol_Text, EditorUIContext::m_ConfigColors.m_SecondaryTextColor);
			int32_t integerPosition = ImGui::FindPositionAfterLength(
				std::to_string(m_CurrentIVec3.x).c_str(), EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth);
			EditorUIContext::RenderTruncatedText(std::to_string(m_CurrentIVec3.x),
				integerPosition == -1 ? std::numeric_limits<int32_t>::max() : integerPosition);
			ImGui::SetCursorPos({ EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosTwo, yPosition });
			integerPosition = ImGui::FindPositionAfterLength(
				std::to_string(m_CurrentIVec3.y).c_str(), EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth);
			EditorUIContext::RenderTruncatedText(std::to_string(m_CurrentIVec3.y),
				integerPosition == -1 ? std::numeric_limits<int32_t>::max() : integerPosition);
			ImGui::SetCursorPos({ EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosThree, yPosition });
			integerPosition = ImGui::FindPositionAfterLength(
				std::to_string(m_CurrentIVec3.z).c_str(), EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth);
			EditorUIContext::RenderTruncatedText(std::to_string(m_CurrentIVec3.z),
				integerPosition == -1 ? std::numeric_limits<int32_t>::max() : integerPosition);
			ImGui::PopStyleColor();
		}

		ImGui::SameLine();
		EditorUIContext::RenderInlineButton(GetNextChildID(), [&]()
			{
				Utility::Operations::ToggleBoolean(m_Editing);
			},
			EditorUIContext::m_UIPresets.m_SmallEditButton,
			m_Editing, m_Editing ? EditorUIContext::m_ConfigColors.m_PrimaryTextColor : EditorUIContext::m_ConfigColors.m_DisabledColor);
	}
	void EditIVec4Widget::RenderIVec4()
	{
		ResetChildID();

		// Draw backgrounds
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec2 screenPosition = ImGui::GetCursorScreenPos();
		draw_list->AddRectFilled(ImVec2(EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosOne - 5.0f, screenPosition.y),
			ImVec2(EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosOne + EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth, screenPosition.y + EditorUIContext::m_ConfigSpacing.m_TextBackgroundHeight),
			ImColor(EditorUIContext::m_ActiveWindowData.m_ActiveBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);
		draw_list->AddRectFilled(ImVec2(EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosTwo - 5.0f, screenPosition.y),
			ImVec2(EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosTwo + EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth, screenPosition.y + EditorUIContext::m_ConfigSpacing.m_TextBackgroundHeight),
			ImColor(EditorUIContext::m_ActiveWindowData.m_ActiveBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);
		draw_list->AddRectFilled(ImVec2(EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosThree - 5.0f, screenPosition.y),
			ImVec2(EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosThree + EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth, screenPosition.y + EditorUIContext::m_ConfigSpacing.m_TextBackgroundHeight),
			ImColor(EditorUIContext::m_ActiveWindowData.m_ActiveBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);
		draw_list->AddRectFilled(ImVec2(EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosFour - 5.0f, screenPosition.y),
			ImVec2(EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosFour + EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth, screenPosition.y + EditorUIContext::m_ConfigSpacing.m_TextBackgroundHeight),
			ImColor(EditorUIContext::m_ActiveWindowData.m_ActiveBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);

		// Display Item
		if (m_Flags & EditIVec4_Indented)
		{
			ImGui::SetCursorPosX(EditorUIContext::m_ConfigSpacing.m_PrimaryTextIndent);
		}
		ImGui::PushStyleColor(ImGuiCol_Text, EditorUIContext::m_ConfigColors.m_PrimaryTextColor);
		int32_t labelPosition = ImGui::FindPositionAfterLength(m_Label.CString(),
			m_Flags & EditIVec4_Indented ? EditorUIContext::m_ActiveWindowData.m_PrimaryTextIndentedWidth : EditorUIContext::m_ActiveWindowData.m_PrimaryTextWidth);
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
			if (ImGui::DragInt(("##" + std::to_string(GetNextChildID())).c_str(), &(m_CurrentIVec4.x), (float)m_ScrollSpeed,
				m_Bounds[0], m_Bounds[1],
				"%d", ImGuiSliderFlags_AlwaysClamp))
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
			if (ImGui::DragInt(("##" + std::to_string(GetNextChildID())).c_str(), &(m_CurrentIVec4.y), (float)m_ScrollSpeed,
				m_Bounds[0], m_Bounds[1],
				"%d", ImGuiSliderFlags_AlwaysClamp))
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
			if (ImGui::DragInt(("##" + std::to_string(GetNextChildID())).c_str(), &(m_CurrentIVec4.z), (float)m_ScrollSpeed,
				m_Bounds[0], m_Bounds[1],
				"%d", ImGuiSliderFlags_AlwaysClamp))
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
				ImGui::TextColored(EditorUIContext::m_ConfigColors.m_HighlightColor2, "Z-Value");
				ImGui::EndTooltip();
			}

			// w value
			ImGui::PushStyleColor(ImGuiCol_Text, EditorUIContext::m_ConfigColors.m_HighlightColor4);
			ImGui::SetCursorPos({ EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosFour, yPosition });
			ImGui::SetNextItemWidth(EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth);
			if (ImGui::DragInt(("##" + std::to_string(GetNextChildID())).c_str(), &(m_CurrentIVec4.w), (float)m_ScrollSpeed,
				m_Bounds[0], m_Bounds[1],
				"%d", ImGuiSliderFlags_AlwaysClamp))
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
				ImGui::TextColored(EditorUIContext::m_ConfigColors.m_HighlightColor2, "W-Value");
				ImGui::EndTooltip();
			}
			ImGui::PopStyleVar();
		}
		else
		{
			float yPosition = ImGui::GetCursorPosY();
			ImGui::SetCursorPos({ EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosOne, yPosition });
			ImGui::PushStyleColor(ImGuiCol_Text, EditorUIContext::m_ConfigColors.m_SecondaryTextColor);
			int32_t integerPosition = ImGui::FindPositionAfterLength(
				std::to_string(m_CurrentIVec4.x).c_str(), EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth);
			EditorUIContext::RenderTruncatedText(std::to_string(m_CurrentIVec4.x),
				integerPosition == -1 ? std::numeric_limits<int32_t>::max() : integerPosition);
			ImGui::SetCursorPos({ EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosTwo, yPosition });
			integerPosition = ImGui::FindPositionAfterLength(
				std::to_string(m_CurrentIVec4.y).c_str(), EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth);
			EditorUIContext::RenderTruncatedText(std::to_string(m_CurrentIVec4.y),
				integerPosition == -1 ? std::numeric_limits<int32_t>::max() : integerPosition);
			ImGui::SetCursorPos({ EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosThree, yPosition });
			integerPosition = ImGui::FindPositionAfterLength(
				std::to_string(m_CurrentIVec4.z).c_str(), EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth);
			EditorUIContext::RenderTruncatedText(std::to_string(m_CurrentIVec4.z),
				integerPosition == -1 ? std::numeric_limits<int32_t>::max() : integerPosition);
			ImGui::SetCursorPos({ EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosFour, yPosition });
			integerPosition = ImGui::FindPositionAfterLength(
				std::to_string(m_CurrentIVec4.w).c_str(), EditorUIContext::m_ActiveWindowData.m_SecondaryTextSmallWidth);
			EditorUIContext::RenderTruncatedText(std::to_string(m_CurrentIVec4.w),
				integerPosition == -1 ? std::numeric_limits<int32_t>::max() : integerPosition);
			ImGui::PopStyleColor();
		}

		ImGui::SameLine();
		EditorUIContext::RenderInlineButton(GetNextChildID(), [&]()
			{
				Utility::Operations::ToggleBoolean(m_Editing);
			},
			EditorUIContext::m_UIPresets.m_SmallEditButton,
			m_Editing, m_Editing ? EditorUIContext::m_ConfigColors.m_PrimaryTextColor : EditorUIContext::m_ConfigColors.m_DisabledColor);
	}
}