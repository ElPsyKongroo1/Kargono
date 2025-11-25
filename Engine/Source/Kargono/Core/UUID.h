#pragma once

#include "Kargono/Utility/Random.h"

#include <string>
#include <cstdint>

namespace Kargono
{
	class UUID
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		constexpr UUID() : m_UUID(0) {}
		constexpr UUID(uint64_t uuid) : m_UUID(uuid) {}
		constexpr UUID(const UUID&) = default;
	public:
		//==============================
		// Operator Overloads
		//==============================
		operator uint64_t() const 
		{ 
			return m_UUID; 
		}
		operator std::string() const 
		{ 
			return std::to_string(m_UUID); 
		}
	private:
		//==============================
		// Internal Fields
		//==============================
		uint64_t m_UUID;
	};

	class RandomUUIDService
	{
	public:
		static UUID GetRandomUUID()
		{
			return UUID(s_Random.GenerateRandomUInt64(1, std::numeric_limits<uint64_t>::max()));
		}
	private:
		static inline Utility::STLRandom s_Random{};
	};

	// Provide empty handle definition
	constexpr uint64_t k_EmptyUUID{ 0 };
}

namespace std
{
	template<>
	struct hash<Kargono::UUID>
	{
		std::size_t operator()(const Kargono::UUID& uuid) const
		{
			return (uint64_t)uuid;
		}
	};
}
