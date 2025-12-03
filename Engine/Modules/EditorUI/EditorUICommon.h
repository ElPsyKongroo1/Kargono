#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Core/FixedBufferString.h"
#include "Modules/Assets/AssetReference.h"
#include "Modules/Rendering/Assets/Texture2D.h"

#include <cstdint>
#include <unordered_map>
#include <functional>

namespace Kargono::EditorUI
{
	struct DragDropPayload
	{
		FixedBufStr32 m_Label;
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
		Assets::AssetRef<Rendering::Texture2D> m_ActiveIcon{};
		Assets::AssetRef<Rendering::Texture2D> m_InactiveIcon{};
		std::string m_ActiveTooltip{};
		std::string m_InactiveTooltip{};
		PositionType m_XPositionType{ PositionType::Inline };
		bool m_Disabled{ false };
	};
}