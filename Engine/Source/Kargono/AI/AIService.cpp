#include "kgpch.h"

#include "Kargono/AI/AIService.h"
#include "Kargono/Scenes/Scene.h"
#include "Kargono/ECS/Entity.h"
#include "Kargono/Projects/Project.h"
#include "Kargono/Utility/FileSystem.h"
#include "Kargono/Assets/AssetService.h"


namespace Kargono::AI
{
	void AIService::Init()
	{
		// Initialize AIContext
		if (!s_AIContext)
		{
			s_AIContext = CreateRef<AI::AIContext>();

			// Load in Message Types from active project (vector<string> -> unordered_set<uint64_t>)
			KG_ASSERT(Projects::ProjectService::GetActive());
			for (const std::string& messageName : Projects::ProjectService::GetAllMessageTypes())
			{
				auto [iterator, success] = s_AIContext->AllMessageTypes.insert(Utility::FileSystem::CRCFromString(messageName));
				KG_ASSERT(success);
			}
		}
		
		// Verify init is successful
		KG_VERIFY(s_AIContext, "AI Service System Initiated");
	}
	void AIService::Terminate()
	{
		// Clear AIContext
		s_AIContext.reset();
		s_AIContext = nullptr;
		
		// Verify terminate is successful
		KG_VERIFY(!s_AIContext, "AI Service System Initiated");
	}
	void AIService::OnUpdate(Timestep timeStep)
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
				Utility::CallWrappedVoidUInt64Float(aiComponent.GlobalStateReference->OnUpdate->m_Function, entity.GetUUID(), timeStep);
			}

			// Call Current State OnUpdate
			if (aiComponent.CurrentStateReference)
			{
				Utility::CallWrappedVoidUInt64Float(aiComponent.CurrentStateReference->OnUpdate->m_Function, entity.GetUUID(), timeStep);
			}
		}

		// Check/Handle AIContext's delayed messages queue
		HandleDelayedMessages(timeStep);
	}
	void AIService::ChangeGlobalState(UUID entityID, Assets::AssetHandle newAIStateHandle)
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
			Utility::CallWrappedVoidUInt64(aiComponent.GlobalStateReference->OnExitState->m_Function, entityID);
		}

		// Switch to new global AIState
		aiComponent.GlobalStateHandle = newAIStateHandle;
		aiComponent.GlobalStateReference = newAIStateRef;

		// Call OnEnter for new global state
		if (aiComponent.GlobalStateReference && aiComponent.GlobalStateReference->OnEnterState)
		{
			Utility::CallWrappedVoidUInt64(aiComponent.GlobalStateReference->OnEnterState->m_Function, entityID);
		}

	}
	void AIService::ChangeCurrentState(UUID entityID, Assets::AssetHandle newAIStateHandle)
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
			Utility::CallWrappedVoidUInt64(aiComponent.CurrentStateReference->OnExitState->m_Function, entityID);
		}

		// Switch to new AIState
		aiComponent.CurrentStateHandle = newAIStateHandle;
		aiComponent.CurrentStateReference = newAIStateRef;

		// Call OnEnter for new state
		if (aiComponent.CurrentStateReference && aiComponent.CurrentStateReference->OnEnterState)
		{
			Utility::CallWrappedVoidUInt64(aiComponent.CurrentStateReference->OnEnterState->m_Function, entityID);
		}
	}
	void AIService::RevertToPreviousState(UUID entityID)
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
			Utility::CallWrappedVoidUInt64(aiComponent.CurrentStateReference->OnExitState->m_Function, entityID);
		}

		// Call ChangeState() into entityID's previous state if it exists
		aiComponent.CurrentStateHandle = aiComponent.PreviousStateHandle;
		aiComponent.CurrentStateReference = aiComponent.PreviousStateReference;

		// Call OnEnter for new current state
		if (aiComponent.CurrentStateReference && aiComponent.CurrentStateReference->OnEnterState)
		{
			Utility::CallWrappedVoidUInt64(aiComponent.CurrentStateReference->OnEnterState->m_Function, entityID);
		}

		// Clear previous state
		aiComponent.PreviousStateHandle = Assets::EmptyHandle;
		aiComponent.PreviousStateReference = nullptr;
		
	}
	void AIService::SendAIMessage(uint32_t messageType, UUID senderEntity, UUID receiverEntity, float messageDelay)
	{
		// Initialize message
		static constexpr float k_IncrementMessageTime { 0.00025f };
		KG_ASSERT(s_AIContext);
		AIMessage newMessage{ messageType, senderEntity, receiverEntity, messageDelay };

		// Check if message should be handled immediately or placed into delay queue
		if (messageDelay > 0.0f)
		{
			// Insert message into delay queue. If unsuccessful, increment delay time and retry until successful
			bool insertionSuccess = false;
			while (!insertionSuccess)
			{
				auto [iterator, success] = s_AIContext->MessageQueue.insert({messageDelay, newMessage});
				insertionSuccess = success;
				messageDelay += k_IncrementMessageTime;
			}
		}
		else
		{
			// Handle message
			HandleAIMessage(std::move(newMessage));
		}
	}
	void AIService::HandleAIMessage(AIMessage&& messageToHandle)
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
			Utility::CallWrappedVoidUInt32UInt64UInt64Float(receiverAIComponent.GlobalStateReference->OnMessage->m_Function, messageToHandle.MessageType, messageToHandle.SenderEntity, messageToHandle.ReceiverEntity, messageToHandle.Delay);
		}

		// Call OnMessage for recipient's current state
		if (receiverAIComponent.CurrentStateReference && receiverAIComponent.CurrentStateReference->OnMessage)
		{
			Utility::CallWrappedVoidUInt32UInt64UInt64Float(receiverAIComponent.CurrentStateReference->OnMessage->m_Function, messageToHandle.MessageType, messageToHandle.SenderEntity, messageToHandle.ReceiverEntity, messageToHandle.Delay);
		}
	}
	void AIService::HandleDelayedMessages(Timestep timeStep)
	{
		// Check all delayed messages in current AIContext
		for (auto& [delayTime, message] : s_AIContext->MessageQueue)
		{
			// Decriment each message's delay using the timestep

			// Check if each message should be handled
		}

		// Call HandleAIMessage() for delay's <= 0.0f

		// Remove all handled messages

	}
}

