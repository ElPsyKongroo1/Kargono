#include "kgpch.h"

#include "Kargono/Assets/AssetService.h"
#include "Kargono/Particles/ParticleService.h"

#include "Kargono/Assets/EmitterConfigManager.h"


namespace Kargono::Assets
{
	void EmitterConfigManager::CreateAssetFileFromName(const std::string& name, AssetInfo& asset, const std::filesystem::path& assetPath)
	{
		// Create Temporary EmitterConfig
		Ref<Particles::EmitterConfig> temporaryEmitterConfig = CreateRef<Particles::EmitterConfig>();

		// Save Binary into File
		SerializeAsset(temporaryEmitterConfig, assetPath);

		// Load data into In-Memory Metadata object
		Ref<Assets::ParticleEmitterConfigMetaData> metadata = CreateRef<Assets::ParticleEmitterConfigMetaData>();
		asset.Data.SpecificFileData = metadata;
	}
	void EmitterConfigManager::SerializeAsset(Ref<Particles::EmitterConfig> assetReference, const std::filesystem::path& assetPath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap; // Start of File Map

		out << YAML::Key << "ColorBegin" << YAML::Value << assetReference->m_ColorBegin;
		out << YAML::Key << "ColorEnd" << YAML::Value << assetReference->m_ColorEnd;

		out << YAML::Key << "SizeBegin" << YAML::Value << assetReference->m_SizeBegin;
		out << YAML::Key << "SizeEnd" << YAML::Value << assetReference->m_SizeEnd;

		out << YAML::EndMap; // End of File Map

		std::ofstream fout(assetPath);
		fout << out.c_str();
		KG_INFO("Successfully Serialized Particle Emitter Config at {}", assetPath);
	}
	Ref<Particles::EmitterConfig> EmitterConfigManager::DeserializeAsset(Assets::AssetInfo& asset, const std::filesystem::path& assetPath)
	{
		Ref<Particles::EmitterConfig> newEmitterConfig = CreateRef<Particles::EmitterConfig>();
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(assetPath.string());
		}
		catch (YAML::ParserException e)
		{
			KG_WARN("Failed to load .kgui file '{0}'\n     {1}", assetPath, e.what());
			return nullptr;
		}

		newEmitterConfig->m_ColorBegin = data["ColorBegin"].as<Math::vec4>();
		newEmitterConfig->m_ColorEnd = data["ColorEnd"].as<Math::vec4>();

		newEmitterConfig->m_SizeBegin = data["SizeBegin"].as<Math::vec3>();
		newEmitterConfig->m_SizeEnd = data["SizeEnd"].as<Math::vec3>();

		return newEmitterConfig;
	}
}
