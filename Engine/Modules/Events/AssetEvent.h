#pragma once

#include "Modules/Events/Event.h"
#include "Modules/Assets/AssetsCommon.h"

#include <sstream>


namespace Kargono::Events
{

	enum class ManageAssetAction : uint16_t
	{
		None = 0,
		PreDelete,
		PostDelete,
		Create,
		UpdateAsset,
		UpdateAssetInfo
	};

	class ManageAsset : public Event
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================
		ManageAsset(UUID AssetID, Assets::AssetIdentifier identifier, ManageAssetAction action, Ref<void> providedData = nullptr)
			: m_AssetID(AssetID), m_AssetIdentifier(identifier), m_Action(action), m_ProvidedData(providedData)
		{
		
		}

		//==============================
		// Getters/Setters
		//==============================
		UUID GetAssetID() const { return m_AssetID; }
		Assets::AssetIdentifier GetAssetIdentifier() const { return m_AssetIdentifier; }
		ManageAssetAction GetAction() const { return m_Action; }
		Ref<void> GetProvidedData() const { return m_ProvidedData; }

		virtual EventType GetEventType() const override { return EventType::ManageAsset; }
		virtual int GetCategoryFlags() const override { return EventCategory::Asset; }
	private:
		UUID m_AssetID;
		Assets::AssetIdentifier m_AssetIdentifier {Assets::k_InvalidAssetIdentifier};
		ManageAssetAction m_Action{ ManageAssetAction::None };
		Ref<void> m_ProvidedData{ nullptr };
	};

}
