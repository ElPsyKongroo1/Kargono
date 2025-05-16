#include "kgpch.h"

#include "Modules/Assets/AssetService.h"
#include "Modules/Particles/ParticleService.h"

#include "Modules/Assets/EmitterConfigManager.h"


namespace Kargono::Assets
{
	void EmitterConfigManager::CreateAssetFileFromName(std::string_view name, AssetInfo& asset, const std::filesystem::path& assetPath)
	{
		UNREFERENCED_PARAMETER(name);

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

		// Emitter specific data
		out << YAML::Key << "BufferSize" << YAML::Value << assetReference->m_BufferSize;
		out << YAML::Key << "SpawnRatePerSec" << YAML::Value << assetReference->m_SpawnRatePerSec;
		out << YAML::Key << "EmitterLifecycle" << YAML::Value <<
			Utility::EmitterLifecycleToString(assetReference->m_EmitterLifecycle);
		out << YAML::Key << "EmitterLifetime" << YAML::Value << assetReference->m_EmitterLifetime;

		// Spawning bounds sequence
		out << YAML::Key << "SpawningBounds" << YAML::Value << YAML::BeginSeq; // Start spawning bounds sequence
		out << YAML::Value << assetReference->m_SpawningBounds[0];
		out << YAML::Value << assetReference->m_SpawningBounds[1];
		out << YAML::EndSeq; // End of spawning bounds sequence

		// Particle specific data
		out << YAML::Key << "UseGravity" << YAML::Value << assetReference->m_UseGravity;
		out << YAML::Key << "GravityAcceleration" << YAML::Value << assetReference->m_GravityAcceleration;
		out << YAML::Key << "ParticleLifetime" << YAML::Value << assetReference->m_ParticleLifetime;
		out << YAML::Key << "ColorInterpolationType" << YAML::Value << 
			Utility::InterpolationTypeToString(assetReference->m_ColorInterpolationType);
		out << YAML::Key << "ColorBegin" << YAML::Value << assetReference->m_ColorBegin;
		out << YAML::Key << "ColorEnd" << YAML::Value << assetReference->m_ColorEnd;

		out << YAML::Key << "SizeInterpolationType" << YAML::Value << 
			Utility::InterpolationTypeToString(assetReference->m_SizeInterpolationType);
		out << YAML::Key << "SizeBegin" << YAML::Value << assetReference->m_SizeBegin;
		out << YAML::Key << "SizeEnd" << YAML::Value << assetReference->m_SizeEnd;

		out << YAML::EndMap; // End of File Map

		std::ofstream fout(assetPath);
		fout << out.c_str();
		KG_INFO("Successfully Serialized Particle Emitter Config at {}", assetPath);
	}
	Ref<Particles::EmitterConfig> EmitterConfigManager::DeserializeAsset(Assets::AssetInfo& asset, const std::filesystem::path& assetPath)
	{
		UNREFERENCED_PARAMETER(asset);

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

		// Get emitter specific data from YAML
		newEmitterConfig->m_BufferSize = data["BufferSize"].as<size_t>();
		newEmitterConfig->m_SpawnRatePerSec = data["SpawnRatePerSec"].as<size_t>();
		newEmitterConfig->m_EmitterLifecycle =
			Utility::StringToEmitterLifecycle(data["EmitterLifecycle"].as<std::string>());
		newEmitterConfig->m_EmitterLifetime = data["EmitterLifetime"].as<float>();
		YAML::Node spawningBoundsNode = data["SpawningBounds"];
		size_t iteration{ 0 };
		for (YAML::iterator::value_type bound : spawningBoundsNode)
		{
			newEmitterConfig->m_SpawningBounds[iteration] = bound.as<Math::vec3>();
			iteration++;
		}

		// Get particle specific data from YAML
		newEmitterConfig->m_UseGravity = data["UseGravity"].as<bool>();
		newEmitterConfig->m_GravityAcceleration = data["GravityAcceleration"].as<Math::vec3>();
		newEmitterConfig->m_ParticleLifetime = data["ParticleLifetime"].as<float>();

		// Get particle color data from YAML
		newEmitterConfig->m_ColorInterpolationType = 
			Utility::StringToInterpolationType(data["ColorInterpolationType"].as<std::string>());
		newEmitterConfig->m_ColorBegin = data["ColorBegin"].as<Math::vec4>();
		newEmitterConfig->m_ColorEnd = data["ColorEnd"].as<Math::vec4>();

		// Get particle size data from YAML
		newEmitterConfig->m_SizeInterpolationType =
			Utility::StringToInterpolationType(data["SizeInterpolationType"].as<std::string>());
		newEmitterConfig->m_SizeBegin = data["SizeBegin"].as<Math::vec3>();
		newEmitterConfig->m_SizeEnd = data["SizeEnd"].as<Math::vec3>();

		return newEmitterConfig;
	}
	void EmitterConfigManager::DeleteAssetValidation(AssetHandle emitterConfigHandle)
	{
		// Ensure all other assets do not contain this emitter config
		// If they do, remove the reference

		// Check scene assets
		for (auto& [sceneHandle, assetInfo] : Assets::AssetService::GetSceneRegistry())
		{
			// Handle UI level function pointers
			Ref<Scenes::Scene> sceneRef = Assets::AssetService::GetScene(sceneHandle);

			bool sceneModified = Assets::AssetService::RemoveEmitterConfigFromScene(sceneRef, emitterConfigHandle);
			if (sceneModified)
			{
				// Save scene
				Assets::AssetService::SaveScene(sceneHandle, sceneRef);
			}
		}
	}
}
