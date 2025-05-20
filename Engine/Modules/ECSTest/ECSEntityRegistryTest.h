#pragma once

#include "ECSCommonTest.h"
#include "Kargono/Core/Base.h"

#include <array>
#include <queue>

namespace Kargono::ECS
{
	class EntityRegistry
	{	
	private:
		//==============================
		// Constructors/Destructors
		//==============================
		EntityRegistry() = default;
		~EntityRegistry() = default;
	public:
		//==============================
		// Lifecycle Functions
		//==============================
		[[nodiscard]] bool Init()
		{
			// Fill available entities buffer
			for (EntityID entityID{ 0 }; entityID < k_MaxEntities; entityID++)
			{
				m_AvailableEntites.push(entityID);
			}

			m_Active = true;
			return m_Active;
		}

		//==============================
		// Manage Entity(s)
		//==============================
		[[nodiscard]] Expected<EntityID> CreateEntity()
		{
			// Check for too many entities
			if (m_ActiveEntityCount >= k_MaxEntities)
			{
				return {};
			}

			// Pop the closest ID
			EntityID id = m_AvailableEntites.front();
			m_AvailableEntites.pop();
			m_ActiveEntityCount++;

			return id;
		}

		[[nodiscard]] bool DestroyEntity(EntityID entityID)
		{
			KG_ASSERT(m_ActiveEntityCount > 0);

			// Check out of bounds index
			if (entityID >= k_MaxEntities)
			{
				return false;
			}

			// Clear entity signature
			m_Signatures[entityID].reset();

			// Move the destroyed ID to the back of the queue
			m_AvailableEntites.push(entityID);
			m_ActiveEntityCount--;

			return true;
		}

		[[nodiscard]] bool SetEntitySignature(EntityID entityID, Signature newSignature)
		{
			// Check out of bounds index
			if (entityID >= k_MaxEntities)
			{
				return false;
			}

			// Update signature
			m_Signatures[entityID] = newSignature;

			return true;
		}

		//==============================
		// Query Entity(s)
		//==============================
		Expected<Signature> GetSignature(EntityID entityID)
		{
			// Check out of bounds index
			if (entityID >= k_MaxEntities)
			{
				return {};
			}

			return m_Signatures[entityID];
		}

	private:
		//==============================
		// Internal Fields
		//==============================
		// Registry state
		bool m_Active{ false };
		// Contained entity(s) state/info
		std::queue<EntityID> m_AvailableEntites{}; // TODO: Maybe another data structure?
		std::array<Signature, k_MaxEntities> m_Signatures{};
		EntityCount m_ActiveEntityCount{ 0 };

	private:
		//==============================
		// Owning Class(s)
		//==============================
		friend class Registry;
	};
}