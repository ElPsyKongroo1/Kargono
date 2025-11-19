#pragma once

#include "Modules/Assets/AssetsCommon.h"
#include "Modules/Assets/AssetManager.h"
#include "Modules/Assets/Module/AssetTag.h"
#include "Kargono/Core/Base.h"

#include "Modules/States/Assets/State.h"
#include "Modules/Audio/Assets/AudioBuffer.h"
#include "Modules/EditorUI/Assets/ColorPalette.h"
#include "Modules/RuntimeUI/Assets/Font.h"
#include "Modules/GlobalState/Assets/GlobalState.h"
#include "Modules/GlobalState/Assets/GameState.h"
#include "Modules/InputMap/Assets/InputMap.h"
#include "Modules/Scenes/Assets/Scene.h"
#include "Modules/Scripting/Assets/Script.h"
#include "Modules/Rendering/Assets/Shader.h"
#include "Modules/Rendering/Assets/Texture2D.h"
#include "Modules/RuntimeUI/Assets/RuntimeUIUserInterface.h"
#include "Modules/ECSInternal/Assets/CustomComponent.h"
#include "Modules/Scripting/Assets/CustomEnum.h"
#include "Modules/Particles/Assets/EmitterConfig.h"


namespace Kargono::Assets
{
	class AssetService
	{
	public:
		// Deserializes all registries into memory
		static void DeserializeAll()
		{
			m_ShaderManager.DeserializeAssetRegistry();
			m_Texture2DManager.DeserializeAssetRegistry();
			m_AudioBufferManager.DeserializeAssetRegistry();
			m_FontManager.DeserializeAssetRegistry();
			m_ScriptManager.DeserializeAssetRegistry();
			m_CustomComponentManager.DeserializeAssetRegistry();
			m_CustomEnumManager.DeserializeAssetRegistry();
			m_ColorPaletteManager.DeserializeAssetRegistry();
			m_InputMapManager.DeserializeAssetRegistry();
			m_EmitterConfigManager.DeserializeAssetRegistry();
			m_GameStateManager.DeserializeAssetRegistry();
			m_GlobalStateManager.DeserializeAssetRegistry();
			m_UserInterfaceManager.DeserializeAssetRegistry();
			m_StateManager.DeserializeAssetRegistry();
			m_SceneManager.DeserializeAssetRegistry(); 
		}

		// Serializes all registries into disk storage
		static void SerializeAll()
		{
			m_ShaderManager.SerializeAssetRegistry ();
			m_Texture2DManager.SerializeAssetRegistry();
			m_AudioBufferManager.SerializeAssetRegistry();
			m_FontManager.SerializeAssetRegistry();
			m_ScriptManager.SerializeAssetRegistry();
			m_CustomComponentManager.SerializeAssetRegistry();
			m_CustomEnumManager.SerializeAssetRegistry();
			m_ColorPaletteManager.SerializeAssetRegistry();
			m_InputMapManager.SerializeAssetRegistry();
			m_EmitterConfigManager.SerializeAssetRegistry();
			m_GameStateManager.SerializeAssetRegistry();
			m_GlobalStateManager.SerializeAssetRegistry();
			m_UserInterfaceManager.SerializeAssetRegistry();
			m_StateManager.SerializeAssetRegistry();
			m_SceneManager.SerializeAssetRegistry();
		}

		// Clears all Registries and In-Memory Assets
		static void ClearAll()
		{
			m_ShaderManager.ClearAssetRegistry();
			m_Texture2DManager.ClearAssetRegistry();
			m_AudioBufferManager.ClearAssetRegistry();
			m_FontManager.ClearAssetRegistry();
			m_ScriptManager.ClearAssetRegistry();
			m_CustomComponentManager.ClearAssetRegistry();
			m_CustomEnumManager.ClearAssetRegistry();
			m_ColorPaletteManager.ClearAssetRegistry();
			m_InputMapManager.ClearAssetRegistry();
			m_EmitterConfigManager.ClearAssetRegistry();
			m_GameStateManager.ClearAssetRegistry();
			m_GlobalStateManager.ClearAssetRegistry();
			m_UserInterfaceManager.ClearAssetRegistry();
			m_StateManager.ClearAssetRegistry();
			m_SceneManager.ClearAssetRegistry();
		}

