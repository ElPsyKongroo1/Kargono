#pragma once

#include "Modules/Assets/AssetsCommon.h"
#include "Modules/Assets/AssetManager.h"
#include "Modules/Assets/Module/AssetTag.h"
#include "Kargono/Core/Base.h"

#include "Modules/Assets/Managers/StateManager.h"
#include "Modules/Assets/Managers/AudioBufferManager.h"
#include "Modules/Assets/Managers/ColorPaletteManager.h"
#include "Modules/Assets/Managers/FontManager.h"
#include "Modules/Assets/Managers/GlobalStateManager.h"
#include "Modules/Assets/Managers/GameStateManager.h"
#include "Modules/Assets/Managers/InputMapManager.h"
#include "Modules/Assets/Managers/SceneManager.h"
#include "Modules/Assets/Managers/ScriptManager.h"
#include "Modules/Assets/Managers/ShaderManager.h"
#include "Modules/Assets/Managers/Texture2DManager.h"
#include "Modules/Assets/Managers/RuntimeUIManager.h"
#include "Modules/Assets/Managers/CustomComponentManager.h"
#include "Modules/Assets/Managers/CustomEnumManager.h"
#include "Modules/Assets/Managers/EmitterConfigManager.h"


namespace Kargono::Assets
{
	class AssetService
	{
	public:
		// Deserializes all registries into memory
		static void DeserializeAll()
		{
			s_ShaderManager.DeserializeAssetRegistry();
			s_Texture2DManager.DeserializeAssetRegistry();
			s_AudioBufferManager.DeserializeAssetRegistry();
			s_FontManager.DeserializeAssetRegistry();
			s_ScriptManager.DeserializeAssetRegistry();
			s_CustomComponentManager.DeserializeAssetRegistry();
			s_CustomEnumManager.DeserializeAssetRegistry();
			s_ColorPaletteManager.DeserializeAssetRegistry();
			s_InputMapManager.DeserializeAssetRegistry();
			s_EmitterConfigManager.DeserializeAssetRegistry();
			s_GameStateManager.DeserializeAssetRegistry();
			s_GlobalStateManager.DeserializeAssetRegistry();
			s_UserInterfaceManager.DeserializeAssetRegistry();
			s_StateManager.DeserializeAssetRegistry();
			s_SceneManager.DeserializeAssetRegistry(); 
		}

		// Serializes all registries into disk storage
		static void SerializeAll()
		{
			s_ShaderManager.SerializeAssetRegistry ();
			s_Texture2DManager.SerializeAssetRegistry();
			s_AudioBufferManager.SerializeAssetRegistry();
			s_FontManager.SerializeAssetRegistry();
			s_ScriptManager.SerializeAssetRegistry();
			s_CustomComponentManager.SerializeAssetRegistry();
			s_CustomEnumManager.SerializeAssetRegistry();
			s_ColorPaletteManager.SerializeAssetRegistry();
			s_InputMapManager.SerializeAssetRegistry();
			s_EmitterConfigManager.SerializeAssetRegistry();
			s_GameStateManager.SerializeAssetRegistry();
			s_GlobalStateManager.SerializeAssetRegistry();
			s_UserInterfaceManager.SerializeAssetRegistry();
			s_StateManager.SerializeAssetRegistry();
			s_SceneManager.SerializeAssetRegistry();
		}

		// Clears all Registries and In-Memory Assets
		static void ClearAll()
		{
			s_ShaderManager.ClearAssetRegistry();
			s_Texture2DManager.ClearAssetRegistry();
			s_AudioBufferManager.ClearAssetRegistry();
			s_FontManager.ClearAssetRegistry();
			s_ScriptManager.ClearAssetRegistry();
			s_CustomComponentManager.ClearAssetRegistry();
			s_CustomEnumManager.ClearAssetRegistry();
			s_ColorPaletteManager.ClearAssetRegistry();
			s_InputMapManager.ClearAssetRegistry();
			s_EmitterConfigManager.ClearAssetRegistry();
			s_GameStateManager.ClearAssetRegistry();
			s_GlobalStateManager.ClearAssetRegistry();
			s_UserInterfaceManager.ClearAssetRegistry();
			s_StateManager.ClearAssetRegistry();
			s_SceneManager.ClearAssetRegistry();
		}

