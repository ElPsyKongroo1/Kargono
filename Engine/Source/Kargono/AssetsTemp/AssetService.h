#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Assets/Asset.h"
#include "Kargono/AssetsTemp/AudioManagerTemp.h"
#include "Kargono/AssetsTemp/EntityClassManagerTemp.h"
#include "Kargono/AssetsTemp/FontManagerTemp.h"
#include "Kargono/AssetsTemp/GameStateManagerTemp.h"
#include "Kargono/AssetsTemp/InputModeManagerTemp.h"
#include "Kargono/AssetsTemp/ProjectManagerTemp.h"
#include "Kargono/AssetsTemp/SceneManagerTemp.h"
#include "Kargono/AssetsTemp/ScriptManagerTemp.h"
#include "Kargono/AssetsTemp/ShaderManagerTemp.h"
#include "Kargono/AssetsTemp/TextureManagerTemp.h"
#include "Kargono/AssetsTemp/UserInterfaceManagerTemp.h"


namespace Kargono
{
}

namespace Kargono::Assets
{

	struct AssetContext
	{
		AudioManager m_AudioManager;
		EntityClassManager m_EntityClassManager;
		FontManager m_FontManager;
		GameStateManager m_GameStateManager;
		InputModeManager m_InputModeManager;
		ProjectManager m_ProjectManager;
		SceneManager m_SceneManager;
		ScriptManager m_ScriptManager;
		ShaderManager m_ShaderManager;
		TextureManager m_TextureManager;
		UserInterfaceManager m_UserInterfaceManager;
	};

	class AssetServiceTemp
	{
	public:
		//==============================
		// Retrieve Assets
		//==============================
		static Ref<Audio::AudioBuffer> GetAudio(const AssetHandle& handle)
		{
			return s_AssetContext.m_AudioManager.GetAsset(handle);
		}
		static Ref<Scenes::EntityClass> GetEntityClass(const AssetHandle& handle)
		{
			return s_AssetContext.m_EntityClassManager.GetAsset(handle);
		}
		static Ref<RuntimeUI::Font> GetFont(const AssetHandle& handle)
		{
			return s_AssetContext.m_FontManager.GetAsset(handle);
		}
		static Ref<Scenes::GameState> GetGameState(const AssetHandle& handle)
		{
			return s_AssetContext.m_GameStateManager.GetAsset(handle);
		}
		static Ref<Input::InputMode> GetInputMode(const AssetHandle& handle)
		{
			return s_AssetContext.m_InputModeManager.GetAsset(handle);
		}
		static Ref<Scenes::Scene> GetScene(const AssetHandle& handle)
		{
			return s_AssetContext.m_SceneManager.GetAsset(handle);
		}
		static Ref<Scripting::Script> GetScript(const AssetHandle& handle)
		{
			return s_AssetContext.m_ScriptManager.GetAsset(handle);
		}
		static Ref<Kargono::Rendering::Shader> GetShader(const AssetHandle& handle)
		{
			return s_AssetContext.m_ShaderManager.GetAsset(handle);
		}
		static Ref<Rendering::Texture2D> GetTexture(const AssetHandle& handle)
		{
			return s_AssetContext.m_TextureManager.GetAsset(handle);
		}
		static Ref<RuntimeUI::UserInterface> GetUserInterface(const AssetHandle& handle)
		{
			return s_AssetContext.m_UserInterfaceManager.GetAsset(handle);
		}
		
