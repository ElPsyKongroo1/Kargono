#include "kgpch.h"

#include "Modules/AI/AIService.h"
#include "Kargono/Scenes/Scene.h"
#include "Modules/ECS/Entity.h"
#include "Kargono/Projects/Project.h"
#include "Modules/FileSystem/FileSystem.h"
#include "Modules/Assets/AssetService.h"
#include "Kargono/Utility/Time.h"


namespace Kargono::AI
{
	bool AIContext::Init()
	{
		return true;
	}
	bool AIContext::Terminate()
	{
		return true;
	}
	void AIContext::OnUpdate(Timestep timeStep)
	{
		// Ensure a valid scene is active
		Ref<Scenes::Scene> activeScene = Scenes::SceneService::GetActiveScene();
		KG_ASSERT(activeScene, "Invalid scene reference when calling AIService's OnUpdate()");

		// Run on update for all active AI with AIComponents including the global, then the current state
		for (entt::entity enttID : Scenes::SceneService::GetActiveScene()->GetAllEntitiesWith<ECS::AIStateComponent>())
		{
			ECS::Entity entity = activeScene->GetEntityByEnttID(enttID);
			KG_ASSERT(entity, "Invalid entity obtained. Could not run OnUpdate on provided entity.");
			ECS::AIStateComponent& aiComponent = entity.GetComponent<ECS::AIStateComponent>();

			// Call Global State OnUpdate
			if (aiComponent.GlobalStateReference && aiComponent.GlobalStateReference->OnUpdate)
			{
				Utility::CallWrapped<WrappedVoidEntityFloat>(aiComponent.GlobalStateReference->OnUpdate->m_Function, entity.GetUUID(), timeStep);
			}

			// Call Current State OnUpdate
			if (aiComponent.CurrentStateReference && aiComponent.CurrentStateReference->OnUpdate)
			{
				Utility::CallWrapped<WrappedVoidEntityFloat>(aiComponent.CurrentStateReference->OnUpdate->m_Function, entity.GetUUID(), timeStep);
			}
		}

		// Check/Handle AIContext's delayed messages queue
		HandleDelayedMessages();
	}
	bool AIContext::IsGlobalState(UUID entityID, Assets::AssetHandle queryAIStateHandle)
	{
		// Ensure a valid scene is active and a valid entity is provided
		Ref<Scenes::Scene> activeScene = Scenes::SceneService::GetActiveScene();
		KG_ASSERT(activeScene, "Invalid scene reference inside AIService");
		ECS::Entity entity = activeScene->GetEntityByUUID(entityID);
		KG_ASSERT(entity, "Invalid entity obtained inside AIService");

		// Get ai component to be queried
		ECS::AIStateComponent& aiComponent = entity.GetComponent<ECS::AIStateComponent>();

		// Return whether the component aiState is the same as query aiState
		return aiComponent.GlobalStateHandle == queryAIStateHandle;
	}
	bool AIContext::IsCurrentState(UUID entityID, Assets::AssetHandle queryAIStateHandle)
	{
		// Ensure a valid scene is active and a valid entity is provided
		Ref<Scenes::Scene> activeScene = Scenes::SceneService::GetActiveScene();
		KG_ASSERT(activeScene, "Invalid scene reference inside AIService");
		ECS::Entity entity = activeScene->GetEntityByUUID(entityID);
		KG_ASSERT(entity, "Invalid entity obtained inside AIService");

		// Get ai component to be queried
		ECS::AIStateComponent& aiComponent = entity.GetComponent<ECS::AIStateComponent>();

		// Return whether the component aiState is the same as query aiState
		return aiComponent.CurrentStateHandle == queryAIStateHandle;
	}
	bool AIContext::IsPreviousState(UUID entityID, Assets::AssetHandle queryAIStateHandle)
	{
		// Ensure a valid scene is active and a valid entity is provided
		Ref<Scenes::Scene> activeScene = Scenes::SceneService::GetActiveScene();
		KG_ASSERT(activeScene, "Invalid scene reference inside AIService");
		ECS::Entity entity = activeScene->GetEntityByUUID(entityID);
		KG_ASSERT(entity, "Invalid entity obtained inside AIService");

		// Get ai component to be queried
		ECS::AIStateComponent& aiComponent = entity.GetComponent<ECS::AIStateComponent>();

		// Return whether the component aiState is the same as query aiState
		return aiComponent.PreviousStateHandle == queryAIStateHandle;
	}
	void AIContext::ChangeGlobalState(UUID entityID, Assets::AssetHandle newAIStateHandle)
	{
		// Ensure a valid scene is active and a valid entity is provided
		Ref<Scenes::Scene> activeScene = Scenes::SceneService::GetActiveScene();
		KG_ASSERT(activeScene, "Invalid scene reference inside AIService");
		ECS::Entity entity = activeScene->GetEntityByUUID(entityID);
		KG_ASSERT(entity, "Invalid entity obtained inside AIService");

		// Ensure new AIState is valid
		Ref<AIState> newAIStateRef = Assets::AssetService::GetAIState(newAIStateHandle);
		KG_ASSERT(newAIStateRef, "Invalid new AI state provided inside AIService");

		// Get ai component to be modified
		ECS::AIStateComponent& aiComponent = entity.GetComponent<ECS::AIStateComponent>();

		// Call OnExitState() for active global AIState
		if (aiComponent.GlobalStateReference && aiComponent.GlobalStateReference->OnExitState)
		{
			Utility::CallWrapped<WrappedVoidEntity>(aiComponent.GlobalStateReference->OnExitState->m_Function, entityID);
		}

		// Switch to new global AIState
		aiComponent.GlobalStateHandle = newAIStateHandle;
		aiComponent.GlobalStateReference = newAIStateRef;

		// Call OnEnter for new global state
		if (aiComponent.GlobalStateReference && aiComponent.GlobalStateReference->OnEnterState)
		{
			Utility::CallWrapped<WrappedVoidEntity>(aiComponent.GlobalStateReference->OnEnterState->m_Function, entityID);
		}

	}
	void AIContext::ChangeCurrentState(UUID entityID, Assets::AssetHandle newAIStateHandle)
	{
		// Ensure a valid scene is active and a valid entity is provided
		Ref<Scenes::Scene> activeScene = Scenes::SceneService::GetActiveScene();
		KG_ASSERT(activeScene, "Invalid scene reference inside AIService");
		ECS::Entity entity = activeScene->GetEntityByUUID(entityID);
		KG_ASSERT(entity, "Invalid entity obtained inside AIService");

		// Ensure new AIState is valid
		Ref<AIState> newAIStateRef = Assets::AssetService::GetAIState(newAIStateHandle);
		KG_ASSERT(newAIStateRef, "Invalid new AI state provided inside AIService");

		// Get ai component to be modified
		ECS::AIStateComponent& aiComponent = entity.GetComponent<ECS::AIStateComponent>();

		// Store current state as the new previous state
		aiComponent.PreviousStateHandle = aiComponent.CurrentStateHandle;
		aiComponent.PreviousStateReference = aiComponent.CurrentStateReference;

		// Call OnExitState() for current AIState
		if (aiComponent.CurrentStateReference && aiComponent.CurrentStateReference->OnExitState)
		{
			Utility::CallWrapped<WrappedVoidEntity>(aiComponent.CurrentStateReference->OnExitState->m_Function, entityID);
		}

		// Switch to new AIState
		aiComponent.CurrentStateHandle = newAIStateHandle;
		aiComponent.CurrentStateReference = newAIStateRef;

		// Call OnEnter for new state
		if (aiComponent.CurrentStateReference && aiComponent.CurrentStateReference->OnEnterState)
		{
			Utility::CallWrapped<WrappedVoidEntity>(aiComponent.CurrentStateReference->OnEnterState->m_Function, entityID);
		}
	}
	void AIContext::RevertPreviousState(UUID entityID)
	{
		// Ensure a valid scene is active and a valid entity is provided
		Ref<Scenes::Scene> activeScene = Scenes::SceneService::GetActiveScene();
		KG_ASSERT(activeScene, "Invalid scene reference inside AIService");
		ECS::Entity entity = activeScene->GetEntityByUUID(entityID);
		KG_ASSERT(entity, "Invalid entity obtained inside AIService");


		// Get ai component to be modified
		ECS::AIStateComponent& aiComponent = entity.GetComponent<ECS::AIStateComponent>();
		
		// Ensure previous state is valid
		if (aiComponent.PreviousStateHandle == Assets::EmptyHandle)
		{
			KG_WARN("Could not revert to previous state. None currently exists.");
			return;
		}

		// Call OnExitState() for current AIState
		if (aiComponent.CurrentStateReference && aiComponent.CurrentStateReference->OnExitState)
		{
			Utility::CallWrapped<WrappedVoidEntity>(aiComponent.CurrentStateReference->OnExitState->m_Function, entityID);
		}

		// Call ChangeState() into entityID's previous state if it exists
		aiComponent.CurrentStateHandle = aiComponent.PreviousStateHandle;
		aiComponent.CurrentStateReference = aiComponent.PreviousStateReference;

		// Call OnEnter for new current state
		if (aiComponent.CurrentStateReference && aiComponent.CurrentStateReference->OnEnterState)
		{
			Utility::CallWrapped<WrappedVoidEntity>(aiComponent.CurrentStateReference->OnEnterState->m_Function, entityID);
		}

		// Clear previous state
		aiComponent.PreviousStateHandle = Assets::EmptyHandle;
		aiComponent.PreviousStateReference = nullptr;
	}

