#include "kgpch.h"

#include "Modules/EditorUI/Widgets/EditorUIGridWidget.h"
#include "Modules/EditorUI/EditorUIContext.h"

#include "Modules/Rendering/Texture.h"

namespace Kargono::EditorUI
{
	void Kargono::EditorUI::GridWidget::RenderGrid()
	{
		ResetChildID();

		// Calculate grid cell count using provided spec sizes
		float cellSize = m_CellIconSize + m_CellPadding;
		float panelWidth = ImGui::GetContentRegionAvail().x;
		int32_t columnCount = (int32_t)(panelWidth / cellSize);
		columnCount = columnCount > 0 ? columnCount : 1;

		// Start drawing columns
		ImGui::Columns(columnCount, m_WidgetIDString.CString(), false);
		ImGui::PushStyleColor(ImGuiCol_Button, k_PureEmpty);
		for (GridEntry& currentEntry : m_Entries)
		{
			// Check if entry is selected
			bool entryIsSelected = currentEntry.m_EntryID == m_SelectedEntry;

			// Get entry archetype and grid element ID
			FixedString<16> entryID{ m_WidgetIDString };
			size_t widgetCount{ 0 };
			GridEntryArchetype* entryArchetype = &(m_EntryArchetypes.at(currentEntry.m_ArchetypeID));
			entryID.AppendInteger(++widgetCount);
			KG_ASSERT(entryArchetype);

			// Display grid icon
			ImGui::PushID(entryID.CString());
			if (ImGui::ImageButton((ImTextureID)(uint64_t)entryArchetype->m_Icon->GetRendererID(), { m_CellIconSize, m_CellIconSize },
				{ 0, 1 }, { 1, 0 },
				-1, entryIsSelected ? EditorUIContext::m_ConfigColors.m_ActiveColor : k_PureEmpty,
				entryArchetype->m_IconColor))
			{
				// Handle on left-click
				if (entryArchetype->m_OnLeftClick)
				{
					entryArchetype->m_OnLeftClick(currentEntry);
				}
				m_SelectedEntry = currentEntry.m_EntryID;
			}

			// Handle double left clicks
			if (entryArchetype->m_OnDoubleLeftClick &&
				ImGui::IsItemHovered() &&
				ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				entryArchetype->m_OnDoubleLeftClick(currentEntry);
			}

			// Handle right clicks
			if (entryArchetype->m_OnRightClick &&
				ImGui::IsItemHovered() &&
				ImGui::IsMouseClicked(ImGuiMouseButton_Right)
				)
			{
				entryArchetype->m_OnRightClick(currentEntry);
			}

			// Handle payloads
			if (m_Flags & GridFlags::Grid_AllowDragDrop)
			{
				// Handle create payload
				if (entryArchetype->m_OnCreatePayload && ImGui::BeginDragDropSource())
				{
					DragDropPayload newPayload;
					entryArchetype->m_OnCreatePayload(currentEntry, newPayload);
					ImGui::SetDragDropPayload(newPayload.m_Label, newPayload.m_DataPointer, newPayload.m_DataSize, ImGuiCond_Once);
					ImGui::EndDragDropSource();
				}

				// Handle receive payload
				if (entryArchetype->m_OnReceivePayload && ImGui::BeginDragDropTarget())
				{
					for (const char* payloadName : entryArchetype->m_AcceptableOnReceivePayloads)
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payloadName))
						{
							entryArchetype->m_OnReceivePayload(currentEntry, payloadName, payload->Data, payload->DataSize);
							break;
						}
					}
					ImGui::EndDragDropTarget();
				}
			}

			// Draw label for each cell
			ImGui::TextWrapped(currentEntry.m_Label.CString());

			// Reset cell data for next call
			ImGui::NextColumn();
			ImGui::PopID();
		}

		// End drawing columns
		ImGui::PopStyleColor();
		ImGui::Columns(1);
	}
	bool GridWidget::AddEntry(GridEntry& newEntry)
	{
		// Ensure a valid archetype id is provided
		if (newEntry.m_ArchetypeID == k_InvalidArchetypeID)
		{
			return false;
		}

		// Ensure UUID is unique inside entry
		while (!ValidateEntryID(newEntry.m_EntryID))
		{
			newEntry.m_EntryID = {};
		}

		// Insert new entry
		m_Entries.push_back(newEntry);
		return true;
	}
	bool GridWidget::AddEntry(GridEntry&& newEntry)
	{
		// Ensure a valid archetype id is provided
		if (newEntry.m_ArchetypeID == k_InvalidArchetypeID)
		{
			return false;
		}

		// Ensure UUID is unique inside entry
		while (!ValidateEntryID(newEntry.m_EntryID))
		{
			newEntry.m_EntryID = {};
		}

		// Insert new entry
		m_Entries.push_back(std::move(newEntry));
		return true;
	}
	void GridWidget::ClearEntries()
	{
		m_Entries.clear();
		ClearSelectedEntry();
	}
	void GridWidget::ClearSelectedEntry()
	{
		m_SelectedEntry = k_EmptyUUID;
	}
	bool GridWidget::AddEntryArchetype(ArchetypeID key, const GridEntryArchetype& newArchetype)
	{
		auto [iterator, success] = m_EntryArchetypes.insert({ key, newArchetype });
		return success;
	}
	bool GridWidget::AddEntryArchetype(ArchetypeID key, GridEntryArchetype&& newArchetype)
	{
		auto [iterator, success] = m_EntryArchetypes.insert({ key, std::move(newArchetype) });
		return success;
	}
	bool GridWidget::ValidateEntryID(UUID queryID)
	{
		// Ensure empty id is not provided
		if (queryID == k_EmptyUUID)
		{
			return false;
		}

		// Ensure that no match id is found in internal entries list
		for (GridEntry& entry : m_Entries)
		{
			if (entry.m_EntryID == queryID)
			{
				return false;
			}
		}

		// Return true if no duplicate is found
		return true;
	}
}

