#pragma once
#include "Modules/EditorUI/Widgets/EditorUIWidget.h"

#include "Kargono/Core/Base.h"
#include "Kargono/Core/FixedString.h"

#include <functional>
#include <unordered_map>
#include <string>

namespace Kargono::EditorUI
{
	enum CollapsingHeaderFlags : WidgetFlags
	{
		CollapsingHeader_None = 0,
		CollapsingHeader_UnderlineTitle = BIT(0), // Underlines the title text
	};

	struct CollapsingHeaderWidget : public Widget
	{
	public:
		using CollapsingHeaderSelectionList = std::unordered_map<std::string, std::function<void(EditorUI::CollapsingHeaderWidget& spec)>>;
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		CollapsingHeaderWidget() : Widget() {}
		~CollapsingHeaderWidget() = default;

	public:
		//==============================
		// Rendering
		//==============================
		void RenderHeader();
	public:
		//==============================
		// Modify Selections List
		//==============================
		void AddToSelectionList(const std::string& label, std::function<void(CollapsingHeaderWidget&)> function);
		void ClearSelectionList();
	public:
		//==============================
		// Query Selections List
		//==============================
		CollapsingHeaderSelectionList& GetSelectionList()
		{
			return m_SelectionList;
		}
	public:
		//==============================
		// Public Fields
		//==============================
		FixedString32 m_Label;
		WidgetFlags m_Flags{ CollapsingHeader_None };
		bool m_Expanded{ false };
		std::function<void()> m_OnExpand{ nullptr };
		Ref<void> m_ProvidedData{ nullptr };
	private:
		//==============================
		// Internal Fields
		//==============================
		CollapsingHeaderSelectionList m_SelectionList{};
	};
}