	void AIContext::ClearGlobalState(UUID entityID)
	{
		// Ensure a valid scene is active and a valid entity is provided
		Ref<Scenes::Scene> activeScene = Scenes::SceneService::GetActiveScene();
		KG_ASSERT(activeScene, "Invalid scene reference inside AIService");
		ECS::Entity entity = activeScene->GetEntityByUUID(entityID);
		KG_ASSERT(entity, "Invalid entity obtained inside AIService");

		// Get ai component to be modified
		ECS::AIStateComponent& aiComponent = entity.GetComponent<ECS::AIStateComponent>();

		// Clear global state
		aiComponent.GlobalStateHandle = Assets::EmptyHandle;
		aiComponent.GlobalStateReference = nullptr;
	}
	void AIContext::ClearCurrentState(UUID entityID)
	{
		// Ensure a valid scene is active and a valid entity is provided
		Ref<Scenes::Scene> activeScene = Scenes::SceneService::GetActiveScene();
		KG_ASSERT(activeScene, "Invalid scene reference inside AIService");
		ECS::Entity entity = activeScene->GetEntityByUUID(entityID);
		KG_ASSERT(entity, "Invalid entity obtained inside AIService");

		// Get ai component to be modified
		ECS::AIStateComponent& aiComponent = entity.GetComponent<ECS::AIStateComponent>();

		// Clear current state
		aiComponent.CurrentStateHandle = Assets::EmptyHandle;
		aiComponent.CurrentStateReference = nullptr;
	}
	void AIContext::ClearPreviousState(UUID entityID)
	{
		// Ensure a valid scene is active and a valid entity is provided
		Ref<Scenes::Scene> activeScene = Scenes::SceneService::GetActiveScene();
		KG_ASSERT(activeScene, "Invalid scene reference inside AIService");
		ECS::Entity entity = activeScene->GetEntityByUUID(entityID);
		KG_ASSERT(entity, "Invalid entity obtained inside AIService");

		// Get ai component to be modified
		ECS::AIStateComponent& aiComponent = entity.GetComponent<ECS::AIStateComponent>();

		// Clear previous state
		aiComponent.PreviousStateHandle = Assets::EmptyHandle;
		aiComponent.PreviousStateReference = nullptr;
	}
	void AIContext::ClearAllStates(UUID entityID)
	{
		// Ensure a valid scene is active and a valid entity is provided
		Ref<Scenes::Scene> activeScene = Scenes::SceneService::GetActiveScene();
		KG_ASSERT(activeScene, "Invalid scene reference inside AIService");
		ECS::Entity entity = activeScene->GetEntityByUUID(entityID);
		KG_ASSERT(entity, "Invalid entity obtained inside AIService");

		// Get ai component to be modified
		ECS::AIStateComponent& aiComponent = entity.GetComponent<ECS::AIStateComponent>();

		// Clear global state
		aiComponent.GlobalStateHandle = Assets::EmptyHandle;
		aiComponent.GlobalStateReference = nullptr;

		// Clear current state
		aiComponent.CurrentStateHandle = Assets::EmptyHandle;
		aiComponent.CurrentStateReference = nullptr;

		// Clear previous state
		aiComponent.PreviousStateHandle = Assets::EmptyHandle;
		aiComponent.PreviousStateReference = nullptr;
	}


