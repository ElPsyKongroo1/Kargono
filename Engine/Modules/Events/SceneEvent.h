#pragma once

#include "Modules/Events/Event.h"
#include "Modules/Assets/Asset.h"

#include <sstream>

namespace Kargono::Scenes { class Scene; }
namespace Kargono::ECS { class Registry; }

namespace Kargono::Events
{

	enum class ManageEntityAction : uint16_t
	{
		None = 0,
		Delete,
		Create
	};
	//============================================================
	// Manage Entity Class
	//============================================================
	class ManageEntity : public Event
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================
		ManageEntity(UUID entityID, ECS::Registry* registry, ManageEntityAction action)
			: m_EntityID(entityID), m_Registry(registry), m_Action(action) {}

		//==============================
		// Getters/Setters
		//==============================

		UUID GetEntityID() const { return m_EntityID; }
		ECS::Registry* GetRegistryReference() const { return m_Registry; }
		ManageEntityAction GetAction() const { return m_Action; }

		virtual EventType GetEventType() const override { return EventType::ManageEntity; }
		virtual int GetCategoryFlags() const override { return EventCategory::Scene; }
	private:
		UUID m_EntityID;
		ECS::Registry* m_Registry;
		ManageEntityAction m_Action;
	};


	enum class ManageSceneAction : uint16_t
	{
		None = 0,
		Open
	};
	//============================================================
	// Manage Scene Class
	//============================================================
	class ManageScene : public Event
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================
		ManageScene(Assets::AssetHandle handle, ManageSceneAction action)
			: m_SceneHandle(handle), m_Action(action) {}

		//==============================
		// Getters/Setters
		//==============================

		Assets::AssetHandle GetSceneHandle() const { return m_SceneHandle; }
		ManageSceneAction GetAction() const { return m_Action; }

		virtual EventType GetEventType() const override { return EventType::ManageScene; }
		virtual int GetCategoryFlags() const override { return EventCategory::Scene; }
	private:
		Assets::AssetHandle m_SceneHandle;
		ManageSceneAction m_Action;
	};

}
