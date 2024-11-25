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
		UpdateAsset,
		UpdateAssetInfo
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
		ManageAsset(UUID AssetID, Assets::AssetType type, ManageAssetAction action, Ref<void> providedData = nullptr)
			: m_AssetID(AssetID), m_AssetType(type), m_Action(action), m_ProvidedData(providedData) 
		{
		
		}

		//==============================
		// Getters/Setters
		//==============================

		UUID GetAssetID() const { return m_AssetID; }
		Assets::AssetType GetAssetType() const { return m_AssetType; }
		ManageAssetAction GetAction() const { return m_Action; }
		Ref<void> GetProvidedData() const { return m_ProvidedData; }

		virtual EventType GetEventType() const override { return EventType::ManageAsset; }
		virtual int GetCategoryFlags() const override { return EventCategory::Asset; }
	private:
		UUID m_AssetID;
		Assets::AssetType m_AssetType {Assets::AssetType::None};
		ManageAssetAction m_Action;
		Ref<void> m_ProvidedData{ nullptr };
	};

}
