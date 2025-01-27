#pragma once
#include <cstdint>

namespace Kargono::RuntimeUI
{
	struct WindowID
	{
		uint64_t m_UserInterfaceID{ 0 };
		uint16_t m_WindowIndex{ 0 };
	};

	struct WidgetID
	{
		uint64_t m_UserInterfaceID{ 0 };
		uint16_t m_WindowIndex{ 0 };
		uint16_t m_WidgetIndex{ 0 };
	};
}
