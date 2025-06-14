#include "kgpch.h"

#include "Modules/EditorUI/Widgets/EditorUITooltipWidget.h"
#include "Modules/EditorUI/EditorUIContext.h"

namespace Kargono::EditorUI
{
	void TooltipWidget::RenderTooltipEntryList(std::vector<TooltipEntry>& entryList)
	{
		// Process each entry in current list
		for (TooltipEntry& currentEntry : entryList)
		{
			// Ignore entries that are not visible
			if (!currentEntry.m_IsVisible)
			{
				continue;
			}

			// Handle case where current entry acts as a menu
			if (std::vector<TooltipEntry>* subEntryList = std::get_if<std::vector<TooltipEntry>>(&currentEntry.m_EntryData))
			{
				// Create menu option
				if (ImGui::BeginMenu(currentEntry.m_Label))
				{
					// Handle sub entries using recursive call
					RenderTooltipEntryList(*subEntryList);

					ImGui::EndMenu();
				}


			}
			// Handle case where current entry is a terminal node
			else if (std::function<void(TooltipEntry&)>* terminalNode = std::get_if<std::function<void(TooltipEntry&)>>(&currentEntry.m_EntryData))
			{
				// Display current node
				if (ImGui::MenuItem(currentEntry.m_Label))
				{
					// Call function pointer if valid
					std::function<void(TooltipEntry&)>& functionPointerRef = *terminalNode;
					if (functionPointerRef)
					{
						functionPointerRef(currentEntry);
					}
				}
			}
			else if (TooltipSeperatorData* tooltipSeperatorData = std::get_if<TooltipSeperatorData>(&currentEntry.m_EntryData))
			{
				ImGui::Separator(1.0f, tooltipSeperatorData->m_SeperatorColor);
			}
			else
			{
				KG_ERROR("Invalid variant type of entry data provided for tooltip entry");
			}

		}
	}

	void TooltipWidget::RenderTooltip()
	{
		// Handle turning on the tooltip
		if (m_TooltipActive)
		{
			// Only open tooltip if menu items are present
			if (m_Entries.size() != 0)
			{
				ImGui::OpenPopup(m_WidgetIDString);
			}
			m_TooltipActive = false;
		}

		// Draw tooltip if active
		if (ImGui::BeginPopup(m_WidgetIDString))
		{
			// Recursively handle tooltip items and sub-menus
			RenderTooltipEntryList(m_Entries);
			ImGui::EndPopup();
		}
	}


	bool TooltipWidget::ValidateEntryID(UUID newEntryID)
	{
		// Ensure empty id is not provided
		if (newEntryID == k_EmptyUUID)
		{
			return false;
		}

		return ValidateEntryIDRecursive(m_Entries, newEntryID);
	}

	bool TooltipWidget::ValidateEntryIDRecursive(std::vector<TooltipEntry>& entries, UUID queryID)
	{
		// Ensure that no match id is found in internal entries list
		for (TooltipEntry& currentEntry : entries)
		{
			// Check each entry
			if (currentEntry.m_EntryID == queryID)
			{
				return false;
			}

			// Check for recursive entries inside menu
			if (std::vector<TooltipEntry>* subEntriesListRef = std::get_if<std::vector<TooltipEntry>>(&currentEntry.m_EntryData))
			{
				std::vector<TooltipEntry>& subEntriesList = *subEntriesListRef;
				bool success = ValidateEntryIDRecursive(subEntriesList, queryID);
				if (!success)
				{
					return false;
				}
			}
		}

		// Return true, if no duplicates are found
		return true;
	}

	void TooltipWidget::AddSeperator(ImVec4 seperatorColor)
	{
		TooltipSeperatorData seperatorData{ seperatorColor };
		TooltipEntry newSeperatorEntry;
		newSeperatorEntry.m_EntryData = seperatorData;

		// Ensure UUID is unique inside entry
		while (!ValidateEntryID(newSeperatorEntry.m_EntryID))
		{
			newSeperatorEntry.m_EntryID = {};
		}

		// Add new entry if valid
		m_Entries.push_back(newSeperatorEntry);
	}

