#include "kgpch.h"

#include "Modules/Physics2D/Components/RigidBody2D.h"

namespace Kargono::Physics2D
{
	void RigidBody2D::Serialize(void* context)
	{
		// Get context
		ECSInternal::SerializeComponentContext* serializeContext =
			(ECSInternal::SerializeComponentContext*)context;
		KG_ASSERT(serializeContext, "Context cannot be null");
		KG_ASSERT(serializeContext->m_Serializer, "Serializer cannot be null");
		// Get serializer
		YAML::Emitter& out = *serializeContext->m_Serializer;
		// Serialize component
		out << YAML::Key << "RigidBody2D";
		out << YAML::BeginMap; // Component Map
		out << YAML::Key << "BodyType" << YAML::Value << Utility::RigidBody2DBodyTypeToString(m_Type);
		out << YAML::Key << "FixedRotation" << YAML::Value << m_FixedRotation;
		out << YAML::Key << "OnCollisionStartHandle" << YAML::Value << static_cast<uint64_t>(m_OnCollisionStartScriptHandle);
		out << YAML::Key << "OnCollisionEndHandle" << YAML::Value << static_cast<uint64_t>(m_OnCollisionEndScriptHandle);
		out << YAML::EndMap; // Component Map
	}
	void RigidBody2D::Deserialize(void* context)
	{
		// Get context
		ECSInternal::DeserializeComponentContext* deserializeContext =
			(ECSInternal::DeserializeComponentContext*)context;
		KG_ASSERT(deserializeContext, "Context cannot be null");
		KG_ASSERT(deserializeContext->m_Node, "Registry node cannot be null");
		// Get node
		YAML::Node& node = *deserializeContext->m_Node;

		// Deserialize
		m_Type = Utility::StringToRigidBody2DBodyType(node["BodyType"].as<std::string>());
		m_FixedRotation = node["FixedRotation"].as<bool>();

		m_OnCollisionStartScriptHandle = node["OnCollisionStartHandle"].as<uint64_t>();
		m_OnCollisionStartScript = Assets::AssetService::m_ScriptManager.GetAssetByHandle(m_OnCollisionStartScriptHandle);
		m_OnCollisionEndScriptHandle = node["OnCollisionEndHandle"].as<uint64_t>();
		m_OnCollisionEndScript = Assets::AssetService::m_ScriptManager.GetAssetByHandle(m_OnCollisionEndScriptHandle);
	}
}