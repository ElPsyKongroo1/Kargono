#pragma once
#include <cstdint>

namespace Kargono::RuntimeUI
{
	constexpr uint16_t k_InvalidWindowID{ std::numeric_limits<uint16_t>().max() };
	constexpr uint16_t k_InvalidWidgetID{ std::numeric_limits<uint16_t>().max() };

	struct WindowID
	{
		uint64_t m_UserInterfaceID{ 0 };
		uint16_t m_WindowIndex{ k_InvalidWindowID };
	};

	struct WidgetID
	{
		uint64_t m_UserInterfaceID{ 0 };
		uint16_t m_WindowIndex{ k_InvalidWindowID };
		uint16_t m_WidgetIndex{ k_InvalidWidgetID };
	};
}
