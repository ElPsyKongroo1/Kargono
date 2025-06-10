#include "kgpch.h"

#include "Modules/EditorUI/Widgets/EditorUITreeWidget.h"
#include "Modules/EditorUI/EditorUI.h"

#include "Modules/EditorUI/ExternalAPI/ImGuiBackendAPI.h"

namespace Kargono::EditorUI
{
	void TreeWidget::DrawEntryList(std::vector<TreeEntry>& entries, uint32_t& widgetCount, TreePath& currentPath, ImVec2 rootPosition)
	{
		// Get initial positions and common resources
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec2 screenPosition{};
		PathDepth depth = currentPath.GetDepth();
		BranchIndex entryIndex = 0;
		for (EditorUI::TreeEntry& treeEntry : entries)
		{
			currentPath.PushBackNode(entryIndex);
			// Set x-position based on current tree depth
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (depth * 30.0f));
			screenPosition = ImGui::GetCursorScreenPos();
			ImVec2 buttonDimensions{ ImGui::CalcTextSize(treeEntry.m_Label.c_str()).x + 34.0f, EditorUIService::s_TextBackgroundHeight };

			// Create Invisible Button for Interation with current node
			if (ImGui::InvisibleButton(("##" + std::to_string(m_WidgetID + EditorUIService::WidgetIterator(widgetCount))).c_str(), buttonDimensions))
			{
				if (treeEntry.m_OnLeftClick)
				{
					treeEntry.m_OnLeftClick(treeEntry);
				}
				m_SelectedEntry = currentPath;
			}

			if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
			{
				if (treeEntry.m_OnRightClick)
				{
					treeEntry.m_OnRightClick(treeEntry);
				}
			}

			if (ImGui::IsItemHovered())
			{
				// Draw SelectedEntry background
				draw_list->AddRectFilled(screenPosition,
					ImVec2(screenPosition.x + buttonDimensions.x, screenPosition.y + buttonDimensions.y),
					ImColor(EditorUIService::s_HoveredColor), 4, ImDrawFlags_RoundCornersAll);

				if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && treeEntry.m_OnDoubleLeftClick)
				{
					treeEntry.m_OnDoubleLeftClick(treeEntry);
				}
			}

			// Display Selected Background
			ImGui::SetCursorScreenPos(screenPosition);
			if (m_SelectedEntry == currentPath)
			{
				// Draw SelectedEntry background
				draw_list->AddRectFilled(screenPosition,
					ImVec2(screenPosition.x + buttonDimensions.x, screenPosition.y + buttonDimensions.y),
					ImColor(EditorUIService::s_ActiveColor), 4, ImDrawFlags_RoundCornersAll);
				if (m_SelectionChanged)
				{
					ImGui::SetScrollHereY();
					m_SelectionChanged = false;
				}
			}

			// Display entry icon
			if (treeEntry.m_IconHandle)
			{
				EditorUIService::CreateImage(treeEntry.m_IconHandle, 14, EditorUIService::s_HighlightColor1);
				ImGui::SameLine();
			}

