#pragma once
#include "Kargono/Core/Timestep.h"
#include "ScriptModule/ScriptService.h"
#include "Kargono/Core/UUID.h"

#include <cstdint>
#include <vector>
#include <map>
#include <unordered_set>
#include <queue>

namespace Kargono::AI
{
	//=========================
	// AI Message Class
	//=========================
	struct AIMessage
	{
		uint32_t MessageType{};
		UUID SenderEntity{ Assets::EmptyHandle };
		UUID ReceiverEntity{ Assets::EmptyHandle };
		float DispatchTime{ 0.0f };
	};


	//=========================
	// AI State Class
	//=========================
	struct AIState
	{
		// Called each frame if entity has AIComponent
		Assets::AssetHandle OnUpdateHandle { Assets::EmptyHandle };
		Ref<Scripting::Script> OnUpdate { nullptr };

		// Called when this state is entered
		Assets::AssetHandle OnEnterStateHandle { Assets::EmptyHandle };
		Ref<Scripting::Script> OnEnterState { nullptr };

		// Called when this state is exited
		Assets::AssetHandle OnExitStateHandle { Assets::EmptyHandle };
		Ref<Scripting::Script> OnExitState { nullptr };

		// Called when a message is received
		Assets::AssetHandle OnMessageHandle { Assets::EmptyHandle };
		Ref<Scripting::Script> OnMessage { nullptr };
	};

	//=========================
	// AI Context Class
	//=========================

	// Comparison functor for sorting AIMessages inside AIContext's MessageQueue
	inline auto k_MessageQueueComparisonFunctor = [](const AIMessage& aiMessageOne, const AIMessage& aiMessageTwo) 
	{
		return aiMessageOne.DispatchTime < aiMessageTwo.DispatchTime;
	};

	struct AIContext
	{
		std::priority_queue<AIMessage, std::vector<AIMessage>, decltype(k_MessageQueueComparisonFunctor)> m_MessageQueue {};
	};


	//=========================
	// AI Service Class
	//=========================
	class AIService
	{
	public:
		//=========================
		// Lifecycle Functions
		//=========================
		static void Init();
		static void Terminate();  

		//=========================
		// On Event Functions
		//=========================
		static void OnUpdate(Timestep timeStep);

		//=========================
		// Query Entity's AIState
		//=========================
		static bool IsGlobalState(UUID entityID, Assets::AssetHandle queryAIStateHandle);
		static bool IsCurrentState(UUID entityID, Assets::AssetHandle queryAIStateHandle);
		static bool IsPreviousState(UUID entityID, Assets::AssetHandle queryAIStateHandle);

		//=========================
		// Manage Entity's AIState
		//=========================
		static void ChangeGlobalState(UUID entityID, Assets::AssetHandle newAIStateHandle);
		static void ChangeCurrentState(UUID entityID, Assets::AssetHandle newAIStateHandle);
		static void RevertPreviousState(UUID entityID);
		static void ClearGlobalState(UUID entityID);
		static void ClearCurrentState(UUID entityID);
		static void ClearPreviousState(UUID entityID);
		static void ClearAllStates(UUID entityID);

		//=========================
		// Send AIMessages
		//=========================
		static void SendAIMessage(uint32_t messageType, UUID senderEntity, UUID receiverEntity, float delayTime);
	private:
		//=========================
		// Internal Functionality
		//=========================
		static void HandleAIMessage(const AIMessage& messageToHandle);
		static void HandleDelayedMessages();
		
	private:
		static inline Ref<AIContext> s_AIContext{ nullptr };
	};
}
