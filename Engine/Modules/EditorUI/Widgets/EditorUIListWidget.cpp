#include "kgpch.h"

#include "Modules/EditorUI/Widgets/EditorUIListWidget.h"
#include "Modules/EditorUI/EditorUIContext.h"

#include "Modules/EditorUI/ExternalAPI/ImGuiBackendAPI.h"
#include "Kargono/Utility/Operations.h"

namespace Kargono::EditorUI
{
	void ListWidget::RenderList()
	{
		ResetChildID();

		if (m_Flags & List_Indented)
		{
			ImGui::SetCursorPosX(EditorUIContext::m_ConfigSpacing.m_PrimaryTextIndent);
		}
		if (!(m_Flags & (List_RegularSizeTitle | List_Indented)))
		{
			ImGui::PushFont(EditorUIContext::m_ConfigFonts.m_HeaderLarge);
		}
		ImGui::PushStyleColor(ImGuiCol_Text, EditorUIContext::m_ConfigColors.m_PrimaryTextColor);
		int32_t labelPosition = ImGui::FindPositionAfterLength(m_Label.CString(),
			m_Flags & List_Indented ? EditorUIContext::m_ActiveWindowData.m_PrimaryTextIndentedWidth : EditorUIContext::m_ActiveWindowData.m_PrimaryTextWidth);
		EditorUIContext::RenderTruncatedText(m_Label.CString(), labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);
		ImGui::PopStyleColor();

		if (!(m_Flags & (List_RegularSizeTitle | List_Indented)))
		{
			ImGui::PopFont();
		}
		EditorUIContext::m_UIPresets.m_ListExpandButton.m_IconSize = 14.0f;
		EditorUIContext::m_UIPresets.m_ListExpandButton.m_YPosition = m_Flags & List_Indented ? 0.0f : 3.0f;
		ImGui::SameLine();
		EditorUIContext::RenderInlineButton(GetNextChildID(), [&]()
			{
				Utility::Operations::ToggleBoolean(m_Expanded);
			},
			EditorUIContext::m_UIPresets.m_ListExpandButton, m_Expanded, m_Expanded ? EditorUIContext::m_ConfigColors.m_HighlightColor1 : EditorUIContext::m_ConfigColors.m_DisabledColor);

		if (m_Expanded && !m_EditListSelectionList.empty())
		{
			ImGui::SameLine();
			EditorUIContext::RenderInlineButton(GetNextChildID(), [&]()
				{
					ImGui::OpenPopup(m_WidgetID - 1);
				}, EditorUIContext::m_UIPresets.m_MediumOptionsButton, false, EditorUIContext::m_ConfigColors.m_DisabledColor);

			if (ImGui::BeginPopupEx(m_WidgetID - 1, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings))
			{
				for (auto& [label, func] : m_EditListSelectionList)
				{
					if (ImGui::Selectable((label.c_str() + m_WidgetIDString).c_str()))
					{
						func();
					}
				}
				ImGui::EndPopup();
			}
		}

		if (m_Flags & List_UnderlineTitle)
		{
			ImGui::Separator();
		}

		if (m_Expanded)
		{
			if (!m_ListEntries.empty())
			{
				// Column Titles
				ImGui::PushStyleColor(ImGuiCol_Text, EditorUIContext::m_ConfigColors.m_HighlightColor1);
				ImGui::SetCursorPosX(m_Flags & List_Indented ? 61.0f : EditorUIContext::m_ConfigSpacing.m_PrimaryTextIndent);
				if (m_Flags & (List_Indented | List_RegularSizeTitle))
				{
					ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 7.0f);
				}
				labelPosition = ImGui::FindPositionAfterLength(m_Column1Title.c_str(), EditorUIContext::m_ActiveWindowData.m_SecondaryTextLargeWidth);
				EditorUIContext::RenderTruncatedText(m_Column1Title, labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);
				ImGui::SameLine();
				ImGui::SetCursorPosX(EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosOne);
				labelPosition = ImGui::FindPositionAfterLength(m_Column2Title.c_str(), EditorUIContext::m_ActiveWindowData.m_SecondaryTextLargeWidth);
				EditorUIContext::RenderTruncatedText(m_Column2Title, labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);
				ImGui::PopStyleColor();
				if (!(m_Flags & (List_Indented | List_RegularSizeTitle)))
				{
					EditorUIContext::Spacing(SpacingAmount::Small);
				}

			}
			ListEntryIndex entryIndex{ 0 };
			uint32_t smallButtonCount{ 0 };
			for (ListEntry& listEntry : m_ListEntries)
			{
				smallButtonCount = 0;
				if (!(m_Flags & (List_Indented | List_RegularSizeTitle)))
				{
					ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3.0f);
				}
				ImGui::SetCursorPosX(m_Flags & List_Indented ? 42.5f : 12.0f);
				EditorUIContext::RenderImage(EditorUIContext::m_GenIcons.m_Dash, 8, EditorUIContext::m_ConfigColors.m_DisabledColor);
				ImGui::SameLine();
				ImGui::SetCursorPosX(m_Flags & List_Indented ? 61.0f : EditorUIContext::m_ConfigSpacing.m_PrimaryTextIndent);
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5.2f);
				EditorUIContext::RenderTruncatedText(listEntry.m_Label, 16);
				ImGui::PushStyleColor(ImGuiCol_Text, EditorUIContext::m_ConfigColors.m_SecondaryTextColor);
				if (!listEntry.m_Value.empty())
				{
					EditorUIContext::RenderMultiLineText(listEntry.m_Value, EditorUIContext::m_ActiveWindowData.m_SecondaryTextLargeWidth, EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosOne, -5.2f);
				}
				ImGui::PopStyleColor();

