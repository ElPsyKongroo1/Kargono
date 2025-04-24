#pragma once
#include <cstdint>

namespace Kargono::RuntimeUI
{
	constexpr uint64_t k_InvalidUserInterfaceID{ 0 };
	constexpr uint16_t k_InvalidWindowIndex{ std::numeric_limits<uint16_t>().max() };
	constexpr uint16_t k_InvalidWidgetIndex{ std::numeric_limits<uint16_t>().max() };
	constexpr int32_t k_InvalidWidgetID{ std::numeric_limits<int32_t>().max() };
	constexpr int32_t k_InvalidWindowID{ std::numeric_limits<int32_t>().max() };

	enum class IDType
	{
		None = 0,
		Window,
		Widget
	};

	struct WindowID
	{
		uint64_t m_UserInterfaceID{ k_InvalidUserInterfaceID };
		int32_t m_WindowID{ k_InvalidWindowID};
	};

	struct WidgetID
	{
		uint64_t m_UserInterfaceID{ k_InvalidUserInterfaceID };
		int32_t m_WidgetID{ k_InvalidWidgetID };
	};
}