		//==============================
		// Clear Asset Data
		//==============================
		static Ref<Audio::AudioBuffer> ClearAudioRegistry()
		{
			s_AssetContext.m_AudioManager.ClearAssetRegistry();
		}
		static Ref<Scenes::EntityClass> ClearEntityClassRegistry()
		{
			s_AssetContext.m_EntityClassManager.ClearAssetRegistry();
		}
		static Ref<RuntimeUI::Font> ClearFontRegistry()
		{
			s_AssetContext.m_FontManager.ClearAssetRegistry();
		}
		static Ref<Scenes::GameState> ClearGameStateRegistry()
		{
			s_AssetContext.m_GameStateManager.ClearAssetRegistry();
		}
		static Ref<Input::InputMode> ClearInputModeRegistry()
		{
			s_AssetContext.m_InputModeManager.ClearAssetRegistry();
		}
		static Ref<Scenes::Scene> ClearSceneRegistry()
		{
			s_AssetContext.m_SceneManager.ClearAssetRegistry();
		}
		static Ref<Scripting::Script> ClearScriptRegistry()
		{
			s_AssetContext.m_ScriptManager.ClearAssetRegistry();
		}
		static Ref<Kargono::Rendering::Shader> ClearShaderRegistry()
		{
			s_AssetContext.m_ShaderManager.ClearAssetRegistry();
		}
		static Ref<Rendering::Texture2D> ClearTextureRegistry()
		{
			s_AssetContext.m_TextureManager.ClearAssetRegistry();
		}
		static Ref<RuntimeUI::UserInterface> ClearUserInterfaceRegistry()
		{
			s_AssetContext.m_UserInterfaceManager.ClearAssetRegistry();
		}

		//==============================
		// Query Asset Metadata
		//==============================
		static std::filesystem::path GetAudioIntermediateLocation(const AssetHandle& handle)
		{
			return s_AssetContext.m_AudioManager.GetAssetIntermediateLocation(handle);
		}
		static std::filesystem::path GetEntityClassIntermediateLocation(const AssetHandle& handle)
		{
			return s_AssetContext.m_EntityClassManager.GetAssetIntermediateLocation(handle);
		}
		static std::filesystem::path GetFontIntermediateLocation(const AssetHandle& handle)
		{
			return s_AssetContext.m_FontManager.GetAssetIntermediateLocation(handle);
		}
		static std::filesystem::path GetGameStateIntermediateLocation(const AssetHandle& handle)
		{
			return s_AssetContext.m_GameStateManager.GetAssetIntermediateLocation(handle);
		}
		static std::filesystem::path GetInputModeIntermediateLocation(const AssetHandle& handle)
		{
			return s_AssetContext.m_InputModeManager.GetAssetIntermediateLocation(handle);
		}
		static std::filesystem::path GetSceneIntermediateLocation(const AssetHandle& handle)
		{
			return s_AssetContext.m_SceneManager.GetAssetIntermediateLocation(handle);
		}
		static std::filesystem::path GetScriptIntermediateLocation(const AssetHandle& handle)
		{
			return s_AssetContext.m_ScriptManager.GetAssetIntermediateLocation(handle);
		}
		static std::filesystem::path GetShaderIntermediateLocation(const AssetHandle& handle)
		{
			return s_AssetContext.m_ShaderManager.GetAssetIntermediateLocation(handle);
		}
		static std::filesystem::path GetTextureIntermediateLocation(const AssetHandle& handle)
		{
			return s_AssetContext.m_TextureManager.GetAssetIntermediateLocation(handle);
		}

		static bool HasAudio(const AssetHandle& handle)
		{
			return s_AssetContext.m_AudioManager.HasAsset(handle);
		}
		static bool HasEntityClass(const AssetHandle& handle)
		{
			return s_AssetContext.m_EntityClassManager.HasAsset(handle);
		}
		static bool HasFont(const AssetHandle& handle)
		{
			return s_AssetContext.m_FontManager.HasAsset(handle);
		}
		static bool HasGameState(const AssetHandle& handle)
		{
			return s_AssetContext.m_GameStateManager.HasAsset(handle);
		}
		static bool HasInputMode(const AssetHandle& handle)
		{
			return s_AssetContext.m_InputModeManager.HasAsset(handle);
		}
		static bool HasScene(const AssetHandle& handle)
		{
			return s_AssetContext.m_SceneManager.HasAsset(handle);
		}
		static bool HasScript(const AssetHandle& handle)
		{
			return s_AssetContext.m_ScriptManager.HasAsset(handle);
		}
		static bool HasShader(const AssetHandle& handle)
		{
			return s_AssetContext.m_ShaderManager.HasAsset(handle);
		}
		static bool HasTexture(const AssetHandle& handle)
		{
			return s_AssetContext.m_TextureManager.HasAsset(handle);
		}
		static bool HasUserInterface(const AssetHandle& handle)
		{
			return s_AssetContext.m_UserInterfaceManager.HasAsset(handle);
		}

	private:
		static inline AssetContext s_AssetContext{};
	};
}