				if (listEntry.m_OnEdit)
				{
					EditorUIContext::m_UIPresets.m_TableEditButton.m_XPosition = EditorUIContext::m_ConfigSpacing.SmallButtonRelativeLocation(smallButtonCount++);
					ImGui::SameLine();
					EditorUIContext::RenderInlineButton(GetNextChildID(), [&]()
						{
							if (listEntry.m_OnEdit)
							{
								listEntry.m_OnEdit(listEntry, entryIndex);
							}
						}, EditorUIContext::m_UIPresets.m_TableEditButton, false, EditorUIContext::m_ConfigColors.m_DisabledColor);
				}
				entryIndex++;
			}
		}
	}
	void ListWidget::InsertListEntry(const std::string& label, const std::string& value, std::function<void(ListEntry& entry, std::size_t iteration)> onEdit, Assets::AssetHandle handle)
	{
		ListEntry newEntry{ label, value, handle, onEdit };
		m_ListEntries.push_back(newEntry);
	}
	void ListWidget::InsertListEntry(const ListEntry& entry)
	{
		m_ListEntries.push_back(entry);
	}
	bool ListWidget::RemoveEntry(ListEntryIndex entryIndex)
	{
		if (entryIndex >= m_ListEntries.size())
		{
			return false;
		}
		m_ListEntries.erase(m_ListEntries.begin() + entryIndex);
		return true;
	}
	ListEntryIndex ListWidget::SearchEntries(std::function<bool(const ListEntry& currentEntry)> searchFunction)
	{
		// Run search function on entry
		ListEntryIndex index{ 0 };
		for (const ListEntry& currentEntry : m_ListEntries)
		{
			if (searchFunction(currentEntry))
			{
				return index;
			}
			index++;
		}

		// Return invalid index if none is found
		return k_ListSearchIndex;
	}
	void ListWidget::EditEntries(std::function<void(ListEntry& currentEntry)> editFunction)
	{
		// Run edit function on every entry
		for (ListEntry& currentEntry : m_ListEntries)
		{
			if (editFunction)
			{
				editFunction(currentEntry);
			}
		}
	}
	void ListWidget::ClearList()
	{
		m_ListEntries.clear();
	}
	void ListWidget::ClearEditListSelectionList()
	{
		m_EditListSelectionList.clear();
	}
	void ListWidget::AddToSelectionList(const std::string& label, std::function<void()> function)
	{
		if (!m_EditListSelectionList.contains(label))
		{
			m_EditListSelectionList.insert_or_assign(label, function);
			return;
		}
	}
	std::size_t ListWidget::GetEntriesListSize()
	{
		return m_ListEntries.size();
	}
	ListEntry& ListWidget::GetEntry(ListEntryIndex index)
	{
		KG_ASSERT(index < m_ListEntries.size());
		return m_ListEntries.at(index);
	}
}

