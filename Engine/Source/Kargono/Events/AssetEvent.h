#pragma once

#include "Kargono/Events/Event.h"
#include "Kargono/Assets/Asset.h"

#include <sstream>


namespace Kargono::Events
{

	enum class ManageAssetAction : uint16_t
	{
		None = 0,
		Delete,
		Create,
		Update
	};
	//============================================================
	// Manage Asset Class
	//============================================================
	class ManageAsset : public Event
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================
		ManageAsset(UUID AssetID, Assets::AssetType type, ManageAssetAction action)
			: m_AssetID(AssetID), m_AssetType(type), m_Action(action) {}

		//==============================
		// Getters/Setters
		//==============================

		UUID GetAssetID() const { return m_AssetID; }
		Assets::AssetType GetAssetType() const { return m_AssetType; }
		ManageAssetAction GetAction() const { return m_Action; }

		virtual EventType GetEventType() const override { return EventType::ManageAsset; }
		virtual int GetCategoryFlags() const override { return EventCategory::Asset; }
	private:
		UUID m_AssetID;
		Assets::AssetType m_AssetType {Assets::AssetType::None};
		ManageAssetAction m_Action;
	};

}