		template<AssetConcept t_AssetType>
		static Metadata<t_AssetType> GetAssetFromAllRegistries(AssetHandle handle, AssetIdentifier identifier)
		{
			switch (identifier)
			{
			case GetAssetIdentifier<States::State>(): return s_StateManager.GetAssetRegistry().at(handle);
			case GetAssetIdentifier<Audio::AudioBuffer>(): return s_AudioBufferManager.GetAssetRegistry().at(handle);
			case GetAssetIdentifier<EditorUI::ColorPalette>(): return s_ColorPaletteManager.GetAssetRegistry().at(handle);
			case GetAssetIdentifier<RuntimeUI::Font>(): return s_FontManager.GetAssetRegistry().at(handle);
			case GetAssetIdentifier<GlobalState::GameState>(): return s_GameStateManager.GetAssetRegistry().at(handle);
			case GetAssetIdentifier<GlobalState::GlobalState>(): return s_GlobalStateManager.GetAssetRegistry().at(handle);
			case GetAssetIdentifier<InputMap::InputMap>(): return s_InputMapManager.GetAssetRegistry().at(handle);
			case GetAssetIdentifier<Scenes::Scene>(): return s_SceneManager.GetAssetRegistry().at(handle);
			case GetAssetIdentifier<Scripting::Script>(): return s_ScriptManager.GetAssetRegistry().at(handle);
			case GetAssetIdentifier<Rendering::Shader>(): return s_ShaderManager.GetAssetRegistry().at(handle);
			case GetAssetIdentifier<Rendering::Texture2D>(): return s_ShaderManager.GetAssetRegistry().at(handle);
			case GetAssetIdentifier<RuntimeUI::UserInterface>(): return s_UserInterfaceManager.GetAssetRegistry().at(handle);
			case GetAssetIdentifier<ECSInternal::CustomComponent>(): return s_CustomComponentManager.GetAssetRegistry().at(handle);
			case GetAssetIdentifier<Scripting::CustomEnum>(): return s_CustomEnumManager.GetAssetRegistry().at(handle);
			case GetAssetIdentifier<Particles::EmitterConfig>(): return s_EmitterConfigManager.GetAssetRegistry().at(handle);
			default:
				KG_ERROR("Invalid asset type provided to GetAssetFromAllRegistry function");
				return {};
			}
		}

		template<AssetConcept t_AssetType>
		static AssetRegistry<t_AssetType>* GetAssetRegistry(AssetIdentifier identifier)
		{
			switch (identifier)
			{
			case GetAssetIdentifier<States::State>(): return &s_StateManager.GetAssetRegistry();
			case GetAssetIdentifier<Audio::AudioBuffer>(): return &s_AudioBufferManager.GetAssetRegistry();
			case GetAssetIdentifier<EditorUI::ColorPalette>(): return &s_ColorPaletteManager.GetAssetRegistry();
			case GetAssetIdentifier<RuntimeUI::Font>(): return &s_FontManager.GetAssetRegistry();
			case GetAssetIdentifier<GlobalState::GameState>(): return &s_GameStateManager.GetAssetRegistry();
			case GetAssetIdentifier<GlobalState::GlobalState>(): return &s_GlobalStateManager.GetAssetRegistry();
			case GetAssetIdentifier<InputMap::InputMap>(): return &s_InputMapManager.GetAssetRegistry();
			case GetAssetIdentifier<Scenes::Scene>(): return &s_SceneManager.GetAssetRegistry();
			case GetAssetIdentifier<Scripting::Script>(): return &s_ScriptManager.GetAssetRegistry();
			case GetAssetIdentifier<Rendering::Shader>(): return &s_ShaderManager.GetAssetRegistry();
			case GetAssetIdentifier<Rendering::Texture2D>(): return &s_ShaderManager.GetAssetRegistry();
			case GetAssetIdentifier<RuntimeUI::UserInterface>(): return &s_UserInterfaceManager.GetAssetRegistry();
			case GetAssetIdentifier<ECSInternal::CustomComponent>(): return &s_CustomComponentManager.GetAssetRegistry();
			case GetAssetIdentifier<Scripting::CustomEnum>(): return &s_CustomEnumManager.GetAssetRegistry();
			case GetAssetIdentifier<Particles::EmitterConfig>(): return &s_EmitterConfigManager.GetAssetRegistry();
			default:
				KG_ERROR("Invalid asset type provided to GetAssetRegistry function");
				return nullptr;
			}
		}
	};
}
