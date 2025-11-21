#include "kgpch.h"

#include "Modules/Scripting/Components/OnUpdate.h"

namespace Kargono::Scripting
{
	void OnUpdate::Serialize(void* context)
	{
		// Get context
		ECSInternal::SerializeComponentContext* serializeContext =
			(ECSInternal::SerializeComponentContext*)context;
		KG_ASSERT(serializeContext, "Context cannot be null");
		// Get serializer
		YAML::Emitter& out = *serializeContext->m_Serializer;

		// Serialize component
		out << YAML::Key << "OnUpdate";
		out << YAML::BeginMap; // Component Map
		out << YAML::Key << "OnUpdateHandle" << YAML::Value << static_cast<uint64_t>(m_OnUpdateScriptHandle);
		out << YAML::EndMap; // Component Map
	}
	void OnUpdate::Deserialize(void* context)
	{
		// Get context
		ECSInternal::DeserializeComponentContext* deserializeContext =
			(ECSInternal::DeserializeComponentContext*)context;
		KG_ASSERT(deserializeContext, "Context cannot be null");
		KG_ASSERT(deserializeContext->m_Node, "Registry node cannot be null");
		// Get node
		YAML::Node& node = *deserializeContext->m_Node;

		// Deserialize component
		m_OnUpdateScriptHandle = node["OnUpdateHandle"].as<uint64_t>();
		m_OnUpdateScript = Assets::s_ScriptManager.GetAssetByHandle(m_OnUpdateScriptHandle);
	} 
}