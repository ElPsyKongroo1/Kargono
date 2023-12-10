#include "kgpch.h"

#include "Kargono/UI/EditorUI.h"
#include "Kargono/Core/Application.h"
#include "Kargono/Input/InputMode.h"

#include <glm/glm.hpp>





namespace Kargono
{
	Ref<Texture2D> EditorUI::s_IconPlay{};
	Ref<Texture2D> EditorUI::s_IconPause{};
	Ref<Texture2D> EditorUI::s_IconStop{};
	Ref<Texture2D> EditorUI::s_IconStep{};
	Ref<Texture2D> EditorUI::s_IconSimulate{};
	Ref<Texture2D> EditorUI::s_IconAddItem{};
	Ref<Texture2D> EditorUI::s_IconSettings{};
	Ref<Texture2D> EditorUI::s_IconDelete{};


	//void EditorUI::SlotSelectorTable(const std::string& tableIdentifier, uint32_t numberOfColumns, const std::vector<std::string>& namesOfColumns)
	//{
	//	KG_CORE_ASSERT(namesOfColumns.size() == numberOfColumns, "Incorrect number of columns provided or number of names provided to SlotSelectorTable");
	//	// Delayed Update Variables. Update Data structure after it has been fully output.
	//	bool updateKeySlot = false;
	//	bool updateKey = false;
	//	bool deleteKeySlot = false;
	//	glm::ivec2 updateItemBuffer{};

	//	// Enable Delete Option
	//	static bool deleteMenuToggle = false;
	//	
	//	ImGui::SameLine();
	//	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.0f);
	//	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
	//	if (ImGui::ImageButton((ImTextureID)(uint64_t)EditorUI::s_IconSettings->GetRendererID(), ImVec2(17, 17), ImVec2{ 0, 1 }, ImVec2{ 1, 0 }, 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)))
	//	{
	//		ImGui::OpenPopup( (tableIdentifier + "Settings").c_str());
	//	}
	//	ImGui::PopStyleColor();

	//	if (ImGui::BeginPopup((tableIdentifier + "Settings").c_str()))
	//	{
	//		if (ImGui::Selectable("Add New Row"))
	//		{
	//			InputMode::AddKeyboardPollingSlot();
	//			ImGui::CloseCurrentPopup();
	//		}
	//		if (ImGui::Selectable("Toggle Delete", deleteMenuToggle))
	//		{
	//			deleteMenuToggle = deleteMenuToggle ? false : true; // Conditional Toggles Boolean
	//			ImGui::CloseCurrentPopup();
	//		}
	//		ImGui::EndPopup();
	//	}

	//	// Main Table that lists audio slots and their corresponding AudioComponents
	//	static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
	//	if (ImGui::BeginTable((tableIdentifier + "Table").c_str(), deleteMenuToggle ? 3 : numberOfColumns, flags))
	//	{
	//		for (uint32_t iterator{0}; iterator < numberOfColumns; iterator++)
	//		{
	//			ImGui::TableSetupColumn(namesOfColumns.at(iterator).c_str(), ImGuiTableColumnFlags_WidthStretch);
	//		}

	//		if (deleteMenuToggle) { ImGui::TableSetupColumn("##", ImGuiTableColumnFlags_WidthFixed, 20.0f); }
	//		ImGui::TableHeadersRow();

	//		for (auto& [key, value] : InputMode::GetKeyboardPolling())
	//		{

	//			for (uint32_t columnIterator{ 0 }; columnIterator < numberOfColumns; columnIterator++)
	//			{
	//				ImGui::TableNextRow();
	//				ImGui::TableSetColumnIndex(columnIterator);
	//				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x + 6.0f);
	//				ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 3.0f);
	//				if (ImGui::BeginCombo(("##" + tableIdentifier + namesOfColumns.at(columnIterator) + std::to_string(key)).c_str(), std::to_string(key).c_str(), ImGuiComboFlags_NoArrowButton))
	//				{
	//					for (uint16_t iterator{ 0 }; iterator < 50; iterator++)
	//					{
	//						if (ImGui::Selectable(std::to_string(iterator).c_str()))
	//						{
	//							updateKeySlot = true;
	//							updateItemBuffer = { key, iterator };
	//						}
	//					}
	//					ImGui::EndCombo();
	//				}
	//			}

	//			if (deleteMenuToggle)
	//			{
	//				ImGui::SameLine();
	//				ImGui::TableSetColumnIndex(2);
	//				//ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x + 6.0f);
	//				ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 3.0f);
	//				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
	//				if (ImGui::ImageButton((tableIdentifier + "Delete" + std::to_string(key)).c_str(), (ImTextureID)(uint64_t)EditorUI::s_IconDelete->GetRendererID(),
	//					ImVec2(17.0f, 17.0f), ImVec2{ 0, 1 }, ImVec2{ 1, 0 }, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)))
	//				{
	//					deleteKeySlot = true;
	//					updateItemBuffer.x = key;
	//				}
	//				ImGui::PopStyleColor();
	//			}

	//		}
	//		ImGui::EndTable();
	//	}

	//	if (updateKeySlot) { InputMode::UpdateKeyboardPollingSlot(updateItemBuffer.x, updateItemBuffer.y); }
	//	if (updateKey) { InputMode::UpdateKeyboardPollingKey(updateItemBuffer.x, updateItemBuffer.y); }
	//	if (deleteKeySlot) { InputMode::DeleteKeyboardPollingSlot(updateItemBuffer.x); }
	//}

	void EditorUI::Init()
	{
		s_IconPlay = Texture2D::CreateEditorTexture((Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/play_icon.png").string());
		s_IconPause = Texture2D::CreateEditorTexture((Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/pause_icon.png").string());
		s_IconSimulate = Texture2D::CreateEditorTexture((Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/simulate_icon.png").string());
		s_IconStop = Texture2D::CreateEditorTexture((Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/stop_icon.png").string());
		s_IconStep = Texture2D::CreateEditorTexture((Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/step_icon.png").string());
		s_IconAddItem = Texture2D::CreateEditorTexture((Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/add_item.png").string());
		s_IconSettings = Texture2D::CreateEditorTexture((Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/settings_icon.png").string());
		s_IconDelete = Texture2D::CreateEditorTexture((Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/delete_icon.png").string());
	}
}
