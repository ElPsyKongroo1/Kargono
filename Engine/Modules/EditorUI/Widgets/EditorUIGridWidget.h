#pragma once
#include "Modules/EditorUI/Widgets/EditorUIWidget.h"
#include "Modules/EditorUI/EditorUI.h"

#include "Kargono/Core/Base.h"
#include "Kargono/Core/FixedString.h"
#include "Kargono/Core/UUID.h"

#include "Modules/EditorUI/ExternalAPI/ImGuiAPI.h"

#include <functional>

namespace Kargono::Rendering { class Texture2D; }

namespace Kargono::EditorUI
{
	enum GridFlags : WidgetFlags
	{
		Grid_None = 0,
		Grid_AllowDragDrop = BIT(0)
	};

	using ArchetypeID = uint32_t;
	constexpr ArchetypeID k_InvalidArchetypeID{ 0 };

	// TODO: Need to fix UUID situation and then fix UUID here!!!

	struct GridEntry
	{
		FixedString64 m_Label;
		ArchetypeID m_ArchetypeID{ k_InvalidArchetypeID };
		UUID m_EntryID;
	};

	struct GridEntryArchetype
	{
		// Display metadata
		Ref<Rendering::Texture2D> m_Icon;
		ImVec4 m_IconColor{ EditorUI::EditorUIService::s_DisabledColor };

		// Handle key input
		std::function<void(GridEntry& currentEntry)> m_OnDoubleLeftClick;
		std::function<void(GridEntry& currentEntry)> m_OnLeftClick;
		std::function<void(GridEntry& currentEntry)> m_OnRightClick;

		// Handle create/receive payload
		std::function<void(GridEntry& currentEntry, DragDropPayload& newPayload)> m_OnCreatePayload;
		std::function<void(GridEntry& currentEntry, const char*, void*, std::size_t)> m_OnReceivePayload;
		std::vector<FixedString32> m_AcceptableOnReceivePayloads;
	};

	struct GridWidget : public Widget
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		GridWidget() : Widget() {}
		~GridWidget() = default;
	public:
		//==============================
		// Rendering
		//==============================
		void RenderGrid();
	public:
		//==============================
		// Modify Grid Entry(s)
		//==============================
		// Create
		bool AddEntry(GridEntry& newEntry);
		bool AddEntry(GridEntry&& newEntry);
		// Delete
		void ClearEntries();
		void ClearSelectedEntry();
	private:
		// Helper
		bool ValidateEntryID(UUID queryID);
	public:
		//==============================
		// Configure Archetypes
		//==============================
		bool AddEntryArchetype(ArchetypeID key, const GridEntryArchetype& newArchetype);
		bool AddEntryArchetype(ArchetypeID key, GridEntryArchetype&& newArchetype);
	public:
		//==============================
		// Public Fields
		//==============================
		FixedString32 m_Label;
		float m_CellPadding{ 25.0f };
		float m_CellIconSize{ 140.0f };
		WidgetFlags m_Flags{ Grid_None };
	private:
		//==============================
		// Internal Fields
		//==============================
		UUID m_SelectedEntry{ k_EmptyUUID };
		std::vector<GridEntry> m_Entries{};
		std::unordered_map<ArchetypeID, GridEntryArchetype> m_EntryArchetypes;
	};
}


