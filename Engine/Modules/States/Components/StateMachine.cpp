#include "kgpch.h"

#include "Modules/States/Components/StateMachine.h"
#include "Modules/Assets/Managers/StateManager.h"


namespace Kargono::States
{
	void StateMachine::Serialize(void* context)
	{
		// Get context
		ECSInternal::SerializeComponentContext* serializeContext =
			(ECSInternal::SerializeComponentContext*)context;
		KG_ASSERT(serializeContext, "Context cannot be null");
		KG_ASSERT(serializeContext->m_Serializer, "Serializer cannot be null");
		// Get serializer
		YAML::Emitter& out = *serializeContext->m_Serializer;
		// Serialize component
		out << YAML::Key << "StateMachine";
		out << YAML::BeginMap; // Component Map
		out << YAML::Key << "CurrentState" << YAML::Value << static_cast<uint64_t>(m_CurrentStateReference.GetAssetHandle());
		out << YAML::Key << "PreviousState" << YAML::Value << static_cast<uint64_t>(m_PreviousStateReference.GetAssetHandle());
		out << YAML::Key << "GlobalState" << YAML::Value << static_cast<uint64_t>(m_GlobalStateReference.GetAssetHandle());
		out << YAML::EndMap; // Component Map
	}
	void StateMachine::Deserialize(void* context)
	{
		// Get context
		ECSInternal::DeserializeComponentContext* deserializeContext =
			(ECSInternal::DeserializeComponentContext*)context;
		KG_ASSERT(deserializeContext, "Context cannot be null");
		KG_ASSERT(deserializeContext->m_Node, "Registry node cannot be null");
		// Get node
		YAML::Node& node = *deserializeContext->m_Node;

		// Deserialize current state
		Assets::AssetHandle currentStateHandle = node["CurrentState"].as<uint64_t>();
		m_CurrentStateReference = Assets::s_StateManager.GetAssetByHandle(currentStateHandle);
		if (currentStateHandle.IsValid() && !m_CurrentStateReference)
		{
			KG_WARN("Valid handle was found, however, could not retrieve a valid reference to current AI state");
		}

		// Deserialize previous state
		Assets::AssetHandle previousStateHandle = node["PreviousState"].as<uint64_t>();
		m_PreviousStateReference = Assets::s_StateManager.GetAssetByHandle(previousStateHandle);
		if (previousStateHandle.IsValid() && !m_PreviousStateReference)
		{
			KG_WARN("Valid handle was found, however, could not retrieve a valid reference for previous AI state");
		}

		// Deserialize global state
		Assets::AssetHandle globalStateHandle = node["GlobalState"].as<uint64_t>();
		m_GlobalStateReference = Assets::s_StateManager.GetAssetByHandle(globalStateHandle);
		if (globalStateHandle.IsValid() && !m_GlobalStateReference)
		{
			KG_WARN("Valid handle was found, however, could not retrieve a valid reference to global AI State");
		}
	}
}