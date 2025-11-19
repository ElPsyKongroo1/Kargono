#include "kgpch.h"

#include "Modules/Particles/Assets/EmitterConfig.h"
#include "Modules/Scenes/Assets/Scene.h"

namespace Kargono::Particles
{
	void EmitterConfig::Serialize(void* context)
	{
		// Get asset context
		KG_ASSERT(context, "Context cannot be null");
		Assets::SerializeAssetContext* assetContext = (Assets::SerializeAssetContext*)context;
		KG_ASSERT(assetContext, "Context cannot be null");
		Assets::Metadata* metadata{ assetContext->m_AssetMetadata };
		KG_ASSERT(metadata, "Metadata cannot be null");

		// Get context fields
		const std::filesystem::path& assetPath{ metadata->GetAssetFullFilePath<EmitterConfig>() };

		YAML::Emitter out;
		out << YAML::BeginMap; // Start of File Map

		// Emitter specific data
		out << YAML::Key << "BufferSize" << YAML::Value << m_BufferSize;
		out << YAML::Key << "SpawnRatePerSec" << YAML::Value << m_SpawnRatePerSec;
		out << YAML::Key << "EmitterLifecycle" << YAML::Value <<
			Utility::EmitterLifecycleToString(m_EmitterLifecycle);
		out << YAML::Key << "EmitterLifetime" << YAML::Value << m_EmitterLifetime;

		// Spawning bounds sequence
		out << YAML::Key << "SpawningBounds" << YAML::Value << YAML::BeginSeq; // Start spawning bounds sequence
		out << YAML::Value << m_SpawningBounds[0];
		out << YAML::Value << m_SpawningBounds[1];
		out << YAML::EndSeq; // End of spawning bounds sequence

		// Particle specific data
		out << YAML::Key << "UseGravity" << YAML::Value << m_UseGravity;
		out << YAML::Key << "GravityAcceleration" << YAML::Value << m_GravityAcceleration;
		out << YAML::Key << "ParticleLifetime" << YAML::Value << m_ParticleLifetime;
		out << YAML::Key << "ColorInterpolationType" << YAML::Value <<
			Utility::InterpolationTypeToString(m_ColorInterpolationType);
		out << YAML::Key << "ColorBegin" << YAML::Value << m_ColorBegin;
		out << YAML::Key << "ColorEnd" << YAML::Value << m_ColorEnd;

		out << YAML::Key << "SizeInterpolationType" << YAML::Value <<
			Utility::InterpolationTypeToString(m_SizeInterpolationType);
		out << YAML::Key << "SizeBegin" << YAML::Value << m_SizeBegin;
		out << YAML::Key << "SizeEnd" << YAML::Value << m_SizeEnd;

		out << YAML::EndMap; // End of File Map

		std::ofstream fout(assetPath);
		fout << out.c_str();
		KG_INFO("Successfully Serialized Particle Emitter Config at {}", assetPath);
	}

	void EmitterConfig::Deserialize(void* context)
	{
		// Get context
		Assets::DeserializeAssetContext* deserializeContext = (Assets::DeserializeAssetContext*)context;
		KG_ASSERT(deserializeContext, "Context cannot be null");
		Assets::Metadata* metadata{ deserializeContext->m_AssetMetadata };
		KG_ASSERT(metadata, "Metadata cannot be null");

		// Get asset path
		const std::filesystem::path& assetPath = metadata->GetAssetFullFilePath<EmitterConfig>();

		YAML::Node data;
		try
		{
			data = YAML::LoadFile(assetPath.string());
		}
		catch (YAML::ParserException e)
		{
			KG_WARN("Failed to load .kgui file '{0}'\n     {1}", assetPath, e.what());
			return;
		}

		// Get emitter specific data from YAML
		m_BufferSize = data["BufferSize"].as<size_t>();
		m_SpawnRatePerSec = data["SpawnRatePerSec"].as<size_t>();
		m_EmitterLifecycle =
			Utility::StringToEmitterLifecycle(data["EmitterLifecycle"].as<std::string>());
		m_EmitterLifetime = data["EmitterLifetime"].as<float>();
		YAML::Node spawningBoundsNode = data["SpawningBounds"];
		size_t iteration{ 0 };
		for (YAML::iterator::value_type bound : spawningBoundsNode)
		{
			m_SpawningBounds[iteration] = bound.as<Math::vec3>();
			iteration++;
		}

		// Get particle specific data from YAML
		m_UseGravity = data["UseGravity"].as<bool>();
		m_GravityAcceleration = data["GravityAcceleration"].as<Math::vec3>();
		m_ParticleLifetime = data["ParticleLifetime"].as<float>();

		// Get particle color data from YAML
		m_ColorInterpolationType =
			Utility::StringToInterpolationType(data["ColorInterpolationType"].as<std::string>());
		m_ColorBegin = data["ColorBegin"].as<Math::vec4>();
		m_ColorEnd = data["ColorEnd"].as<Math::vec4>();

		// Get particle size data from YAML
		m_SizeInterpolationType =
			Utility::StringToInterpolationType(data["SizeInterpolationType"].as<std::string>());
		m_SizeBegin = data["SizeBegin"].as<Math::vec3>();
		m_SizeEnd = data["SizeEnd"].as<Math::vec3>();
	}

	void EmitterConfig::ValidateDelete(Assets::Metadata& metadata)
	{
		// Ensure all other assets do not contain this emitter config
		// If they do, remove the reference

		// Check scene assets
		for (auto& [sceneHandle, assetInfo] : Assets::AssetService::m_SceneManager.GetAssetRegistry())
		{
			// Handle UI level function pointers
			Assets::AssetRef<Scenes::Scene> sceneRef = Assets::AssetService::m_SceneManager.GetAssetByHandle(sceneHandle);

			bool sceneModified = sceneRef->RemoveEmitterConfig(metadata.m_Handle);
			if (sceneModified)
			{
				// Save scene
				Assets::AssetService::m_SceneManager.UpdateAsset(sceneRef);
			}
		}
	}

	void EmitterConfig::CreateFromName(Assets::Metadata& metadata)
	{
		// Create default emitter config
		EmitterConfig defaultConfig{};

		// Save binary into file
		Assets::SerializeAssetContext context {&metadata};
		defaultConfig.Serialize((void*)&context);
	}
}