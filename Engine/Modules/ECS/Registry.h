#pragma once

#include "Kargono/Core/UUID.h"

#include "Modules/ECSInternal/RegistryInternal.h"

#include <unordered_map>


namespace Kargono::ECS
{
	class Entity;

	using EntityUUIDMap = std::unordered_map<UUID, ECSInternal::EntityID>;

	struct Registry
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		Registry() = default;
		~Registry() = default;
	public:
		//==============================
		// Lifecycle Functions
		//==============================
		[[nodiscard]] bool Init(Memory::IAllocator* backingAlloc);
		[[nodiscard]] bool Terminate();

		//==============================
		// Entity Management
		//==============================
		// Entity Creation
		Entity CreateEntity(std::string_view name = "");
		Entity CreateEntityWithUUID(UUID uuid, std::string_view name = "");
		Entity DuplicateEntity(Entity entity);
		// Entity Destruction
		void DestroyEntity(Entity entity);
		void ClearEntities();
		// Entity Lookup
		Entity GetEntityByName(std::string_view name);
		Entity GetEntityByUUID(UUID uuid);
		Entity GetEntityByECSID(ECSInternal::EntityID enttID);
		bool IsEntityValid(ECSInternal::EntityID entity);

		UUID GetUUIDByName(std::string_view name);

		//==============================
		// Component Views
		//==============================
		template<typename... t_ComponentTypes>
		auto GetView()
		{
			return m_Registry.GetFlatView<t_ComponentTypes...>();
		}

		//==============================
		// Component Registration
		//==============================
		// Handle custom component(s)
		[[nodiscard]] bool RegisterCustomComponent(Assets::AssetHandle customComponentHandle);
		[[nodiscard]] bool UnRegisterCustomComponent(Assets::AssetHandle customComponentHandle);
		// Handle templated components
		template<typename t_ComponentType>
		[[nodiscard]] bool RegisterComponent()
		{
			return m_Registry.RegisterComponent<t_ComponentType>();
		}

		//==============================
		// Query State
		//==============================
		size_t GetCustomComponentCount(Assets::AssetHandle customComponentHandle);

		//==============================
		// Operator Overloads
		//==============================
		bool operator==(const Registry& other) const
		{
			return this == &other;
		}
		bool operator!=(const Registry& other) const
		{
			return this != &other;
		}
	public:
		//==============================
		// Public Fields
		//==============================
		ECSInternal::RegistryInternal m_Registry{};
		EntityUUIDMap m_EntityMap;
	};
}
