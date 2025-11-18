#pragma once

#include "Modules/Assets/AssetsCommon.h"
#include "Modules/Assets/AssetManager.h"
#include "Modules/Assets/Module/AssetTag.h"
#include "Kargono/Core/Base.h"

#include "Modules/AI/Assets/AIState.h"
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
		//// Deserializes all registries into memory
		//static void DeserializeAll()
		//{
		//	DeserializeShaderRegistry();
		//	DeserializeTexture2DRegistry();
		//	DeserializeAudioBufferRegistry();
		//	DeserializeFontRegistry();
		//	DeserializeScriptRegistry();
		//	DeserializeCustomComponentRegistry();
		//	DeserializeProjectEnumRegistry();
		//	DeserializeColorPaletteRegistry();
		//	DeserializeInputMapRegistry();
		//	DeserializeEmitterConfigRegistry();
		//	DeserializeGameStateRegistry();
		//	DeserializeGlobalStateRegistry();
		//	DeserializeUserInterfaceRegistry();
		//	DeserializeAIStateRegistry();
		//	DeserializeSceneRegistry();
		//}

		//// Serializes all registries into disk storage
		//static void SerializeAll()
		//{
		//	SerializeShaderRegistry();
		//	SerializeTexture2DRegistry();
		//	SerializeAudioBufferRegistry();
		//	SerializeFontRegistry();
		//	SerializeScriptRegistry();
		//	SerializeCustomComponentRegistry();
		//	SerializeProjectEnumRegistry();
		//	SerializeColorPaletteRegistry();
		//	SerializeInputMapRegistry();
		//	SerializeEmitterConfigRegistry();
		//	SerializeGameStateRegistry();
		//	SerializeGlobalStateRegistry();
		//	SerializeUserInterfaceRegistry();
		//	SerializeAIStateRegistry();
		//	SerializeSceneRegistry();
		//}

		//// Clears all Registries and In-Memory Assets
		//static void ClearAll()
		//{
		//	ClearTexture2DRegistry();
		//	ClearShaderRegistry();
		//	ClearAudioBufferRegistry();
		//	ClearFontRegistry();
		//	ClearScriptRegistry();
		//	ClearCustomComponentRegistry();
		//	ClearProjectEnumRegistry();
		//	ClearColorPaletteRegistry();
		//	ClearInputMapRegistry();
		//	ClearGameStateRegistry();
		//	ClearGlobalStateRegistry();
		//	ClearUserInterfaceRegistry();
		//	ClearEmitterConfigRegistry();
		//	ClearAIStateRegistry();
		//	ClearSceneRegistry();
		//}

		//static AssetInfo GetAssetFromAllRegistries(AssetHandle handle, AssetType type)
		//{
		//	switch (type)
		//	{
		//	case AssetType::AIState: return GetAIStateRegistry().at(handle);
		//	case AssetType::Audio: return GetAudioBufferRegistry().at(handle);
		//	case AssetType::ColorPalette: return GetColorPaletteRegistry().at(handle);
		//	case AssetType::Font: return GetFontRegistry().at(handle);
		//	case AssetType::GameState: return GetGameStateRegistry().at(handle);
		//	case AssetType::GlobalState: return GetGlobalStateRegistry().at(handle);
		//	case AssetType::InputMap: return GetInputMapRegistry().at(handle);
		//	case AssetType::Scene: return GetSceneRegistry().at(handle);
		//	case AssetType::Script: return GetScriptRegistry().at(handle);
		//	case AssetType::Shader: return GetShaderRegistry().at(handle);
		//	case AssetType::Texture: return GetTexture2DRegistry().at(handle);
		//	case AssetType::UserInterface: return GetUserInterfaceRegistry().at(handle);
		//	case AssetType::CustomComponent: return GetCustomComponentRegistry().at(handle);
		//	case AssetType::ProjectEnum: return GetProjectEnumRegistry().at(handle);
		//	case AssetType::EmitterConfig: return GetEmitterConfigRegistry().at(handle);
		//	case AssetType::None: 
		//	default:
		//		KG_ERROR("Invalid asset type provided to GetAssetFromAllRegistries function");
		//		return {};
		//	}
		//}

		static AssetRegistry* GetAssetRegistry(AssetIdentifier identifier)
		{
			switch (identifier)
			{
			case GetAssetIdentifier<AI::AIState>(): return &m_AIStateManager.GetAssetRegistry();
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
			case GetAssetIdentifier<Scripting::CustomEnum>(): return &m_ProjectEnumManager.GetAssetRegistry();
			case GetAssetIdentifier<Particles::EmitterConfig>(): return &m_EmitterConfigManager.GetAssetRegistry();
			default:
				KG_ERROR("Invalid asset type provided to GetAssetRegistry function");
				return nullptr;
			}
		}

		public:
			static inline AssetManager m_AIStateManager;
			static inline AssetManager m_AudioBufferManager;
			static inline AssetManager m_ColorPaletteManager;
			static inline AssetManager m_FontManager;
			static inline AssetManager m_GameStateManager;
			static inline AssetManager m_GlobalStateManager;
			static inline AssetManager m_InputMapManager;
			static inline AssetManager m_CustomComponentManager;
			static inline AssetManager m_ProjectEnumManager;
			static inline AssetManager m_SceneManager;
			static inline AssetManager m_ScriptManager;
			static inline AssetManager m_ShaderManager;
			static inline AssetManager m_Texture2DManager;
			static inline AssetManager m_UserInterfaceManager;
			static inline AssetManager m_EmitterConfigManager;
	};
}