		static Metadata GetAssetFromAllRegistries(AssetHandle handle, AssetIdentifier identifier)
		{
			switch (identifier)
			{
			case GetAssetIdentifier<States::State>(): return m_StateManager.GetAssetRegistry().at(handle);
			case GetAssetIdentifier<Audio::AudioBuffer>(): return m_AudioBufferManager.GetAssetRegistry().at(handle);
			case GetAssetIdentifier<EditorUI::ColorPalette>(): return m_ColorPaletteManager.GetAssetRegistry().at(handle);
			case GetAssetIdentifier<RuntimeUI::Font>(): return m_FontManager.GetAssetRegistry().at(handle);
			case GetAssetIdentifier<GlobalState::GameState>(): return m_GameStateManager.GetAssetRegistry().at(handle);
			case GetAssetIdentifier<GlobalState::GlobalState>(): return m_GlobalStateManager.GetAssetRegistry().at(handle);
			case GetAssetIdentifier<InputMap::InputMap>(): return m_InputMapManager.GetAssetRegistry().at(handle);
			case GetAssetIdentifier<Scenes::Scene>(): return m_SceneManager.GetAssetRegistry().at(handle);
			case GetAssetIdentifier<Scripting::Script>(): return m_ScriptManager.GetAssetRegistry().at(handle);
			case GetAssetIdentifier<Rendering::Shader>(): return m_ShaderManager.GetAssetRegistry().at(handle);
			case GetAssetIdentifier<Rendering::Texture2D>(): return m_ShaderManager.GetAssetRegistry().at(handle);
			case GetAssetIdentifier<RuntimeUI::UserInterface>(): return m_UserInterfaceManager.GetAssetRegistry().at(handle);
			case GetAssetIdentifier<ECSInternal::CustomComponent>(): return m_CustomComponentManager.GetAssetRegistry().at(handle);
			case GetAssetIdentifier<Scripting::CustomEnum>(): return m_CustomEnumManager.GetAssetRegistry().at(handle);
			case GetAssetIdentifier<Particles::EmitterConfig>(): return m_EmitterConfigManager.GetAssetRegistry().at(handle);
			default:
				KG_ERROR("Invalid asset type provided to GetAssetFromAllRegistry function");
				return {};
			}
		}

		static AssetRegistry* GetAssetRegistry(AssetIdentifier identifier)
		{
			switch (identifier)
			{
			case GetAssetIdentifier<States::State>(): return &m_StateManager.GetAssetRegistry();
			case GetAssetIdentifier<Audio::AudioBuffer>(): return &m_AudioBufferManager.GetAssetRegistry();
			case GetAssetIdentifier<EditorUI::ColorPalette>(): return &m_ColorPaletteManager.GetAssetRegistry();
			case GetAssetIdentifier<RuntimeUI::Font>(): return &m_FontManager.GetAssetRegistry();
			case GetAssetIdentifier<GlobalState::GameState>(): return &m_GameStateManager.GetAssetRegistry();
			case GetAssetIdentifier<GlobalState::GlobalState>(): return &m_GlobalStateManager.GetAssetRegistry();
			case GetAssetIdentifier<InputMap::InputMap>(): return &m_InputMapManager.GetAssetRegistry();
			case GetAssetIdentifier<Scenes::Scene>(): return &m_SceneManager.GetAssetRegistry();
			case GetAssetIdentifier<Scripting::Script>(): return &m_ScriptManager.GetAssetRegistry();
			case GetAssetIdentifier<Rendering::Shader>(): return &m_ShaderManager.GetAssetRegistry();
			case GetAssetIdentifier<Rendering::Texture2D>(): return &m_ShaderManager.GetAssetRegistry();
			case GetAssetIdentifier<RuntimeUI::UserInterface>(): return &m_UserInterfaceManager.GetAssetRegistry();
			case GetAssetIdentifier<ECSInternal::CustomComponent>(): return &m_CustomComponentManager.GetAssetRegistry();
			case GetAssetIdentifier<Scripting::CustomEnum>(): return &m_CustomEnumManager.GetAssetRegistry();
			case GetAssetIdentifier<Particles::EmitterConfig>(): return &m_EmitterConfigManager.GetAssetRegistry();
			default:
				KG_ERROR("Invalid asset type provided to GetAssetRegistry function");
				return nullptr;
			}
		}

		public:
			static inline AssetManager<States::State> m_StateManager;
			static inline AssetManager<Audio::AudioBuffer> m_AudioBufferManager;
			static inline AssetManager<EditorUI::ColorPalette> m_ColorPaletteManager;
			static inline AssetManager<RuntimeUI::Font> m_FontManager;
			static inline AssetManager<GlobalState::GameState> m_GameStateManager;
			static inline AssetManager<GlobalState::GlobalState> m_GlobalStateManager;
			static inline AssetManager<InputMap::InputMap> m_InputMapManager;
			static inline AssetManager<ECSInternal::CustomComponent> m_CustomComponentManager;
			static inline AssetManager<Scripting::CustomEnum> m_CustomEnumManager;
			static inline AssetManager<Scenes::Scene> m_SceneManager;
			static inline AssetManager<Scripting::Script> m_ScriptManager;
			static inline AssetManager<Rendering::Shader> m_ShaderManager;
			static inline AssetManager<Rendering::Texture2D> m_Texture2DManager;
			static inline AssetManager<RuntimeUI::UserInterface> m_UserInterfaceManager;
			static inline AssetManager<Particles::EmitterConfig> m_EmitterConfigManager;
	};
}
