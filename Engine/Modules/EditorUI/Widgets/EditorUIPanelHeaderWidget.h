#pragma once
#include "Modules/EditorUI/Widgets/EditorUIWidget.h"
#include "Modules/EditorUI/EditorUICommon.h"

#include "Kargono/Core/Base.h"
#include "Kargono/Core/FixedString.h"

#include <functional>

namespace Kargono::EditorUI
{
	struct PanelHeaderWidget : public Widget
	{
		using PanelHeaderSelectionList = std::unordered_map<std::string, std::function<void(EditorUI::PanelHeaderWidget& spec)>>;
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		PanelHeaderWidget() : Widget() {}
		~PanelHeaderWidget() = default;
	public:
		//==============================
		// Rendering
		//==============================
		void RenderHeader();
	public:
		//==============================
		// Modify Selection List
		//==============================
		void AddToSelectionList(const std::string& label, std::function<void()> function);
		void ClearSelectionList();
		//==============================
		// Getters/Setters
		//==============================
		SelectionList& GetSelectionList()
		{
			return m_SelectionsList;
		}
	public:
		//==============================
		// Public Fields
		//==============================
		FixedString32 m_Label;
		bool m_EditColorActive{ false };
	private:
		//==============================
		// Internal Fields
		//==============================
		SelectionList m_SelectionsList{};
	};
}


