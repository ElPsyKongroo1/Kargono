#pragma once
#include "Modules/EditorUI/Widgets/EditorUIWidget.h"

#include "Kargono/Core/Base.h"
#include "Kargono/Core/FixedString.h"

#include <functional>

namespace Kargono::EditorUI
{
	enum ChooseDirectoryFlags : WidgetFlags
	{
		ChooseDirectory_None = 0,
		ChooseDirectory_Indented = BIT(0) // Display indented
	};

	struct ChooseDirectoryWidget : public Widget
	{
	public:
		//==============================
		// Constructor/Destructor
		//==============================
		ChooseDirectoryWidget() : Widget() {}
		~ChooseDirectoryWidget() = default;
	public:
		//==============================
		// Rendering
		//==============================
		void RenderChooseDir();
	public:
		//==============================
		// Public Fields
		//==============================
		FixedString32 m_Label;
		WidgetFlags m_Flags{ ChooseDirectoryFlags::ChooseDirectory_None };
		std::filesystem::path m_CurrentOption{};
		std::function<void(const std::string&)> m_ConfirmAction{ nullptr };
	};
}

