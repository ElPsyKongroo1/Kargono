#include "kgpch.h"

#include "Modules/AI/Components/AIStateComponent.h"

namespace Kargono::AI
{
	void AIStateComponent::Serialize(void* context)
	{
		// Get context
		ECSInternal::SerializeComponentContext* serializeContext =
			(ECSInternal::SerializeComponentContext*)context;
		KG_ASSERT(serializeContext, "Context cannot be null");
		KG_ASSERT(serializeContext->m_Serializer, "Serializer cannot be null");
		// Get serializer
		YAML::Emitter& out = *serializeContext->m_Serializer;
		// Serialize component
		out << YAML::Key << "AIStateComponent";
		out << YAML::BeginMap; // Component Map
		out << YAML::Key << "CurrentState" << YAML::Value << static_cast<uint64_t>(m_CurrentStateHandle);
		out << YAML::Key << "PreviousState" << YAML::Value << static_cast<uint64_t>(m_PreviousStateHandle);
		out << YAML::Key << "GlobalState" << YAML::Value << static_cast<uint64_t>(m_GlobalStateHandle);
		out << YAML::EndMap; // Component Map
	}
	void AIStateComponent::Deserialize(void* context)
	{
		// Get context
		ECSInternal::DeserializeComponentContext* deserializeContext =
			(ECSInternal::DeserializeComponentContext*)context;
		KG_ASSERT(deserializeContext, "Context cannot be null");
		KG_ASSERT(deserializeContext->m_Node, "Registry node cannot be null");
		// Get node
		YAML::Node& node = *deserializeContext->m_Node;

		// Deserialize current state
		m_CurrentStateHandle = node["CurrentState"].as<uint64_t>();
		m_CurrentStateReference = Assets::AssetService::GetAIState(m_CurrentStateHandle);
		if (m_CurrentStateHandle != Assets::k_EmptyHandle && !m_CurrentStateReference)
		{
			KG_WARN("Valid handle was found, however, could not retrieve a valid reference to current AI state");
		}

		// Deserialize previous state
		m_PreviousStateHandle = node["PreviousState"].as<uint64_t>();
		m_PreviousStateReference = Assets::AssetService::GetAIState(m_PreviousStateHandle);
		if (m_PreviousStateHandle != Assets::k_EmptyHandle && !m_PreviousStateReference)
		{
			KG_WARN("Valid handle was found, however, could not retrieve a valid reference for previous AI state");
		}

		// Deserialize global state
		m_GlobalStateHandle = node["GlobalState"].as<uint64_t>();
		m_GlobalStateReference = Assets::AssetService::GetAIState(m_GlobalStateHandle);
		if (m_GlobalStateHandle != Assets::k_EmptyHandle && !m_GlobalStateReference)
		{
			KG_WARN("Valid handle was found, however, could not retrieve a valid reference to global AI State");
		}
	}
}