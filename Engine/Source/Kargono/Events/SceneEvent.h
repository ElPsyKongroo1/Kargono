#pragma once

#include "Kargono/Events/Event.h"
#include "Kargono/Assets/Asset.h"

#include <sstream>

namespace Kargono::Scenes { class Scene; }

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
		ManageEntity(UUID entityID, Scenes::Scene* sceneReference, ManageEntityAction action)
			: m_EntityID(entityID), m_SceneReference(sceneReference), m_Action(action) {}

		//==============================
		// Getters/Setters
		//==============================

		UUID GetEntityID() const { return m_EntityID; }
		Scenes::Scene* GetSceneReference() const { return m_SceneReference; }
		ManageEntityAction GetAction() const { return m_Action; }

		virtual EventType GetEventType() const override { return EventType::ManageEntity; }
		virtual int GetCategoryFlags() const override { return EventCategory::Scene; }
	private:
		UUID m_EntityID;
		Scenes::Scene* m_SceneReference;
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
