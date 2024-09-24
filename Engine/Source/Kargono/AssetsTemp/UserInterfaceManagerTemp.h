#pragma once
#include "Kargono/AssetsTemp/AssetManagerTemp.h"

namespace Kargono::Assets
{
	class UserInterfaceManager : public AssetManagerTemp<RuntimeUI::UserInterface>
	{
	public:
		UserInterfaceManager() : AssetManagerTemp<RuntimeUI::UserInterface>()
		{
			m_Flags.set(AssetManagerOptions::UseRuntimeCache, false);
		}
		virtual ~UserInterfaceManager() = default;
	public:
		// Override virtual functions
		virtual Ref<RuntimeUI::UserInterface> InstantiateAssetIntoMemory(Assets::Asset& asset) override;

		// Functions specific to this manager type
		bool DeserializeUserInterface(Ref<RuntimeUI::UserInterface> userInterface, const std::filesystem::path& filepath);
	};
}
