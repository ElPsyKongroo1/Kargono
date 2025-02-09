#pragma once

namespace Kargono::ECS
{
	static inline std::array<uint64_t, 32> s_AllBufferSizes
	{
		4, 8, 12, 16, 20, 24, 28, 32,
			40, 48, 56, 64, 72, 80, 88, 96,
			112, 128, 144, 160, 176, 192, 208, 224,
			256, 288, 320, 352, 384, 416, 448, 480
	};

	inline uint64_t GetBufferSizeFromComponentSize(uint64_t componentSize)
	{
		auto upperBoundRef = std::lower_bound(s_AllBufferSizes.begin(), s_AllBufferSizes.end(), componentSize);
		KG_ASSERT(upperBoundRef != s_AllBufferSizes.end(), "Could not locate buffer value that contains component size properly");
		return *upperBoundRef;
	}

	constexpr uint16_t InvalidBufferSlot = std::numeric_limits<int16_t>::max();
}
