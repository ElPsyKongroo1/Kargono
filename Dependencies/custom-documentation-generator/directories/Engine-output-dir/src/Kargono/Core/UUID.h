#pragma once

#include <string>

namespace Kargono
{
	class UUID
	{
	public:
		
		UUID();
		UUID(uint64_t uuid);
		UUID(const UUID&) = default;

		operator uint64_t() const { return m_UUID; }
		operator std::string() const { return std::to_string(m_UUID); }
	private:
		uint64_t m_UUID;
	};
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
