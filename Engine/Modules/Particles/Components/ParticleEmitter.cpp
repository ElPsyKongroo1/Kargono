#include "kgpch.h"

#include "Modules/Particles/Components/ParticleEmitter.h"
#include "Modules/Assets/AssetService.h"

namespace Kargono::Particles
{
	void ParticleEmitter::Serialize(void* context)
	{
		// Get context
		ECSInternal::SerializeComponentContext* serializeContext =
			(ECSInternal::SerializeComponentContext*)context;
		KG_ASSERT(serializeContext, "Context cannot be null");
		KG_ASSERT(serializeContext->m_Serializer, "Serializer cannot be null");

		// Get serializer
		YAML::Emitter& out = *serializeContext->m_Serializer;

		// Serialize component
		out << YAML::Key << "ParticleEmitter";
		out << YAML::BeginMap; // Component Map
		out << YAML::Key << "EmitterHandle" << YAML::Value << static_cast<uint64_t>(m_EmitterConfigRef.GetAssetHandle());
		out << YAML::EndMap; // Component Map
	}
	void ParticleEmitter::Deserialize(void* context)
	{
		// Get context
		ECSInternal::DeserializeComponentContext* deserializeContext =
			(ECSInternal::DeserializeComponentContext*)context;
		KG_ASSERT(deserializeContext, "Context cannot be null");
		KG_ASSERT(deserializeContext->m_Node, "Registry node cannot be null");
		// Get node
		YAML::Node& node = *deserializeContext->m_Node;

		Assets::AssetHandle emitterConfigHandle = node["EmitterHandle"].as<uint64_t>();
		m_EmitterConfigRef = Assets::AssetService::m_EmitterConfigManager.GetAssetByHandle(emitterConfigHandle);
	}
}