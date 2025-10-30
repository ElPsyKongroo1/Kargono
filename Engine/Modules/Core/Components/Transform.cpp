#include "kgpch.h"

#include "Modules/Core/Components/Transform.h"

namespace Kargono
{
	void Transform::Serialize(void* context)
	{
		// Get context
		ECSInternal::SerializeComponentContext* serializeContext =
			(ECSInternal::SerializeComponentContext*)context;
		KG_ASSERT(serializeContext, "Context cannot be null");
		KG_ASSERT(serializeContext->m_Serializer, "Serializer cannot be null");
		// Get serializer
		YAML::Emitter& out = *serializeContext->m_Serializer;
		// Serialize component
		out << YAML::BeginMap; // Component Map
		out << YAML::Key << "Translation" << YAML::Value << m_Translation;
		out << YAML::Key << "Rotation" << YAML::Value << m_Rotation;
		out << YAML::Key << "Scale" << YAML::Value << m_Scale;
		out << YAML::EndMap; // Component Map
	}
	void Transform::Deserialize(void* context)
	{
		// Get context
		ECSInternal::DeserializeComponentContext* deserializeContext =
			(ECSInternal::DeserializeComponentContext*)context;
		KG_ASSERT(deserializeContext, "Context cannot be null");
		KG_ASSERT(deserializeContext->m_Node, "Registry node cannot be null");
		// Get node
		YAML::Node& node = *deserializeContext->m_Node;

		// Deserialize component
		m_Translation = node["Translation"].as<Math::vec3>();
		m_Rotation = node["Rotation"].as<Math::vec3>();
		m_Scale = node["Scale"].as<Math::vec3>();
	}

	Math::mat4 Transform::GetTransform() const
	{
		const Math::mat4 rotation{ glm::toMat4(Math::quat(m_Rotation)) };

		return glm::translate(Math::mat4(1.0f), m_Translation)
			* rotation
			* glm::scale(Math::mat4(1.0f), m_Scale);
	}

	Math::mat4 Transform::GetTranslation() const
	{
		return glm::translate(Math::mat4(1.0f), m_Translation);
	}

	Math::mat4 Transform::GetRotation() const
	{
		return glm::toMat4(Math::quat(m_Rotation));
	}
}