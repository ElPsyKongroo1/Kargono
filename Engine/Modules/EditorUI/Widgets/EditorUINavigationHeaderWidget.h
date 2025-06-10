#pragma once
#include "Modules/EditorUI/Widgets/EditorUIWidget.h"

#include "Kargono/Core/Base.h"
#include "Kargono/Core/FixedString.h"

#include <functional>

namespace Kargono::EditorUI
{
	enum NavigationHeaderFlags : WidgetFlags
	{
		NavigationHeader_None = 0,
		NavigationHeader_AllowDragDrop = BIT(0)
	};

	struct NavigationHeaderWidget : public Widget
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		NavigationHeaderWidget() : Widget() {}
		~NavigationHeaderWidget() = default;
	public:
		//==============================
		// Rendering
		//==============================
		void RenderHeader();
	public:
		//==============================
		// Public Fields
		//==============================
		FixedString64 m_Label;
		std::function<void()> m_OnNavigateBack{};
		std::function<void()> m_OnNavigateForward{};
		std::function<void(const char*, void*, std::size_t)> m_OnReceivePayloadBack{};
		std::function<void(const char*, void*, std::size_t)> m_OnReceivePayloadForward{};
		std::vector<FixedString32> m_AcceptableOnReceivePayloads;
		WidgetFlags m_Flags{ 0 };
		bool m_IsBackActive{ false };
		bool m_IsForwardActive{ false };
	};
}


