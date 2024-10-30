#pragma once

#include <string>

namespace Kargono
{
	//==============================
	// Universal Identifier Class
	//==============================
	class UUID
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		UUID();
		UUID(uint64_t uuid);
		UUID(const UUID&) = default;
		//==============================
		// Getters/Setters
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

	// Provide empty handle definition
	static inline UUID k_EmptyUUID{ 0 };
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
