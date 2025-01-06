#pragma once
#include "Kargono/Assets/AssetManager.h"

namespace Kargono::Particles { struct EmitterConfig; }

namespace Kargono::Assets
{
	class EmitterConfigManager : public AssetManager<Particles::EmitterConfig>
	{
	public:
		EmitterConfigManager() : AssetManager<Particles::EmitterConfig>()
		{
			m_AssetName = "Particle Emitter Config";
			m_AssetType = AssetType::ParticleEmitterConfig;
			m_FileExtension = ".kgparticle";
			m_RegistryLocation = "ParticleEmitterConfig/ParticleEmitterConfigRegistry.kgreg";
			m_Flags.set(AssetManagerOptions::HasAssetCache, true);
			m_Flags.set(AssetManagerOptions::HasIntermediateLocation, false);
			m_Flags.set(AssetManagerOptions::HasFileLocation, true);
			m_Flags.set(AssetManagerOptions::HasFileImporting, false);
			m_Flags.set(AssetManagerOptions::HasAssetSaving, true);
			m_Flags.set(AssetManagerOptions::HasAssetCreationFromName, true);
		}
		virtual ~EmitterConfigManager() = default;
	public:

		// Class specific functions
		virtual void CreateAssetFileFromName(const std::string& name, AssetInfo& asset, const std::filesystem::path& assetPath) override;
		virtual void SerializeAsset(Ref<Particles::EmitterConfig> assetReference, const std::filesystem::path& assetPath) override;
		virtual Ref<Particles::EmitterConfig> DeserializeAsset(Assets::AssetInfo& asset, const std::filesystem::path& assetPath) override;
	};
}
