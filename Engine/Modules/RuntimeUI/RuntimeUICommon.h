#pragma once

#include "Modules/Assets/AssetsTypes.h"

#include <cstdint>

namespace Kargono::RuntimeUI
{
	using WidgetIndex = uint16_t;
	using WindowIndex = uint16_t;
	using WidgetID = int32_t;
	using WindowID = int32_t;
	using WidgetOrWindowID = int32_t;

	constexpr uint64_t k_InvalidUserInterfaceID{ 0 };
	constexpr WindowIndex k_InvalidWindowIndex{ std::numeric_limits<WindowIndex>().max() };
	constexpr WidgetIndex k_InvalidWidgetIndex{ std::numeric_limits<WidgetIndex>().max() };
	constexpr WidgetID k_InvalidWidgetID{ std::numeric_limits<WidgetID>().max() };
	constexpr WindowID k_InvalidWindowID{ std::numeric_limits<WindowID>().max() };

	enum class IDType
	{
		None = 0,
		Window,
		Widget
	};

	struct WindowHandle
	{
		Assets::AssetHandle m_UserInterfaceID{ k_InvalidUserInterfaceID };
		WindowID m_WindowID{ k_InvalidWindowID};
	};

	struct WidgetHandle
	{
		Assets::AssetHandle m_UserInterfaceID{ k_InvalidUserInterfaceID };
		WidgetID m_WidgetID{ k_InvalidWidgetID };
	};
}
