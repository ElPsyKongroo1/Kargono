#pragma once
#include "Modules/EditorUI/Widgets/EditorUIWidget.h"

#include "Kargono/Core/Base.h"
#include "Kargono/Core/FixedString.h"
#include "Kargono/Core/UUID.h"
#include "Modules/Rendering/Texture.h"

#include "Modules/EditorUI/ExternalAPI/ImGuiAPI.h"

#include <functional>
#include <unordered_set>

namespace Kargono::EditorUI
{
	using BranchIndex = uint16_t;
	using PathDepth = size_t;

	class TreePath
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		TreePath() = default;
		~TreePath() = default;
	public:
		//==============================
		// Modify Path
		//==============================
		// Add
		void PushBackNode(BranchIndex newNode);
		// Remove
		void PopBack();
		// Update
		void SetNode(BranchIndex newValue, PathDepth depth);
		void SetBack(BranchIndex newNode);
	public:
		//==============================
		// Query Path
		//==============================
		// Get
		BranchIndex GetBack();
		PathDepth GetDepth();
		// Is same
		bool SameParentPath(const TreePath& other) const;
	public:
		//==============================
		// Operator Overloads
		//==============================
		bool operator==(const TreePath& other) const;
		operator bool() const;

	public:
		//==============================
		// Getters/Setters
		//==============================
		const std::vector<BranchIndex>& GetPath() const
		{
			return m_Path;
		}
	private:
		//==============================
		// Internal Fields
		//==============================
		std::vector<BranchIndex> m_Path{};
	};
}

namespace std
{
	template<>
	struct hash<Kargono::EditorUI::TreePath>
	{
		std::size_t operator()(const Kargono::EditorUI::TreePath& path) const
		{
			std::size_t seed = 0;
			for (uint16_t node : path.GetPath())
			{
				seed ^= std::hash<uint16_t>()(node) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			}
			return seed;
		}
	};
}

namespace Kargono::EditorUI
{
	struct TreeEntry;

	struct SelectionEntry
	{
		std::string m_Label{};
		std::function<void(TreeEntry&)> m_OnClick{ nullptr };
	};

	struct TreeEntry
	{
		std::string m_Label{};
		UUID m_Handle{};
		Ref<Rendering::Texture2D> m_IconHandle{ nullptr };
		std::function<void(TreeEntry& entry)> m_OnLeftClick{ nullptr };
		std::function<void(TreeEntry& entry)> m_OnDoubleLeftClick{ nullptr };
		std::function<void(TreeEntry& entry)> m_OnRightClick{ nullptr };
		Ref<void> m_ProvidedData{ nullptr };
		std::vector<TreeEntry> m_SubEntries{};
	};

	struct TreeWidget : public Widget
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		TreeWidget() : Widget() {}
		~TreeWidget() = default;
	public:
		//==============================
		// Rendering
		//==============================
		void RenderTree();
	private:
		// Rendering helper
		void DrawEntryList(std::vector<TreeEntry>& entries, TreePath& currentPath, ImVec2 rootPosition);
	public:
		//==============================
		// Interact w/ Tree
		//==============================
		// Move directionally
		void MoveUp();
		void MoveDown();
		void MoveLeft();
		void MoveRight();
		// Jump
		void SelectFirstEntry();
		bool SelectEntry(TreePath& path);
		// Expand
		void ExpandFirstLayer();
		void ClearExpandedNodes();
		void ExpandNodePath(TreePath& path);

	public:
		//==============================
		// Modify Tree Entries
		//==============================
		// Create
		void InsertEntry(const TreeEntry& entry);
		// Remove/delete
		void RemoveEntry(TreePath& path);
		void ClearTree();
		// Edit/update
		void EditDepth(std::function<void(TreeEntry& entry)> editFunction, size_t depth = 0);

	public:
		//==============================
		// Query Tree Entries
		//==============================
		// Find entry(s)
		TreeEntry* SearchFirstLayer(UUID queryHandle);
		TreeEntry* SearchDepth(UUID queryHandle, size_t terminalDepth = 0);
		TreeEntry* GetEntryFromPath(TreePath& path);
		std::vector<TreePath> SearchDepth(std::function<bool(TreeEntry& entry)> searchFunction, size_t depth = 0);
		TreePath GetPathFromEntryReference(TreeEntry* entryQuery);
	private:
		// Query helpers
		void SearchDepthRecursive(TreeEntry& currentEntry, size_t currentDepth, size_t terminalDepth, std::function<bool(TreeEntry& entry)> searchFunction, std::vector<TreePath>& allPaths);
		TreeEntry* SearchDepthRecursive(TreeEntry& currentEntry, size_t currentDepth, size_t terminalDepth, UUID queryID);
		void EditDepthRecursive(TreeEntry& currentEntry, size_t currentDepth, size_t terminalDepth, std::function<void(TreeEntry& entry)> editFunction);
	public:
		//==============================
		// Getters/Setters
		//==============================
		std::vector<TreeEntry>& GetTreeEntries()
		{
			return m_TreeEntries;
		}
	
	public:
		//==============================
		// Public Fields
		//==============================
		FixedString32 m_Label;
		TreePath m_SelectedEntry{};
		std::function<void()> m_OnRefresh{ nullptr };
	private:
		//==============================
		// Internal Fields
		//==============================
		std::vector<TreeEntry> m_TreeEntries{};
		std::unordered_set<TreePath> m_ExpandedNodes{};
		bool m_SelectionChanged{ false };
	};
}






