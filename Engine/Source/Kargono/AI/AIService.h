#pragma once
#include "Kargono/Core/Timestep.h"
#include "Kargono/Scripting/ScriptService.h"
#include "Kargono/Core/UUID.h"

#include <cstdint>
#include <vector>
#include <map>

namespace Kargono::AI
{
	//=========================
	// AI Message Class
	//=========================
	struct AIMessage
	{
		uint64_t MessageType{};
		UUID SenderEntity{ Assets::EmptyHandle };
		UUID ReceiverEntity{ Assets::EmptyHandle };
		float Delay{ 0.0f };
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
		Assets::AssetHandle OnAIMessageHandle { Assets::EmptyHandle };
		Ref<Scripting::Script> OnAIMessage { nullptr };
	};

	//=========================
	// AI Context Class
	//=========================
	struct AIContext
	{
		std::map<float, AIMessage> AllMessages {};
		std::vector<std::string> AllMessageTypes{};
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
		// Manage Entity's AIState
		//=========================
		static void ChangeCurrentState(UUID entityID, Assets::AssetHandle newAIState);
		static void RevertToPreviousState(UUID entityID);
		static void ChangeGlobalState(UUID entityID, Assets::AssetHandle newAIState);

		//=========================
		// Send AIMessages
		//=========================
		static void SendAIMessage(uint64_t messageType, UUID senderEntity, UUID receiverEntity, float messageDelay);
	private:
		//=========================
		// Internal Functionality
		//=========================
		static void HandleAIMessage(AIMessage& messageToHandle);
		static void HandleDelayedMessages(Timestep timeStep);
		
	private:
		static inline Ref<AIContext> s_AIContext{ nullptr };
	};
}