	void AIContext::SendAIMessage(uint32_t messageType, UUID senderEntity, UUID receiverEntity, float delayTime)
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
	void AIContext::HandleAIMessage(const AIMessage& messageToHandle)
	{
		// Ensure a valid scene is active and a valid entity is provided
		Ref<Scenes::Scene> activeScene = Scenes::SceneService::GetActiveScene();
		KG_ASSERT(activeScene, "Invalid scene reference inside AIService");
		ECS::Entity receiverEntity = activeScene->GetEntityByUUID(messageToHandle.ReceiverEntity);
		KG_ASSERT(receiverEntity, "Invalid entity obtained inside AIService");


		// Get ai component whose OnMessage() function needs to be called
		ECS::AIStateComponent& receiverAIComponent = receiverEntity.GetComponent<ECS::AIStateComponent>();

		// Call OnMessage for recipient's global state
		if (receiverAIComponent.GlobalStateReference && receiverAIComponent.GlobalStateReference->OnMessage)
		{
			Utility::CallWrapped<WrappedVoidUInt32EntityEntityFloat>(receiverAIComponent.GlobalStateReference->OnMessage->m_Function, messageToHandle.MessageType, messageToHandle.SenderEntity, messageToHandle.ReceiverEntity, messageToHandle.DispatchTime);
		}

		// Call OnMessage for recipient's current state
		if (receiverAIComponent.CurrentStateReference && receiverAIComponent.CurrentStateReference->OnMessage)
		{
			Utility::CallWrapped<WrappedVoidUInt32EntityEntityFloat>(receiverAIComponent.CurrentStateReference->OnMessage->m_Function, messageToHandle.MessageType, messageToHandle.SenderEntity, messageToHandle.ReceiverEntity, messageToHandle.DispatchTime);
		}
	}
	void AIContext::HandleDelayedMessages()
	{
		float currentTime = Utility::Time::GetTime();
		auto& messageQueue = m_MessageQueue;

		// Loop through messageQueue and process messages if they are due for dispatch
		while (!messageQueue.empty())
		{
			// Get the current top message
			const AIMessage& currentMessage = messageQueue.top();

			// Check if current message is due for dispatch
			if (currentMessage.DispatchTime <= currentTime)
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

