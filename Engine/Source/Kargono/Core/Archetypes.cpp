#include "kgpch.h"

#include "Kargono/Core/Archetypes.h"

namespace Kargono
{
	void ArchetypeContext::CreateEntity(ArchetypeID id)
	{
		//// Add archetype to buffer
		//if (!m_Archetypes.contains(id))
		//{
		//	// Add a new archetype
		//	CreateArchetype(id);
		//}

		//Archetype& archetype = m_Archetypes[id];
		//std::vector<uint8_t>& buffer = archetype.m_ArchBuffer.m_DataBuffer;

		//// Ensure buffer is big enough for new entity
		//if (buffer.size() + (size_t)archetype.m_ArchSize > buffer.capacity())
		//{
		//	buffer.reserve(buffer.capacity() * 2);
		//}

		//// Insert entity into archetype
		////memcpy(buffer.data() + archetype.m_ArchBuffer.m_TerminalIndex, )
		////;
		
	}
	ComponentID ArchetypeContext::AddComponent(size_t componentSize)
	{
		//// Ensure a new component can be added
		//if (m_TerminalComponentIndex >= k_MaximumComponentIndex)
		//{
		//	KG_WARN("Attempt to add a component to an archetype, however, maximum components reached!");
		//	return k_InvalidComponentIndex;
		//}

		//// Create new component ID
		//ComponentID newID{ (uint16_t)BIT(m_TerminalComponentIndex) };
		//m_TerminalComponentIndex++;

		//// Insert new component
		////m_ComponentSizes.insert_or_assign(newID, componentSize);
		return 0;
	}
	void ArchetypeContext::CreateArchetype(ArchetypeID id)
	{
		//// Create new archetype
		//Archetype newArchetype;
		//newArchetype.m_ID = id;

		//// Sort the component ID's in ascending order
		//std::set<ComponentID> compIDs;
		//for (auto [componentID, componentSize] : m_ComponentSizes)
		//{
		//	//compIDs.insert(componentID);
		//}

		//// Store ordered list of components and their buffer indices
		//uint16_t bufferLocation{ 0 };
		//for (ComponentID id : compIDs)
		//{
		//	//newArchetype.m_Components.push_back(id);
		//	//newArchetype.m_ComponentBufferLocation.insert_or_assign(id, bufferLocation);
		//	bufferLocation += (uint16_t)m_ComponentSizes[id];
		//}

		//newArchetype.m_ArchSize = bufferLocation;

		//// Insert archetype
		////m_Archetypes.insert_or_assign(id, newArchetype);
	}
}
