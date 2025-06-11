#include "kgpch.h"

#include "Modules/EditorUI/Widgets/EditorUINavigationHeaderWidget.h"
#include "Modules/EditorUI/EditorUIContext.h"

#include "Modules/Rendering/Texture.h"

namespace Kargono::EditorUI
{
	void NavigationHeaderWidget::RenderHeader()
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		if (!m_IsBackActive)
		{
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
		}

		// Draw icon for moving a directory back
		if (ImGui::ImageButton((ImTextureID)(uint64_t)EditorUIService::m_ContentBrowserIcons.m_Back->GetRendererID(),
			{ 24.0f, 24.0f }, { 0, 1 }, { 1, 0 },
			-1, ImVec4(0, 0, 0, 0),
			m_IsBackActive ? EditorUIService::m_ConfigColors.s_PrimaryTextColor : EditorUIService::m_ConfigColors.s_DisabledColor))
		{
			if (m_IsBackActive && m_OnNavigateBack)
			{
				m_OnNavigateBack();
			}
		}

		if (!m_IsBackActive)
		{
			ImGui::PopStyleColor(2);
		}
		// Handle back navigation's payload
		if (m_Flags & NavigationHeaderFlags::NavigationHeader_AllowDragDrop &&
			m_IsBackActive &&
			m_OnReceivePayloadBack &&
			ImGui::BeginDragDropTarget())
		{
			for (const char* payloadName : m_AcceptableOnReceivePayloads)
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payloadName))
				{
					m_OnReceivePayloadBack(payloadName, payload->Data, payload->DataSize);
					break;
				}
			}
			ImGui::EndDragDropTarget();
		}

		// Draw forward navigation icon
		ImGui::SameLine();
		if (!m_IsForwardActive)
		{
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
		}
		if (ImGui::ImageButton((ImTextureID)(uint64_t)EditorUIService::m_ContentBrowserIcons.m_Forward->GetRendererID(),
			{ 24.0f, 24.0f }, { 0, 1 }, { 1, 0 },
			-1, ImVec4(0, 0, 0, 0),
			m_IsForwardActive ? EditorUIService::m_ConfigColors.s_PrimaryTextColor : EditorUIService::m_ConfigColors.s_DisabledColor))
		{
			if (m_IsForwardActive && m_OnNavigateForward)
			{
				m_OnNavigateForward();
			}
		}
		if (!m_IsForwardActive)
		{
			ImGui::PopStyleColor(2);
		}
		if (m_Flags & NavigationHeaderFlags::NavigationHeader_AllowDragDrop &&
			m_IsForwardActive &&
			m_OnReceivePayloadForward &&
			ImGui::BeginDragDropTarget())
		{
			for (const char* payloadName : m_AcceptableOnReceivePayloads)
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payloadName))
				{
					m_OnReceivePayloadForward(payloadName, payload->Data, payload->DataSize);
					break;
				}
			}
			ImGui::EndDragDropTarget();
		}
		ImGui::PopStyleColor();

		ImGui::PushFont(EditorUIService::m_ConfigFonts.m_Title);
		ImGui::SameLine();
		ImGui::TextUnformatted(m_Label);
		ImGui::PopFont();

		ImGui::Separator();
		EditorUIService::Spacing(0.2f);
	}
}

