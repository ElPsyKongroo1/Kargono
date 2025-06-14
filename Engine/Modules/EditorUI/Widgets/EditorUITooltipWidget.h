#pragma once
#include "Modules/EditorUI/Widgets/EditorUIWidget.h"

#include "Kargono/Core/Base.h"
#include "Kargono/Core/FixedString.h"

#include "Modules/EditorUI/ExternalAPI/ImGuiAPI.h"
#include "Modules/EditorUI/EditorUIContext.h"

#include <functional>

namespace Kargono::EditorUI
{
	struct TooltipSeperatorData
	{
		ImVec4 m_SeperatorColor{ EditorUIContext::m_ConfigColors.m_DisabledColor };
	};

	struct TooltipEntry
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		TooltipEntry(const char* itemLabel, std::function<void(TooltipEntry&)> onClick) : m_Label(itemLabel), m_EntryData(onClick){}
		TooltipEntry(const char* itemLabel, const std::vector<TooltipEntry>& subEntryList) : m_Label(itemLabel), m_EntryData(subEntryList){}
		TooltipEntry(const char* itemLabel, std::vector<TooltipEntry>&& subEntryList) : m_Label(itemLabel), m_EntryData(std::move(subEntryList)){}
	private:
		TooltipEntry() {};
	public:
		//==============================
		// Public Fields
		//==============================
		FixedString32 m_Label;
		UUID m_EntryID;
		bool m_IsVisible{ true };
		UUID m_UserHandle;
		void* m_ProvidedData{ nullptr };
	private:
		//==============================
		// Internal Fields
		//==============================
		std::variant<std::vector<TooltipEntry>, std::function<void(TooltipEntry&)>, TooltipSeperatorData> m_EntryData;
	private:
		friend struct TooltipWidget;
	};

	struct TooltipWidget : public Widget
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		TooltipWidget() : Widget() {}
		~TooltipWidget() = default;
	public:
		//==============================
		// Rendering
		//==============================
		void RenderTooltip();
	private:
		// Render helper(s)
		void RenderTooltipEntryList(std::vector<TooltipEntry>& entryList);
	public:
		//==============================
		// Add Menu/Item Entry To Tooltip
		//==============================
		// Add seperator entry
		void AddSeperator(ImVec4 seperatorColor);
		// Add menu/item entry
		UUID AddTooltipEntry(TooltipEntry& newEntry);
		UUID AddTooltipEntry(TooltipEntry&& newEntry);

	public:
		//==============================
		// Interact w/ Existing Entries
		//==============================
		bool SetIsVisible(UUID entry, bool isVisible);
		bool SetAllChildrenIsVisible(UUID entry, bool isVisible);
	private:
		// Helpers
		bool SetIsVisibleRecursive(std::vector<TooltipEntry>& entries, UUID queryID, bool isVisible);
		bool SetAllChildrenIsVisibleRecursive(std::vector<TooltipEntry>& entries, UUID queryID, bool isVisible);

	public:
		//==============================
		// Remove Entries
		//==============================
		void ClearEntries();
	private:
		//==============================
		// Entry Validation
		//==============================
		bool ValidateEntryID(UUID queryID);
		bool ValidateEntryIDRecursive(std::vector<TooltipEntry>& entries, UUID queryID);
	public:
		//==============================
		// Public Fields
		//==============================
		FixedString32 m_Label;
		bool m_TooltipActive{ false };
	private:
		//==============================
		// Internal Fields
		//==============================
		std::vector<TooltipEntry> m_Entries{};
	};
}


