#pragma once

#include "Modules/ECSTest/CommonTest.h"
#include "Kargono/Core/Base.h"

#include "Modules/ECSTest/DataStructures/SparseSetTest.h"

#include <array>
#include <queue>

namespace Kargono::ECS
{
	class EntityRegistryTest
	{	
	private:
		//==============================
		// Constructors/Destructors
		//==============================
		EntityRegistryTest() = default;
		~EntityRegistryTest() = default;
	public:
		//==============================
		// Lifecycle Functions
		//==============================
		[[nodiscard]] bool Init()
		{
			ResetEntities();

			m_Active = true;
			return m_Active;
		}

		[[nodiscard]] bool Terminate()
		{
			ResetEntities();

			m_Active = false;
			return m_Active;
		}

		[[nodiscard]] bool Clear()
		{
			ResetEntities();
		}
	private:
		// Helpers
		void ResetEntities()
		{
			// Fill the signatures w/ a free list
			for (EntityID i = 0; i < m_Signatures.size(); i++)
			{
				// Assign each slot w/ a reference to the next slot
				m_Signatures[i] = i + 1;
			}

			// Set up free list head and tail
			m_FreeListHead = 0;
			m_Signatures[m_Signatures.size() - 1] = k_InvalidEntityID;

			// Reset entity sparse set
			m_EntitySet.Clear();
		}

	public:
		//==============================
		// Manage Entity(s)
		//==============================
		[[nodiscard]] Expected<EntityID> CreateEntity()
		{
			EntityID newEntityID{ m_FreeListHead };

			// Ensure entity is available in freelist
			if (newEntityID == k_InvalidEntityID)
			{
				return {};
			}

			// Insert the entity
			EntityID denseIndex{ m_EntitySet.InsertElement(newEntityID) };
			if (!m_EntitySet.IsValidDenseIndex(denseIndex))
			{
				return {};
			}

			KG_ASSERT(denseIndex < m_Signatures.size());

			// Update the free list and appropriate signature
			Signature& entitySignature{ m_Signatures[(EntityID)newEntityID] };
			m_FreeListHead = (EntityID)entitySignature;
			entitySignature.ClearAllFlags();

			return newEntityID;
		}

		[[nodiscard]] bool DestroyEntity(EntityID entityID)
		{
			// Destroy the entity in the sparse set
			EntityID removedDenseIndex{ m_EntitySet.DeleteElement(entityID) };

			// Check if removal failed
			if (!m_EntitySet.IsValidDenseIndex(removedDenseIndex))
			{
				return false;
			}

			KG_ASSERT(removedDenseIndex < m_Signatures.size());

			// Update the freelist
			Signature& removedSignature{ m_Signatures[entityID] };
			removedSignature = (Signature)m_FreeListHead;
			m_FreeListHead = entityID;

			return true;
		}

		[[nodiscard]] bool SetEntitySignature(EntityID entityID, Signature newSignature)
		{
			EntityID denseIndex{ m_EntitySet.GetDenseIndex(entityID) };

			// Ensure the entity is found in the sparse set
			if (!m_EntitySet.IsValidDenseIndex(denseIndex))
			{
				return false;
			}
			KG_ASSERT(denseIndex < m_Signatures.size());

			// Return the appropriate signature
			m_Signatures[denseIndex] = newSignature;

			return true;
		}

	public:
		//==============================
		// Interact w/ Other Registries
		//==============================
		void CopyRegistry(EntityRegistryTest& otherRegistry)
		{
			KG_ASSERT(m_Active);
			KG_ASSERT(otherRegistry.m_Active);

			otherRegistry.m_FreeListHead = m_FreeListHead;
			otherRegistry.m_EntitySet = m_EntitySet;
			otherRegistry.m_Signatures = m_Signatures;
		}

	public:
		//==============================
		// Query Entity(s)
		//==============================
		Expected<Signature> GetSignature(EntityID entityID)
		{
			EntityID denseIndex{ m_EntitySet.GetDenseIndex(entityID) };

			// Ensure the entity is found in the sparse set
			if (!m_EntitySet.IsValidDenseIndex(denseIndex))
			{
				return {};
			}
			KG_ASSERT(denseIndex < m_Signatures.size());

			// Return the appropriate signature
			return m_Signatures[denseIndex];
		}

		bool HasEntity(EntityID entityID) const
		{
			return m_EntitySet.HasSparseIndex(entityID);
		}

		std::span<EntityID> GetAllEntities()
		{
			return m_EntitySet.GetDenseList();
		}

	private:
		//==============================
		// Internal Fields
		//==============================
		// Registry state
		bool m_Active{ false };
		// Contained entity(s) state/info
		EntityID m_FreeListHead{ k_InvalidEntityID };
		std::array<Signature, k_MaxEntities> m_Signatures{};
		SparseSet<EntityID, EntityID> m_EntitySet{k_MaxEntities, k_MaxEntities};
	private:
		//==============================
		// Owning Class(s)
		//==============================
		friend class Registry;
	};
}