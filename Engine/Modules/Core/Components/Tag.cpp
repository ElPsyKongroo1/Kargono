#include "kgpch.h"

#include "Modules/Core/Components/Tag.h"

namespace Kargono
{
	void Tag::Serialize(void* context)
	{
		// Get context
		ECSInternal::SerializeComponentContext* serializeContext =
			(ECSInternal::SerializeComponentContext*)context;
		KG_ASSERT(serializeContext, "Context cannot be null");

		// Get serializer
		YAML::Emitter& out = *serializeContext->m_Serializer;

		// Serialize tag component
		out << YAML::Key << "Tag";

		out << YAML::BeginMap; // Component Map
		out << YAML::Key << "Tag" << YAML::Value << m_Tag;
		out << YAML::Key << "Group" << YAML::Value << m_Group;
		out << YAML::EndMap; // Component Map
	}
	void Tag::Deserialize(void* context)
	{
		// Get context
		ECSInternal::DeserializeComponentContext* deserializeContext =
			(ECSInternal::DeserializeComponentContext*)context;
		KG_ASSERT(deserializeContext, "Context cannot be null");
		KG_ASSERT(deserializeContext->m_Node, "Registry node cannot be null");
		// Get node
		YAML::Node& node = *deserializeContext->m_Node;

		// Deserialize
		m_Tag = node["Tag"].as<std::string>();
		m_Group = node["Group"].as<std::string>();
	}
}