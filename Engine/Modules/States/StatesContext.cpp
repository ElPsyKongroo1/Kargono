#include "kgpch.h"

#include "Modules/States/StatesContext.h"
#include "Modules/States/Assets/State.h"
#include "Modules/Scenes/Assets/Scene.h"
#include "Modules/Scenes/SceneContext.h"
#include "Modules/ECS/Entity.h"
#include "Kargono/Projects/Project.h"
#include "Modules/FileSystem/FileSystem.h"
#include "Modules/Scripting/Assets/Script.h"
#include "Modules/Assets/Managers/StateManager.h"
#include "Kargono/Utility/Time.h"
#include "Modules/States/Components/StateMachine.h"


namespace Kargono::States
{
	bool StatesContext::Init()
	{
		return true;
	}
	bool StatesContext::Terminate()
	{
		return true;
	}
	void StatesContext::OnUpdate(Timestep timeStep)
	{
		// Ensure a valid scene is active
		Assets::AssetRef<Scenes::Scene>activeScene = Scenes::SceneService::GetActiveContext().GetActiveScene();
		KG_ASSERT(activeScene, "Invalid scene reference when calling AIService's OnUpdate()");

		// Run on update for all active StateMachines with AIComponents including the global, then the current state
		for (ECSInternal::EntityID id : activeScene->m_EntityRegistry.GetView<States::StateMachine>())
		{
			ECS::Entity entity = activeScene->m_EntityRegistry.GetEntityByECSID(id);
			KG_ASSERT(entity, "Invalid entity obtained. Could not run OnUpdate on provided entity.");
			States::StateMachine& aiComponent = entity.GetComponent<States::StateMachine>();

			// Call Global State OnUpdate
			if (aiComponent.m_GlobalStateReference && aiComponent.m_GlobalStateReference->m_OnUpdate)
			{
				Ref<WrappedFunction> onUpdateFunc = aiComponent.m_GlobalStateReference->m_OnUpdate->m_Function;
				Utility::CallWrapped<WrappedVoidEntityFloat>(onUpdateFunc, entity.GetUUID(), timeStep);
			}

			// Call Current State OnUpdate
			if (aiComponent.m_CurrentStateReference && aiComponent.m_CurrentStateReference->m_OnUpdate)
			{

				Utility::CallWrapped<WrappedVoidEntityFloat>(aiComponent.m_CurrentStateReference->m_OnUpdate->m_Function, entity.GetUUID(), timeStep);
			}
		}

		// Check/Handle StatesContext's delayed messages queue
		HandleDelayedMessages();
	}
	bool StatesContext::IsGlobalState(UUID entityID, Assets::AssetHandle queryStateHandle)
	{
		// Ensure a valid scene is active and a valid entity is provided
		Assets::AssetRef<Scenes::Scene>activeScene = Scenes::SceneService::GetActiveContext().GetActiveScene();
		KG_ASSERT(activeScene, "Invalid scene reference inside AIService");
		ECS::Entity entity = activeScene->m_EntityRegistry.GetEntityByUUID(entityID);
		KG_ASSERT(entity, "Invalid entity obtained inside AIService");

		// Get ai component to be queried
		States::StateMachine& aiComponent = entity.GetComponent<States::StateMachine>();

		// Return whether the component aiState is the same as query aiState
		return aiComponent.m_GlobalStateReference.GetAssetHandle() == queryStateHandle;
	}
	bool StatesContext::IsCurrentState(UUID entityID, Assets::AssetHandle queryStateHandle)
	{
		// Ensure a valid scene is active and a valid entity is provided
		Assets::AssetRef<Scenes::Scene>activeScene = Scenes::SceneService::GetActiveContext().GetActiveScene();
		KG_ASSERT(activeScene, "Invalid scene reference inside AIService");
		ECS::Entity entity = activeScene->m_EntityRegistry.GetEntityByUUID(entityID);
		KG_ASSERT(entity, "Invalid entity obtained inside AIService");

		// Get ai component to be queried
		States::StateMachine& aiComponent = entity.GetComponent<States::StateMachine>();

		// Return whether the component aiState is the same as query aiState
		return aiComponent.m_CurrentStateReference.GetAssetHandle() == queryStateHandle;
	}
	bool StatesContext::IsPreviousState(UUID entityID, Assets::AssetHandle queryStateHandle)
	{
		// Ensure a valid scene is active and a valid entity is provided
		Assets::AssetRef<Scenes::Scene>activeScene = Scenes::SceneService::GetActiveContext().GetActiveScene();
		KG_ASSERT(activeScene, "Invalid scene reference inside AIService");
		ECS::Entity entity = activeScene->m_EntityRegistry.GetEntityByUUID(entityID);
		KG_ASSERT(entity, "Invalid entity obtained inside AIService");

		// Get ai component to be queried
		States::StateMachine& aiComponent = entity.GetComponent<States::StateMachine>();

		// Return whether the component aiState is the same as query aiState
		return aiComponent.m_PreviousStateReference.GetAssetHandle() == queryStateHandle;
	}
	void StatesContext::ChangeGlobalState(UUID entityID, Assets::AssetHandle newStateHandle)
	{
		// Ensure a valid scene is active and a valid entity is provided
		Assets::AssetRef<Scenes::Scene>activeScene = Scenes::SceneService::GetActiveContext().GetActiveScene();
		KG_ASSERT(activeScene, "Invalid scene reference inside AIService");
		ECS::Entity entity = activeScene->m_EntityRegistry.GetEntityByUUID(entityID);
		KG_ASSERT(entity, "Invalid entity obtained inside AIService");

		// Ensure new State is valid
		Assets::AssetRef<State> newStateRef = Assets::s_StateManager.GetAssetByHandle(newStateHandle);
		KG_ASSERT(newStateRef, "Invalid new AI state provided inside AIService");

		// Get ai component to be modified
		States::StateMachine& aiComponent = entity.GetComponent<States::StateMachine>();

		// Call OnExitState() for active global State
		if (aiComponent.m_GlobalStateReference && aiComponent.m_GlobalStateReference.GetAsset().m_OnExitState)
		{
			Utility::CallWrapped<WrappedVoidEntity>(aiComponent.m_GlobalStateReference.GetAsset().m_OnExitState->m_Function, entityID);
		}

		// Switch to new global State
		aiComponent.m_GlobalStateReference = newStateRef;

		// Call OnEnter for new global state
		if (aiComponent.m_GlobalStateReference && aiComponent.m_GlobalStateReference->m_OnEnterState)
		{
			Utility::CallWrapped<WrappedVoidEntity>(aiComponent.m_GlobalStateReference->m_OnEnterState->m_Function, entityID);
		}

	}
	void StatesContext::ChangeCurrentState(UUID entityID, Assets::AssetHandle newStateHandle)
	{
		// Ensure a valid scene is active and a valid entity is provided
		Assets::AssetRef<Scenes::Scene>activeScene = Scenes::SceneService::GetActiveContext().GetActiveScene();
		KG_ASSERT(activeScene, "Invalid scene reference inside AIService");
		ECS::Entity entity = activeScene->m_EntityRegistry.GetEntityByUUID(entityID);
		KG_ASSERT(entity, "Invalid entity obtained inside AIService");

		// Ensure new State is valid
		Assets::AssetReference<State> newStateRef = Assets::s_StateManager.GetAssetByHandle(newStateHandle);
		KG_ASSERT(newStateRef);

		// Get ai component to be modified
		States::StateMachine& aiComponent = entity.GetComponent<States::StateMachine>();

		// Store current state as the new previous state
		aiComponent.m_PreviousStateReference = aiComponent.m_CurrentStateReference;

		// Call OnExitState() for current State
		if (aiComponent.m_CurrentStateReference && aiComponent.m_CurrentStateReference.GetAsset().m_OnExitState)
		{
			Utility::CallWrapped<WrappedVoidEntity>(aiComponent.m_CurrentStateReference.GetAsset().m_OnExitState->m_Function, entityID);
		}

		// Switch to new State
		aiComponent.m_CurrentStateReference = newStateRef;

		// Call OnEnter for new state
		if (aiComponent.m_CurrentStateReference && aiComponent.m_CurrentStateReference->m_OnEnterState)
		{
			Utility::CallWrapped<WrappedVoidEntity>(aiComponent.m_CurrentStateReference->m_OnEnterState->m_Function, entityID);
		}
	}
	void StatesContext::RevertPreviousState(UUID entityID)
	{
		// Ensure a valid scene is active and a valid entity is provided
		Assets::AssetRef<Scenes::Scene>activeScene = Scenes::SceneService::GetActiveContext().GetActiveScene();
		KG_ASSERT(activeScene, "Invalid scene reference inside AIService");
		ECS::Entity entity = activeScene->m_EntityRegistry.GetEntityByUUID(entityID);
		KG_ASSERT(entity, "Invalid entity obtained inside AIService");


		// Get ai component to be modified
		States::StateMachine& aiComponent = entity.GetComponent<States::StateMachine>();
		
		// Ensure previous state is valid
		if (aiComponent.m_PreviousStateReference.GetAssetHandle() == Assets::k_EmptyHandle)
		{
			KG_WARN("Could not revert to previous state. None currently exists.");
			return;
		}

		// Call OnExitState() for current State
		if (aiComponent.m_CurrentStateReference && aiComponent.m_CurrentStateReference->m_OnExitState)
		{
			Utility::CallWrapped<WrappedVoidEntity>(aiComponent.m_CurrentStateReference->m_OnExitState->m_Function, entityID);
		}

		// Call ChangeState() into entityID's previous state if it exists
		aiComponent.m_CurrentStateReference = aiComponent.m_PreviousStateReference;

		// Call OnEnter for new current state
		if (aiComponent.m_CurrentStateReference && aiComponent.m_CurrentStateReference->m_OnEnterState)
		{
			Utility::CallWrapped<WrappedVoidEntity>(aiComponent.m_CurrentStateReference->m_OnEnterState->m_Function, entityID);
		}

		// Clear previous state
		aiComponent.m_PreviousStateReference.Reset();
	}