	UUID TooltipWidget::AddTooltipEntry(TooltipEntry& newEntry)
	{
		// Ensure valid label is provided
		if (newEntry.m_Label.IsEmpty())
		{
			return k_EmptyUUID;
		}

		// Ensure UUID is unique inside entry
		while (!ValidateEntryID(newEntry.m_EntryID))
		{
			newEntry.m_EntryID = {};
		}

		// Add new entry if valid
		m_Entries.push_back(newEntry);
		return newEntry.m_EntryID;
	}

	UUID TooltipWidget::AddTooltipEntry(TooltipEntry&& newEntry)
	{
		// Ensure valid label is provided
		if (newEntry.m_Label.IsEmpty())
		{
			return k_EmptyUUID;
		}

		// Ensure UUID is unique inside entry
		while (!ValidateEntryID(newEntry.m_EntryID))
		{
			newEntry.m_EntryID = {};
		}

		// Add new entry if valid
		UUID cacheID = newEntry.m_EntryID;
		m_Entries.push_back(std::move(newEntry));
		return cacheID;
	}

	bool TooltipWidget::SetIsVisible(UUID entry, bool isVisible)
	{
		// Ensure empty id is not provided
		if (entry == k_EmptyUUID)
		{
			return false;
		}

		return SetIsVisibleRecursive(m_Entries, entry, isVisible);
	}

	bool TooltipWidget::SetIsVisibleRecursive(std::vector<TooltipEntry>& entries, UUID queryID, bool isVisible)
	{
		// Ensure that no match id is found in internal entries list
		for (TooltipEntry& currentEntry : entries)
		{
			// Check each entry
			if (currentEntry.m_EntryID == queryID)
			{
				// Handle setting visibility
				currentEntry.m_IsVisible = isVisible;
				return true;
			}

			// Check for recursive entries inside menu
			if (std::vector<TooltipEntry>* subEntriesListRef = std::get_if<std::vector<TooltipEntry>>(&currentEntry.m_EntryData))
			{
				std::vector<TooltipEntry>& subEntriesList = *subEntriesListRef;
				bool success = SetIsVisibleRecursive(subEntriesList, queryID, isVisible);
				if (success)
				{
					return true;
				}
			}
		}

		// Return true, if no duplicates are found
		return false;
	}

	bool TooltipWidget::SetAllChildrenIsVisible(UUID entry, bool isVisible)
	{
		// Ensure empty id is not provided
		if (entry == k_EmptyUUID)
		{
			return false;
		}

		return SetAllChildrenIsVisibleRecursive(m_Entries, entry, isVisible);
	}

	void TooltipWidget::ClearEntries()
	{
		m_Entries.clear();
	}

	bool TooltipWidget::SetAllChildrenIsVisibleRecursive(std::vector<TooltipEntry>& entries, UUID queryID, bool isVisible)
	{
		// Ensure that no match id is found in internal entries list
		for (TooltipEntry& currentEntry : entries)
		{
			// Check each entry
			if (currentEntry.m_EntryID == queryID)
			{
				// Handle setting visibility
				if (std::vector<TooltipEntry>* subEntriesListRef = std::get_if<std::vector<TooltipEntry>>(&currentEntry.m_EntryData))
				{
					std::vector<TooltipEntry>& subEntriesList = *subEntriesListRef;
					for (TooltipEntry& subEntry : subEntriesList)
					{
						subEntry.m_IsVisible = isVisible;
					}
					return true;
				}
				// Found entry, however, entry is not a menu.
				else
				{
					return false;
				}
			}

			// Check for recursive entries inside menu
			if (std::vector<TooltipEntry>* subEntriesListRef = std::get_if<std::vector<TooltipEntry>>(&currentEntry.m_EntryData))
			{
				std::vector<TooltipEntry>& subEntriesList = *subEntriesListRef;
				bool success = SetAllChildrenIsVisibleRecursive(subEntriesList, queryID, isVisible);
				if (success)
				{
					return true;
				}
			}
		}

		// Return true, if no duplicates are found
		return false;
	}
}