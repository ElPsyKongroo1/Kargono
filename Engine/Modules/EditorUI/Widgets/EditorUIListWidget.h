#pragma once
#include "Modules/EditorUI/Widgets/EditorUIWidget.h"
#include "Modules/EditorUI/EditorUICommon.h"

#include "Kargono/Core/Base.h"
#include "Kargono/Core/FixedString.h"
#include "Modules/Assets/AssetsTypes.h"

#include <functional>
#include <limits>
#include <string>

namespace Kargono::EditorUI
{
	enum ListFlags : WidgetFlags
	{
		List_None = 0,
		List_UnderlineTitle = BIT(0), // Adds an underline to the title
		List_RegularSizeTitle = BIT(1), // Ensures title uses normally sized font
		List_Indented = BIT(2) // Indents the list over once
	};

	struct ListEntry
	{
		std::string m_Label;
		std::string m_Value;
		UUID m_Handle;
		std::function<void(ListEntry& entry, std::size_t iteration)> m_OnEdit{ nullptr };
	};

	using ListEntryIndex = size_t;
	constexpr ListEntryIndex k_ListSearchIndex{ std::numeric_limits<ListEntryIndex>::max() };
	constexpr ListEntryIndex k_ListIndex{ std::numeric_limits<ListEntryIndex>::max() };

	struct ListWidget : public Widget
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		ListWidget() : Widget() {}
		~ListWidget() = default;
	public:
		//==============================
		// Rendering
		//==============================
		void RenderList();
	public:
		//==============================
		// Manage List Entries
		//==============================
		// Add
		void InsertListEntry(const std::string& label, const std::string& value, std::function<void(ListEntry& entry, std::size_t iteration)> onEdit, Assets::AssetHandle handle = 0);
		void InsertListEntry(const ListEntry& entry);
		// Remove
		bool RemoveEntry(ListEntryIndex entryIndex);
		void ClearList();
		// Modify
		void EditEntries(std::function<void(ListEntry& currentEntry)> editFunction);

		//==============================
		// Query List Entries
		//==============================
		// List 
		size_t GetEntriesListSize();
		// Entry(s)
		ListEntry& GetEntry(ListEntryIndex index);
		ListEntryIndex SearchEntries(std::function<bool(const ListEntry& currentEntry)> searchFunction);

		//==============================
		// Manage Selection List
		//==============================
		void AddToSelectionList(const std::string& label, std::function<void()> function);
		void ClearEditListSelectionList();

		//==============================
		// Getters/Setters
		//==============================
		SelectionList& GetEditSelectionList()
		{
			return m_EditListSelectionList;
		}
	public:
		//==============================
		// Public Fields
		//==============================
		FixedString32 m_Label;
		WidgetFlags m_Flags{ List_None };
		std::string m_Column1Title{};
		std::string m_Column2Title{};
		bool m_Expanded{ false };
		std::function<void()> m_OnRefresh{ nullptr };
	private:
		//==============================
		// Internal Fields
		//==============================
		std::vector<ListEntry> m_ListEntries{};
		SelectionList m_EditListSelectionList{};
	};
}