			// Display entry text
			ImGui::PushStyleColor(ImGuiCol_Text, EditorUIService::s_PrimaryTextColor);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 1.5f);
			ImGui::TextUnformatted(treeEntry.m_Label.c_str());
			ImGui::PopStyleColor();


			// Handle all sub-entries
			if (treeEntry.m_SubEntries.size() > 0)
			{
				// Draw expand icon
				ImGui::SameLine();
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2.5f);
				ImGui::PushStyleColor(ImGuiCol_Button, EditorUIService::s_PureEmpty);
				const Ref<Rendering::Texture2D> icon = m_ExpandedNodes.contains(currentPath) ? EditorUIService::s_IconDown : EditorUIService::s_IconRight;
				if (ImGui::ImageButtonEx(m_WidgetID + EditorUIService::WidgetIterator(widgetCount),
					(ImTextureID)(uint64_t)icon->GetRendererID(),
					ImVec2(13, 13), ImVec2{ 0, 1 }, ImVec2{ 1, 0 },
					EditorUIService::s_PureEmpty,
					m_ExpandedNodes.contains(currentPath) ? EditorUIService::s_HighlightColor1 : EditorUIService::s_DisabledColor, 0))
				{
					if (m_ExpandedNodes.contains(currentPath))
					{
						m_ExpandedNodes.erase(currentPath);
					}
					else
					{
						m_ExpandedNodes.insert(currentPath);
					}
				}
				ImGui::PopStyleColor();

				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					ImGui::TextColored(EditorUI::EditorUIService::s_HighlightColor1, m_ExpandedNodes.contains(currentPath) ? "Collapse" : "Expand");
					ImGui::EndTooltip();
				}

				// Draw all sub-entries
				if (m_ExpandedNodes.contains(currentPath))
				{
					DrawEntryList(treeEntry.m_SubEntries, widgetCount, currentPath, screenPosition);
				}
			}

			// Add horizontal line for each child node
			if (depth > 0)
			{
				draw_list->AddLine(ImVec2(rootPosition.x + 10.0f, screenPosition.y + 10.0f),
					ImVec2(screenPosition.x, screenPosition.y + 10.0f),
					ImColor(EditorUIService::s_PrimaryTextColor));
			}
			currentPath.PopBack();
			entryIndex++;
		}

		// Add vertical line for parent node
		if (depth > 0)
		{
			draw_list->AddLine(ImVec2(rootPosition.x + 10.0f, rootPosition.y + 21.0f),
				ImVec2(rootPosition.x + 10.0f, screenPosition.y + 10.0f),
				ImColor(EditorUIService::s_PrimaryTextColor));
		}
	}

	void TreeWidget::RenderTree()
	{
		uint32_t widgetCount{ 0 };
		TreePath treePath{};
		DrawEntryList(m_TreeEntries, widgetCount, treePath, {});
	}

	static bool RecursiveGetPathFromEntry(TreePath& outputPath, TreeEntry* entryQuery, const std::vector<TreeEntry>& entries)
	{
		BranchIndex entryIndex{ 0 };
		for (const EditorUI::TreeEntry& treeEntry : entries)
		{
			outputPath.PushBackNode(entryIndex);
			if (entryQuery == &treeEntry)
			{
				return true;
			}

			if (treeEntry.m_SubEntries.size() > 0)
			{
				bool success = RecursiveGetPathFromEntry(outputPath, entryQuery, treeEntry.m_SubEntries);
				if (success)
				{
					return true;
				}
			}

			outputPath.PopBack();
			entryIndex++;
		}
		return false;
	}

	void TreeWidget::ClearExpandedNodes()
	{
		m_ExpandedNodes.clear();
	}

	void TreeWidget::ExpandFirstLayer()
	{
		for (TreeEntry& entry : m_TreeEntries)
		{
			m_ExpandedNodes.insert(GetPathFromEntryReference(&entry));
		}
	}

	void TreeWidget::ExpandNodePath(TreePath& path)
	{
		// Ensure the path is valid
		if (GetEntryFromPath(path))
		{
			// Add every node to the expanded path list
			TreePath expandingPath;
			for (BranchIndex index : path.GetPath())
			{
				// Add every entry 
				expandingPath.PushBackNode(index);
				m_ExpandedNodes.insert(expandingPath);
			}
		}
	}

	TreeEntry* TreeWidget::GetEntryFromPath(TreePath& path)
	{
		std::vector<TreeEntry>* currentEntryList = &m_TreeEntries;
		TreeEntry* currentEntry{ nullptr };
		for (BranchIndex index : path.GetPath())
		{
			if (index >= currentEntryList->size())
			{
				return {};
			}

			currentEntry = &currentEntryList->at(index);
			currentEntryList = &currentEntry->m_SubEntries;
		}

		return currentEntry;
	}

	TreePath TreeWidget::GetPathFromEntryReference(TreeEntry* entryQuery)
	{
		TreePath newPath{};
		if (RecursiveGetPathFromEntry(newPath, entryQuery, GetTreeEntries()))
		{
			return newPath;
		}

		KG_WARN("Could not locate provided entry query");
		return {};

	}

	void TreeWidget::MoveUp()
	{
		BranchIndex currentSelectedBack = m_SelectedEntry.GetBack();
		TreePath newPath = m_SelectedEntry;

		// Check if we can move up within current parent node
		if (currentSelectedBack <= 0)
		{
			// If we are in the top-most depth, we cannot move up further
			if (newPath.GetDepth() <= 1)
			{
				return;
			}
			// If we cannot move up in current parent node, find node above current parent node
			newPath.PopBack();
			currentSelectedBack = newPath.GetBack();
			// Check if a node exists above current parent node
			if (currentSelectedBack <= 0)
			{
				return;
			}
			// Set new path to last entry of new parent node
			currentSelectedBack--;
			newPath.SetBack(currentSelectedBack);
			TreeEntry* entry = GetEntryFromPath(newPath);
			TreePath newParentPath = newPath;
			newPath.PushBackNode((uint16_t)entry->m_SubEntries.size() - 1);
			// Exit if no final node could be found
			if (!GetEntryFromPath(newPath))
			{
				return;
			}
			// Set new SelectedEntry
			m_ExpandedNodes.insert(newParentPath);
			m_SelectedEntry = newPath;
			m_SelectionChanged = true;
			return;

		}
		// Move up within current parent node
		currentSelectedBack--;
		newPath.SetBack(currentSelectedBack);
		m_SelectedEntry = newPath;
		m_SelectionChanged = true;
	}

	void TreeWidget::MoveDown()
	{
		BranchIndex currentSelectedBack = m_SelectedEntry.GetBack();
		currentSelectedBack++;
		TreePath newPath = m_SelectedEntry;
		newPath.SetBack(currentSelectedBack);

		// Check if new path leads to valid entry
		if (!GetEntryFromPath(newPath))
		{
			// If we are in the top-most depth, we cannot move down further
			if (newPath.GetDepth() <= 1)
			{
				return;
			}
			// Attempt to move to the next node in the same depth of the tree
			newPath.PopBack();
			currentSelectedBack = newPath.GetBack();
			currentSelectedBack++;
			newPath.SetBack(currentSelectedBack);
			TreePath newParentPath = newPath;
			newPath.PushBackNode(0);
			// Exit if no node could be found
			if (!GetEntryFromPath(newPath))
			{
				return;
			}
			m_ExpandedNodes.insert(newParentPath);
		}
		m_SelectedEntry = newPath;
		m_SelectionChanged = true;
	}

	void TreeWidget::MoveLeft()
	{
		PathDepth currentDepth = m_SelectedEntry.GetDepth();

		// Exit if we are already at the top level of tree
		if (currentDepth <= 1)
		{
			if (m_ExpandedNodes.contains(m_SelectedEntry))
			{
				m_ExpandedNodes.erase(m_SelectedEntry);
			}
			return;
		}

		TreePath newPath = m_SelectedEntry;
		newPath.PopBack();
		m_SelectedEntry = newPath;
		m_SelectionChanged = true;
	}

	void TreeWidget::MoveRight()
	{
		TreeEntry* currentEntry = GetEntryFromPath(m_SelectedEntry);

		// Exit if current entry node does not contain any sub entries
		if (!currentEntry || currentEntry->m_SubEntries.size() == 0)
		{
			return;
		}
		if (!m_ExpandedNodes.contains(m_SelectedEntry))
		{
			m_ExpandedNodes.insert(m_SelectedEntry);
		}
		m_SelectedEntry.PushBackNode(0);
		m_SelectionChanged = true;
	}

	void TreeWidget::SelectFirstEntry()
	{
		if (m_TreeEntries.size() > 0)
		{
			m_SelectedEntry = GetPathFromEntryReference(&m_TreeEntries.at(0));
		}
	}

	bool TreeWidget::SelectEntry(TreePath& path)
	{
		// Check if entry exists
		if (TreeEntry* entry = GetEntryFromPath(path))
		{
			// Handle Left Click
			if (entry->m_OnLeftClick)
			{
				entry->m_OnLeftClick(*entry);
			}
			// Set SelectedEntry to provided path
			m_SelectedEntry = path;
			m_SelectionChanged = true;
			return true;
		}

		// Entry could not be found
		return false;

	}

	TreeEntry* TreeWidget::SearchFirstLayer(UUID handle)
	{
		for (TreeEntry& entry : m_TreeEntries)
		{
			// Found the entry
			if (entry.m_Handle == handle)
			{
				return &entry;
			}
		}
		return {};
	}

	TreeEntry* TreeWidget::SearchDepth(UUID queryHandle, size_t terminalDepth)
	{
		if (terminalDepth == 0)
		{
			for (TreeEntry& entry : m_TreeEntries)
			{
				if (entry.m_Handle == queryHandle)
				{
					return &entry;
				}
			}
			return nullptr;
		}

		TreeEntry* returnEntry{ nullptr };
		for (TreeEntry& entry : m_TreeEntries)
		{
			returnEntry = SearchDepthRecursive(entry, 1, terminalDepth, queryHandle);
			if (returnEntry)
			{
				return returnEntry;
			}
		}
		return nullptr;
	}

	TreeEntry* TreeWidget::SearchDepthRecursive(TreeEntry& currentEntry, size_t currentDepth, size_t terminalDepth, UUID queryHandle)
	{
		if (currentDepth == terminalDepth)
		{
			for (TreeEntry& subEntry : currentEntry.m_SubEntries)
			{
				if (subEntry.m_Handle == queryHandle)
				{
					return &subEntry;
				}
			}
			return nullptr;
		}

		if (currentDepth > terminalDepth)
		{
			return nullptr;
		}

		TreeEntry* returnedEntry{ nullptr };
		for (TreeEntry& subEntry : currentEntry.m_SubEntries)
		{
			returnedEntry = SearchDepthRecursive(subEntry, currentDepth + 1, terminalDepth, queryHandle);
			if (returnedEntry)
			{
				return returnedEntry;
			}
		}
		return nullptr;
	}

	void TreeWidget::SearchDepthRecursive(TreeEntry& currentEntry, size_t currentDepth, size_t terminalDepth,
		std::function<bool(TreeEntry& entry)> searchFunction, std::vector<TreePath>& allPaths)
	{
		if (currentDepth >= terminalDepth)
		{
			for (TreeEntry& subEntry : currentEntry.m_SubEntries)
			{
				if (searchFunction(subEntry))
				{
					allPaths.push_back(GetPathFromEntryReference(&subEntry));
				}
			}
			return;
		}

		for (TreeEntry& subEntry : currentEntry.m_SubEntries)
		{
			SearchDepthRecursive(subEntry, currentDepth + 1, terminalDepth, searchFunction, allPaths);
		}

	}

	std::vector<TreePath> TreeWidget::SearchDepth(std::function<bool(TreeEntry& entry)> searchFunction, size_t terminalDepth)
	{
		std::vector<TreePath> allPaths{};
		if (terminalDepth == 0)
		{
			for (TreeEntry& entry : m_TreeEntries)
			{
				if (searchFunction(entry))
				{
					allPaths.push_back(GetPathFromEntryReference(&entry));
				}
			}
			return allPaths;
		}

		for (TreeEntry& entry : m_TreeEntries)
		{
			SearchDepthRecursive(entry, 1, terminalDepth, searchFunction, allPaths);
		}
		return allPaths;
	}

	void TreeWidget::EditDepthRecursive(TreeEntry& currentEntry, size_t currentDepth, size_t terminalDepth, std::function<void(TreeEntry& entry)> editFunction)
	{
		if (currentDepth >= terminalDepth)
		{
			for (TreeEntry& subEntry : currentEntry.m_SubEntries)
			{
				editFunction(subEntry);
			}
			return;
		}

		for (TreeEntry& subEntry : currentEntry.m_SubEntries)
		{
			EditDepthRecursive(subEntry, currentDepth + 1, terminalDepth, editFunction);
		}
	}

	void TreeWidget::EditDepth(std::function<void(TreeEntry& entry)> editFunction, size_t terminalDepth)
	{
		std::vector<TreePath> allPaths{};
		if (terminalDepth == 0)
		{
			for (TreeEntry& entry : m_TreeEntries)
			{
				editFunction(entry);
			}
			return;
		}

		for (TreeEntry& entry : m_TreeEntries)
		{
			EditDepthRecursive(entry, 1, terminalDepth, editFunction);
		}
	}

	void TreeWidget::InsertEntry(const TreeEntry& entry)
	{
		m_TreeEntries.push_back(entry);
	}

	void TreeWidget::RemoveEntry(TreePath& path)
	{
		PathDepth depth{ 0 };
		TreeEntry* parentEntry{ nullptr };
		TreeEntry* currentEntry{ nullptr };

		// Locate entry and its parent entry using the provided path
		for (BranchIndex index : path.GetPath())
		{
			if (depth == 0)
			{
				if (index >= m_TreeEntries.size())
				{
					KG_WARN("Failed to remove entry. Path provided is beyond TreeEntries vector bounds!");
					return;
				}
				currentEntry = &(m_TreeEntries.at(index));
			}
			else
			{
				if (index >= currentEntry->m_SubEntries.size())
				{
					KG_WARN("Failed to remove entry. Path provided is beyond SubEntries vector bounds!");
					return;
				}
				parentEntry = currentEntry;
				currentEntry = &(currentEntry->m_SubEntries.at(index));
			}
			depth++;
		}

		if (!currentEntry)
		{
			KG_WARN("Failed to remove entry. Invalid entry acquired");
			return;
		}

		BranchIndex locationCurrentList = path.GetPath().at(depth - 1);

		// Clear SelectedEntry field if path is the same
		if (m_SelectedEntry == path)
		{
			m_SelectedEntry = {};
		}

		// Decriment SelectedEntry if end of path is greater
		if (m_SelectedEntry.SameParentPath(path) && m_SelectedEntry.GetPath().at(depth - 1) > locationCurrentList)
		{
			m_SelectedEntry.SetNode(m_SelectedEntry.GetPath().at(m_SelectedEntry.GetPath().size() - 1) - 1, m_SelectedEntry.GetPath().size() - 1);
		}

		// Remove Entry from Tree
		if (parentEntry == nullptr)
		{
			m_TreeEntries.erase(m_TreeEntries.begin() + locationCurrentList);
		}
		else
		{
			parentEntry->m_SubEntries.erase(parentEntry->m_SubEntries.begin() + locationCurrentList);
		}

		// Decriment elements inside ExpandedNodes that are higher than the provided path
		std::vector<TreePath> pathCache{};
		for (const TreePath& nodePath : m_ExpandedNodes)
		{
			if (nodePath.SameParentPath(path) && nodePath.GetPath().at(depth - 1) > locationCurrentList)
			{
				pathCache.push_back(nodePath);
			}
		}
		// Remove current path from ExpandedNodes
		if (m_ExpandedNodes.contains(path))
		{
			m_ExpandedNodes.erase(path);
		}
		for (TreePath& nodePath : pathCache)
		{
			m_ExpandedNodes.erase(nodePath);
			nodePath.SetNode(nodePath.GetPath().at(nodePath.GetPath().size() - 1) - 1, nodePath.GetPath().size() - 1);
		}
		for (TreePath& nodePath : pathCache)
		{
			m_ExpandedNodes.insert(nodePath);
		}
	}
	void TreeWidget::ClearTree()
	{
		m_TreeEntries.clear();
		m_ExpandedNodes.clear();
		m_SelectedEntry = {};
	}
	void TreePath::PushBackNode(BranchIndex newNode)
	{
		m_Path.push_back(newNode);
	}
	void TreePath::SetNode(BranchIndex newValue, PathDepth depth)
	{
		if (depth >= m_Path.size())
		{
			KG_WARN("Invalid depth provided. Cannot update TreePath node");
			return;
		}

		m_Path.at(depth) = newValue;
	}
	void TreePath::PopBack()
	{
		m_Path.pop_back();
	}
	uint16_t TreePath::GetBack()
	{
		return m_Path.back();
	}
	void TreePath::SetBack(BranchIndex newNode)
	{
		PopBack();
		m_Path.push_back(newNode);
	}
	std::size_t TreePath::GetDepth()
	{
		return m_Path.size();
	}
	bool TreePath::operator==(const TreePath& other) const
	{
		return m_Path == other.m_Path;
	}
	bool TreePath::SameParentPath(const TreePath& other) const
	{
		// Parent path cannot be equal if sizes do not match
		if (m_Path.size() != other.m_Path.size())
		{
			return false;
		}

		for (PathDepth depth{ 0 }; depth < m_Path.size(); depth++)
		{
			// Check if nodes differ
			if (m_Path.at(depth) != other.m_Path.at(depth))
			{
				if (depth == m_Path.size() - 1)
				{
					return true;
				}
				else
				{
					return false;
				}
			}
		}

		// If paths are equivalent, return true
		return true;
	}
	TreePath::operator bool() const
	{
		return (bool)m_Path.size();
	}
}