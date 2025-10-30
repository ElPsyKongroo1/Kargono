#include "kgpch.h"

#include "Modules/Scripting/Components/OnCreate.h"

namespace Kargono::Scripting
{
	void OnCreate::Serialize(void* context)
	{
		// Get context
		ECSInternal::SerializeComponentContext* serializeContext =
			(ECSInternal::SerializeComponentContext*)context;
		KG_ASSERT(serializeContext, "Context cannot be null");
		KG_ASSERT(serializeContext->m_Serializer, "Serializer cannot be null");
		// Get serializer
		YAML::Emitter& out = *serializeContext->m_Serializer;
		// Serialize component
		out << YAML::Key << "OnCreate";
		out << YAML::BeginMap; // Component Map
		out << YAML::Key << "OnCreateHandle" << YAML::Value << static_cast<uint64_t>(m_OnCreateScriptHandle);
		out << YAML::EndMap; // Component Map
	}
	void OnCreate::Deserialize(void* context)
	{
		// Get context
		ECSInternal::DeserializeComponentContext* deserializeContext =
			(ECSInternal::DeserializeComponentContext*)context;
		KG_ASSERT(deserializeContext, "Context cannot be null");
		KG_ASSERT(deserializeContext->m_Node, "Registry node cannot be null");
		// Get node
		YAML::Node& node = *deserializeContext->m_Node;

		m_OnCreateScriptHandle = node["OnCreateHandle"].as<uint64_t>();
		m_OnCreateScript = Assets::AssetService::GetScript(m_OnCreateScriptHandle);
	} 
}