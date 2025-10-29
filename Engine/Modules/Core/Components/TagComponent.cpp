#include "kgpch.h"

#include "Modules/Core/Components/TagComponent.h"

namespace Kargono
{
	void TagComponent::Serialize(void* context)
	{
		// Get context
		ECSInternal::SerializeComponentContext* serializeContext =
			(ECSInternal::SerializeComponentContext*)context;
		KG_ASSERT(serializeContext, "Context cannot be null");

		// Get serializer
		YAML::Emitter& out = *serializeContext->m_Serializer;

		// Serialize tag component
		out << YAML::Key << "TagComponent";

		out << YAML::BeginMap; // Component Map
		out << YAML::Key << "Tag" << YAML::Value << m_Tag;
		out << YAML::Key << "Group" << YAML::Value << m_Group;
		out << YAML::EndMap; // Component Map
	}
	void TagComponent::Deserialize(void* context)
	{

		tagComp.m_Tag = tagNode["Tag"].as<std::string>();
		tagComp.m_Group = tagNode["Group"].as<std::string>();
	}
}