#include "kgpch.h"

#include "Modules/EditorUI/Widgets/EditorUIPlotWidget.h"
#include "Modules/EditorUI/EditorUI.h"

#include "Modules/EditorUI/ExternalAPI/ImPlotAPI.h"

namespace Kargono::EditorUI
{
	void PlotWidget::RenderPlot()
	{
		KG_ASSERT(m_BufferSize > 0);

		// Local Variables
		FixedString<16> id{ "##" };
		id.AppendInteger(m_WidgetID);

		constexpr float k_PlotHeight{ 140.0f };

		// Draw background
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec2 screenPosition = ImGui::GetCursorScreenPos();
		draw_list->AddRectFilled(ImVec2(EditorUIService::s_WindowPosition.x + EditorUIService::s_SecondaryTextPosOne - 5.0f, screenPosition.y),
			ImVec2(EditorUIService::s_WindowPosition.x + EditorUIService::s_SecondaryTextPosOne + EditorUIService::s_SecondaryTextLargeWidth, screenPosition.y + k_PlotHeight),
			ImColor(EditorUIService::s_ActiveBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);

		if (m_Flags & Plot_Indented)
		{
			ImGui::SetCursorPosX(EditorUIService::s_TextLeftIndentOffset);
		}
		// Display Primary Label
		ImGui::PushStyleColor(ImGuiCol_Text, EditorUIService::s_PrimaryTextColor);
		int32_t labelPosition = ImGui::FindPositionAfterLength(m_Label.CString(),
			m_Flags & Plot_Indented ? EditorUIService::s_PrimaryTextIndentedWidth : EditorUIService::s_PrimaryTextWidth);
		EditorUIService::TruncateText(m_Label.CString(), labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);
		ImGui::PopStyleColor();

		// Shift button to secondary text position one
		ImGui::SameLine(EditorUIService::s_SecondaryTextPosOne - 2.5f);

		ImPlotFlags flags = ImPlotFlags_NoMouseText | ImPlotFlags_NoLegend | ImPlotFlags_NoInputs;
		ImPlotAxisFlags axisFlags = ImPlotAxisFlags_NoTickLabels;

		if (ImPlot::BeginPlot(id, ImVec2(EditorUIService::s_SecondaryTextLargeWidth, k_PlotHeight), flags))
		{
			ImPlot::SetupAxes("##", m_YAxisLabel.CString(), axisFlags, 0);
			ImPlot::SetupAxesLimits((double)m_Offset - (double)m_BufferSize, (double)m_Offset - 1.0, 0, m_MaxYVal, ImPlotCond_Always);
			ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, 0.25f);
			ImPlot::PlotShaded("##", &m_XValues[0], &m_YValues[0], (int)m_BufferSize, 0, 0, (int)m_Offset);
			ImPlot::PopStyleVar();

			ImPlot::PlotLine("##", &m_XValues[0], &m_YValues[0], (int)m_BufferSize, 0, (int)m_Offset);

			ImPlot::EndPlot();
		}
	}
	void PlotWidget::SetBufferSize(size_t newSize)
	{
		// Update the buffer size
		m_BufferSize = newSize;

		// Update the X/Y container values
		m_XValues.resize(newSize);
		m_YValues.resize(newSize);

		// Clear the buffer
		Clear();
	}
	void PlotWidget::SetYAxisLabel(std::string_view text)
	{
		if (text.size() == 0)
		{
			m_YAxisLabel = "##";
			return;
		}

		m_YAxisLabel = text;
	}
	void PlotWidget::AddValue(float yValue)
	{
		// Add the coordinate
		m_XValues[m_Offset % m_BufferSize] = (float)m_Offset;
		m_YValues[m_Offset % m_BufferSize] = yValue;

		// Update the offset index
		m_Offset++;
	}
	void PlotWidget::UpdateValue(float yValue, size_t offset)
	{
		KG_ASSERT(offset < m_BufferSize);

		m_YValues[(m_Offset - offset - 1) % m_BufferSize] = yValue;
	}
	void PlotWidget::Clear()
	{
		// Clear all values in buffer
		for (size_t i{ 0 }; i < m_BufferSize; i++)
		{
			m_XValues[i] = 0.0f;
			m_YValues[i] = 0.0f;
		}

		// Clear the offset
		m_Offset = m_BufferSize;
	}
}