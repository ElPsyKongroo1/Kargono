#pragma once
#include <cstdint>

namespace Kargono::RuntimeUI
{
	constexpr uint16_t k_InvalidWindowIndex{ std::numeric_limits<uint16_t>().max() };
	constexpr uint16_t k_InvalidWidgetIndex{ std::numeric_limits<uint16_t>().max() };
	constexpr uint64_t k_InvalidWidgetID{ std::numeric_limits<uint64_t>().max() };

	struct WindowID
	{
		uint64_t m_UserInterfaceID{ 0 };
		uint16_t m_WindowIndex{ k_InvalidWindowIndex };
	};

	struct WidgetID
	{
		uint64_t m_UserInterfaceID{ 0 };
		uint16_t m_WindowIndex{ k_InvalidWindowIndex };
		uint16_t m_WidgetIndex{ k_InvalidWidgetIndex };
	};
}
