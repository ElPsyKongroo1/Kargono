#include "kgpch.h"

#include "Kargono/AI/AIService.h"
#include "Kargono/Scenes/Scene.h"
#include "Kargono/ECS/Entity.h"

namespace Kargono::AI
{
	void AIService::Init()
	{
		// Initialize AIContext

		// Load in Message Types 

		// Verify init is successful
	}
	void AIService::Terminate()
	{
		// Clear AIContext

		// Verify terminate is successful
	}
	void AIService::OnUpdate(Timestep timeStep)
	{
		// Ensure a valid scene is active
		Scenes::Scene* activeScene = Scenes::SceneService::GetActiveScene().get();
		KG_ASSERT(activeScene, "Invalid scene reference when calling AIService's OnUpdate()");

		// Run on update for all active AI with AIComponents including the global, then the current state
		for (auto enttID : Scenes::SceneService::GetActiveScene()->GetAllEntitiesWith<ECS::AIStateComponent>())
		{
			ECS::Entity entity { enttID, &activeScene->m_Registry };
			KG_ASSERT(entity, "Invalid entity obtained. Could not run OnUpdate on provided entity.");
			ECS::AIStateComponent& aiComponent = entity.GetComponent<ECS::AIStateComponent>();

			// Call Global State OnUpdate
			if (aiComponent.GlobalStateReference && aiComponent.GlobalStateReference->OnUpdate)
			{
				Utility::CallWrappedVoidUInt64(aiComponent.GlobalStateReference->OnUpdate->m_Function, entity.GetUUID());
			}

			// Call Current State OnUpdate
			if (aiComponent.CurrentStateReference)
			{
				Utility::CallWrappedVoidUInt64(aiComponent.CurrentStateReference->OnUpdate->m_Function, entity.GetUUID());
			}
		}

		// Check/Handle AIContext's delayed messages queue
		HandleDelayedMessages(timeStep);
	}
	void AIService::ChangeGlobalState(UUID entityID, Assets::AssetHandle newAIState)
	{
		// Ensure a valid scene is active and a valid entity is provided
		Scenes::Scene* activeScene = Scenes::SceneService::GetActiveScene().get();
		KG_ASSERT(activeScene, "Invalid scene reference when calling AIService's OnUpdate()");
		ECS::Entity entity = activeScene->GetEntityByUUID(entityID);
		KG_ASSERT(entity, "Invalid entity obtained. Could not update global state.");

		// Ensure new AIState is valid
		//Ref<AIState> newState = Assets::AssetManager::GetAIState();
		//KG_ASSERT(newState, "Invalid new AI state provided. Could not update global state.");

		ECS::AIStateComponent& aiComponent = entity.GetComponent<ECS::AIStateComponent>();

		// Call OnExitState() for active global AIState
		if (aiComponent.GlobalStateReference && aiComponent.GlobalStateReference->OnExitState)
		{
			Utility::CallWrappedVoidUInt64(aiComponent.GlobalStateReference->OnExitState->m_Function, entityID);
		}

		// Switch to new global AIState


		// Call OnEnter for new global state

	}
	void AIService::ChangeCurrentState(UUID entityID, Assets::AssetHandle newAIState)
	{
		// Store current state as the new previous state

		// Call OnExitState() for current AIState

		// Switch to new AIState

		// Call OnEnter for new state
	}
	void AIService::RevertToPreviousState(UUID entityID)
	{
		// Call ChangeState() into entityID's previous state if it exists
	}
	void AIService::SendAIMessage(uint64_t messageType, UUID senderEntity, UUID receiverEntity, float messageDelay)
	{
		// Route message to either the HandleAIMessage() function or the HandleDelayedMessages() function
		//		based on the messageDelay
	}
	void AIService::HandleAIMessage(AIMessage& messageToHandle)
	{
		// Call OnMessage() for recipient
	}
	void AIService::HandleDelayedMessages(Timestep timeStep)
	{
		// Check all delayed messages in current AIContext

		// Decriment each message's delay using the timestep

		// Check if each message should be handled

		// Call HandleAIMessage() for delay's <= 0.0f

		// Remove all handled messages

	}
}

