#pragma once

#include "Kargono/Core/BitField.h"
#include "Kargono/Core/Buffer.h"

#include <vector>
#include <array>
#include <unordered_map>
#include <cstdint>

namespace Kargono
{
	//==============================
	// Supporting Defines for ArchetypeContext Class
	//==============================
	// NOTE this system can only hold a maximum of 16 different components
	constexpr inline uint16_t k_MaximumComponentIndex{ 15 };
	constexpr inline uint16_t k_InvalidComponentIndex{ std::numeric_limits<uint16_t>().max() };

	using ArchetypeID = BitField<uint16_t>;
	using ComponentID = uint16_t;

	struct ArchetypeBuffer
	{
		std::vector<uint8_t> m_DataBuffer;
		size_t m_TerminalIndex{ 0 };
	};

	struct Archetype 
	{
		ArchetypeID m_ID;
		uint16_t m_ArchSize;
		std::vector<ComponentID> m_Components;
		std::unordered_map<ComponentID, uint16_t> m_ComponentBufferLocation;
		ArchetypeBuffer m_ArchBuffer;
	};

	//==============================
	// Main ArchetypeContext Object Class
	//==============================
	class ArchetypeContext
	{
	public:
		//==============================
		// Constructor/Destructor
		//==============================
		ArchetypeContext() = default;
		~ArchetypeContext() = default;

	public:
		void CreateEntity(ArchetypeID id);
		ComponentID AddComponent(size_t componentSize);
	private:
		void CreateArchetype(ArchetypeID id);
	private:
		//==============================
		// Internal Data
		//==============================
		std::unordered_map<ArchetypeID, Archetype> m_Archetypes;
		std::unordered_map<ComponentID, size_t> m_ComponentSizes;
		uint16_t m_TerminalComponentIndex{ 0 };
	};


	
}
