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
		static bool Has##typeName(const std::string& assetName) \
		{\
			return s_AssetsContext.m_##typeName##Manager.HasAsset(assetName); \
		}\
		static AssetHandle Create##typeName(const std::string& assetName) \
		{\
			return s_AssetsContext.m_##typeName##Manager.CreateAsset(assetName); \
		}\
		static void Serialize##typeName##Registry() \
		{\
			s_AssetsContext.m_##typeName##Manager.SerializeAssetRegistry(); \
		}\
		static void Deserialize##typeName##Registry() \
		{\
			s_AssetsContext.m_##typeName##Manager.DeserializeAssetRegistry(); \
		}\
		static void Clear##typeName##Registry() \
		{\
			s_AssetsContext.m_##typeName##Manager.ClearAssetRegistry(); \
		}\
		static std::unordered_map<AssetHandle, Asset>& Get##typeName##Registry() \
		{\
			return s_AssetsContext.m_##typeName##Manager.GetAssetRegistry(); \
		}\
		static std::unordered_map<AssetHandle, Ref<typeNamespace##::##typeName>>& Get##typeName##Cache() \
		{\
			return s_AssetsContext.m_##typeName##Manager.GetAssetCache(); \
		}\
		static void Save##typeName(AssetHandle assetHandle, Ref<typeNamespace##::##typeName> assetReference) \
		{\
			s_AssetsContext.m_##typeName##Manager.SaveAsset(assetHandle, assetReference); \
		}\
		static void Delete##typeName(AssetHandle assetHandle) \
		{\
			s_AssetsContext.m_##typeName##Manager.DeleteAsset(assetHandle); \
		}\
		static AssetHandle Import##typeName##FromFile(const std::filesystem::path& filePath) \
		{\
			return s_AssetsContext.m_##typeName##Manager.ImportAssetFromFile(filePath); \
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

		// Expose unique functionality of each class
	public:
		static void SaveEntityClass(AssetHandle entityClassHandle, Ref<Scenes::EntityClass> entityClass, Ref<Scenes::Scene> editorScene)
		{
			s_AssetsContext.m_EntityClassManager.SaveEntityClass(entityClassHandle, entityClass, editorScene);
		}
		static void DeleteEntityClass(AssetHandle handle, Ref<Scenes::Scene> editorScene)
		{
			s_AssetsContext.m_EntityClassManager.DeleteEntityClass(handle, editorScene);
		}

		static std::tuple<AssetHandle, bool> CreateNewScript(ScriptSpec& spec)
		{
			return s_AssetsContext.m_ScriptManager.CreateNewScript(spec);
		}
		static bool SaveScript(AssetHandle scriptHandle, ScriptSpec& spec)
		{
			return s_AssetsContext.m_ScriptManager.SaveScript(scriptHandle, spec);
		}
		static bool AddScriptSectionLabel(const std::string& newLabel)
		{
			return s_AssetsContext.m_ScriptManager.AddScriptSectionLabel(newLabel);
		}
		static bool EditScriptSectionLabel(const std::string& oldLabel, const std::string& newLabel)	
		{
			return s_AssetsContext.m_ScriptManager.EditScriptSectionLabel(oldLabel, newLabel);
		}
		static bool DeleteScriptSectionLabel(const std::string& label)
		{
			return s_AssetsContext.m_ScriptManager.DeleteScriptSectionLabel(label);
		}

		static AssetHandle CreateNewShader(const Rendering::ShaderSpecification& shaderSpec)
		{
			return s_AssetsContext.m_ShaderManager.CreateNewShader(shaderSpec);
		}
		static std::tuple<AssetHandle, Ref<Kargono::Rendering::Shader>> GetShader(const Rendering::ShaderSpecification& shaderSpec)
		{
			return s_AssetsContext.m_ShaderManager.GetShader(shaderSpec);
		}

		static AssetHandle ImportNewTextureFromData(Buffer buffer, int32_t width, int32_t height, int32_t channels)
		{
			return s_AssetsContext.m_Texture2DManager.ImportNewTextureFromData(buffer, width, height, channels);
		}

	private:
		static inline AssetsContext s_AssetsContext{};
	};
}
