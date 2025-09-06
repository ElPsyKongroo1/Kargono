#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Core/FixedString.h"

#include <cstdint>
#include <unordered_map>
#include <functional>

namespace Kargono::Rendering { class Texture2D; }

namespace Kargono::EditorUI
{
	struct DragDropPayload
	{
		FixedString32 m_Label;
		void* m_DataPointer;
		size_t m_DataSize;
	};

	using SelectionList = std::unordered_map<std::string, std::function<void()>>;
	enum class SpacingAmount : uint8_t
	{
		None = 0,
		Small,
		Medium,
		Large
	};

	enum class PositionType : uint8_t
	{
		None = 0,
		Absolute,
		Relative,
		Inline
	};

	struct InlineButton
	{
	public:
		float m_XPosition{ 0.0f };
		float m_YPosition{ 0.0f };
		float m_IconSize{ 0.0f };
		Ref<Rendering::Texture2D> m_ActiveIcon{ nullptr };
		Ref<Rendering::Texture2D> m_InactiveIcon{ nullptr };
		std::string m_ActiveTooltip{};
		std::string m_InactiveTooltip{};
		PositionType m_XPositionType{ PositionType::Inline };
		bool m_Disabled{ false };
	};
}