	void StatesContext::ClearGlobalState(UUID entityID)
	{
		// Ensure a valid scene is active and a valid entity is provided
		Assets::AssetRef<Scenes::Scene>activeScene = Scenes::SceneService::GetActiveContext().GetActiveScene();
		KG_ASSERT(activeScene, "Invalid scene reference inside AIService");
		ECS::Entity entity = activeScene->m_EntityRegistry.GetEntityByUUID(entityID);
		KG_ASSERT(entity, "Invalid entity obtained inside AIService");

		// Get ai component to be modified
		States::StateMachine& aiComponent = entity.GetComponent<States::StateMachine>();

		// Clear global state
		aiComponent.m_GlobalStateReference.Reset();
	}
	void StatesContext::ClearCurrentState(UUID entityID)
	{
		// Ensure a valid scene is active and a valid entity is provided
		Assets::AssetRef<Scenes::Scene>activeScene = Scenes::SceneService::GetActiveContext().GetActiveScene();
		KG_ASSERT(activeScene, "Invalid scene reference inside AIService");
		ECS::Entity entity = activeScene->m_EntityRegistry.GetEntityByUUID(entityID);
		KG_ASSERT(entity, "Invalid entity obtained inside AIService");

		// Get ai component to be modified
		States::StateMachine& aiComponent = entity.GetComponent<States::StateMachine>();

		// Clear current state
		aiComponent.m_CurrentStateReference = {};
	}
	void StatesContext::ClearPreviousState(UUID entityID)
	{
		// Ensure a valid scene is active and a valid entity is provided
		Assets::AssetRef<Scenes::Scene>activeScene = Scenes::SceneService::GetActiveContext().GetActiveScene();
		KG_ASSERT(activeScene, "Invalid scene reference inside AIService");
		ECS::Entity entity = activeScene->m_EntityRegistry.GetEntityByUUID(entityID);
		KG_ASSERT(entity, "Invalid entity obtained inside AIService");

		// Get ai component to be modified
		States::StateMachine& aiComponent = entity.GetComponent<States::StateMachine>();

		// Clear previous state
		aiComponent.m_PreviousStateReference = {};
	}
	void StatesContext::ClearAllStates(UUID entityID)
	{
		// Ensure a valid scene is active and a valid entity is provided
		Assets::AssetRef<Scenes::Scene>activeScene = Scenes::SceneService::GetActiveContext().GetActiveScene();
		KG_ASSERT(activeScene, "Invalid scene reference inside AIService");
		ECS::Entity entity = activeScene->m_EntityRegistry.GetEntityByUUID(entityID);
		KG_ASSERT(entity, "Invalid entity obtained inside AIService");

		// Get ai component to be modified
		States::StateMachine& aiComponent = entity.GetComponent<States::StateMachine>();

		// Clear global state
		aiComponent.m_GlobalStateReference.Reset();

		// Clear current state
		aiComponent.m_CurrentStateReference.Reset();

		// Clear previous state
		aiComponent.m_PreviousStateReference.Reset();
	}


