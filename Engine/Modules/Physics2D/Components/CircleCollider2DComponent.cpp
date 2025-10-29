#include "kgpch.h"

#include "Modules/Physics2D/Components/CircleCollider2DComponent.h"

namespace Kargono::Physics2D
{
	void CircleCollider2DComponent::Serialize(void* context)
	{
		// Get context
		ECSInternal::SerializeComponentContext* serializeContext =
			(ECSInternal::SerializeComponentContext*)context;
		KG_ASSERT(serializeContext, "Context cannot be null");
		KG_ASSERT(serializeContext->m_Serializer, "Serializer cannot be null");
		// Get serializer
		YAML::Emitter& out = *serializeContext->m_Serializer;
		// Serialize component
		out << YAML::Key << "CircleCollider2DComponent";
		out << YAML::BeginMap; // Component Map
		out << YAML::Key << "Offset" << YAML::Value << m_Offset;
		out << YAML::Key << "Radius" << YAML::Value << m_Radius;
		out << YAML::Key << "Density" << YAML::Value << m_Density;
		out << YAML::Key << "Friction" << YAML::Value << m_Friction;
		out << YAML::Key << "Restitution" << YAML::Value << m_Restitution;
		out << YAML::Key << "RestitutionThreshold" << YAML::Value << m_RestitutionThreshold;
		out << YAML::Key << "IsSensor" << YAML::Value << m_IsSensor;
		out << YAML::EndMap; // Component Map
	}
	void CircleCollider2DComponent::Deserialize(void* context)
	{
		// Get context
		ECSInternal::DeserializeComponentContext* deserializeContext =
			(ECSInternal::DeserializeComponentContext*)context;
		KG_ASSERT(deserializeContext, "Context cannot be null");
		KG_ASSERT(deserializeContext->m_Node, "Registry node cannot be null");
		// Get node
		YAML::Node& node = *deserializeContext->m_Node;

		// Deserialize
		m_Offset = node["Offset"].as<Math::vec2>();
		m_Radius = node["Radius"].as<float>();
		m_Density = node["Density"].as<float>();
		m_Friction = node["Friction"].as<float>();
		m_Restitution = node["Restitution"].as<float>();
		m_RestitutionThreshold = node["RestitutionThreshold"].as<float>();
		m_IsSensor = node["IsSensor"].as<bool>();
	}
}