#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Assets/Asset.h"
#include "Kargono/AssetsTemp/AudioManagerTemp.h"
#include "Kargono/AssetsTemp/EntityClassManagerTemp.h"
#include "Kargono/AssetsTemp/FontManagerTemp.h"
#include "Kargono/AssetsTemp/GameStateManagerTemp.h"
#include "Kargono/AssetsTemp/InputModeManagerTemp.h"
#include "Kargono/AssetsTemp/SceneManagerTemp.h"
#include "Kargono/AssetsTemp/ScriptManagerTemp.h"
#include "Kargono/AssetsTemp/ShaderManagerTemp.h"
#include "Kargono/AssetsTemp/TextureManagerTemp.h"
#include "Kargono/AssetsTemp/UserInterfaceManagerTemp.h"

#define DEFINE_MANAGER(typeNamespace, typeName) \
		static Ref<typeNamespace##::##typeName> Get##typeName(const AssetHandle& handle) \
		{\
			return s_AssetsContext.m_##typeName##Manager.GetAsset(handle); \
		}\
		static std::tuple<AssetHandle, Ref<typeNamespace##::##typeName>> Get##typeName(const std::filesystem::path& fileLocation) \
		{\
			return s_AssetsContext.m_##typeName##Manager.GetAsset(fileLocation); \
		}\
		static std::filesystem::path Get##typeName##FileLocation(const AssetHandle& handle) \
		{\
			return s_AssetsContext.m_##typeName##Manager.GetAssetFileLocation(handle); \
		}\
		static std::filesystem::path Get##typeName##IntermediateLocation(const AssetHandle& handle) \
		{\
			return s_AssetsContext.m_##typeName##Manager.GetAssetIntermediateLocation(handle); \
		}\
		static bool Has##typeName(const AssetHandle& handle) \
		{\
			return s_AssetsContext.m_##typeName##Manager.HasAsset(handle); \
		}\
		static void Clear##typeName##Registry() \
		{\
			return s_AssetsContext.m_##typeName##Manager.ClearAssetRegistry(); \
		}

namespace Kargono::Assets
{
	struct AssetsContext
	{
		AudioBufferManager m_AudioBufferManager;
		EntityClassManager m_EntityClassManager;
		FontManager m_FontManager;
		GameStateManager m_GameStateManager;
		InputModeManager m_InputModeManager;
		SceneManager m_SceneManager;
		ScriptManager m_ScriptManager;
		ShaderManager m_ShaderManager;
		Texture2DManager m_Texture2DManager;
		UserInterfaceManager m_UserInterfaceManager;
	};

	class AssetService
	{
	public:
		// Define common functionality of each asset manager type
		DEFINE_MANAGER(Audio, AudioBuffer)
		DEFINE_MANAGER(Scenes, EntityClass)
		DEFINE_MANAGER(RuntimeUI, Font)
		DEFINE_MANAGER(Scenes, GameState)
		DEFINE_MANAGER(Input, InputMode)
		DEFINE_MANAGER(Scenes, Scene)
		DEFINE_MANAGER(Scripting, Script)
		DEFINE_MANAGER(Rendering, Shader)
		DEFINE_MANAGER(Rendering, Texture2D)
		DEFINE_MANAGER(RuntimeUI, UserInterface)
	private:
		static inline AssetsContext s_AssetsContext{};
	};
}