	void StatesContext::SendAIMessage(uint32_t messageType, UUID senderEntity, UUID receiverEntity, float delayTime)
	{
		AIMessage newMessage{ messageType, senderEntity, receiverEntity, Utility::Time::GetTime() + delayTime };

		// Check if message should be handled immediately or placed into delay queue
		if (delayTime > 0.001f)
		{
			// Insert message into delay queue. If unsuccessful, increment delay time and retry until successful
			m_MessageQueue.push({std::move(newMessage)});
		}
		else
		{
			// Handle message
			HandleAIMessage(std::move(newMessage));
		}
	}
	void StatesContext::HandleAIMessage(const AIMessage& messageToHandle)
	{
		// Ensure a valid scene is active and a valid entity is provided
		Assets::AssetRef<Scenes::Scene>activeScene = Scenes::SceneService::GetActiveContext().GetActiveScene();
		KG_ASSERT(activeScene, "Invalid scene reference inside AIService");
		ECS::Entity receiverEntity = activeScene->m_EntityRegistry.GetEntityByUUID(messageToHandle.m_ReceiverEntity);
		KG_ASSERT(receiverEntity, "Invalid entity obtained inside AIService");


		// Get ai component whose OnMessage() function needs to be called
		States::StateMachine& receiverAIComponent = receiverEntity.GetComponent<States::StateMachine>();

		// Call OnMessage for recipient's global state
		if (receiverAIComponent.m_GlobalStateReference && receiverAIComponent.m_GlobalStateReference->m_OnMessage)
		{
			Utility::CallWrapped<WrappedVoidUInt32EntityEntityFloat>(receiverAIComponent.m_GlobalStateReference->m_OnMessage->m_Function, messageToHandle.m_MessageType, messageToHandle.m_SenderEntity, messageToHandle.m_ReceiverEntity, messageToHandle.m_DispatchTime);
		}

		// Call OnMessage for recipient's current state
		if (receiverAIComponent.m_CurrentStateReference && receiverAIComponent.m_CurrentStateReference->m_OnMessage)
		{
			Utility::CallWrapped<WrappedVoidUInt32EntityEntityFloat>(receiverAIComponent.m_CurrentStateReference->m_OnMessage->m_Function, messageToHandle.m_MessageType, messageToHandle.m_SenderEntity, messageToHandle.m_ReceiverEntity, messageToHandle.m_DispatchTime);
		}
	}
	void StatesContext::HandleDelayedMessages()
	{
		float currentTime = Utility::Time::GetTime();
		auto& messageQueue = m_MessageQueue;

		// Loop through messageQueue and process messages if they are due for dispatch
		while (!messageQueue.empty())
		{
			// Get the current top message
			const AIMessage& currentMessage = messageQueue.top();

			// Check if current message is due for dispatch
			if (currentMessage.m_DispatchTime <= currentTime)
			{
				// Remove message from queue
				messageQueue.pop();

				// Handle message
				HandleAIMessage(std::move(currentMessage));

			}

			// Exit if current message (and remainder of queue) is still waiting...
			else
			{
				break;
			}
		}
	}
}

