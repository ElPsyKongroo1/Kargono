#pragma once
#include "Kargono/Core/Timestep.h"
#include "Modules/Scripting/ScriptModuleBinder.h"
#include "Kargono/Core/UUID.h"

#include <cstdint>
#include <vector>
#include <map>
#include <unordered_set>
#include <queue>

namespace Kargono::States
{
	struct AIMessage
	{
		uint32_t m_MessageType{};
		UUID m_SenderEntity{ Assets::k_EmptyHandle };
		UUID m_ReceiverEntity{ Assets::k_EmptyHandle };
		float m_DispatchTime{ 0.0f };
	};

	// Comparison functor for sorting AIMessages inside StatesContext's MessageQueue
	inline auto k_MessageQueueComparisonFunctor = [](const AIMessage& aiMessageOne, const AIMessage& aiMessageTwo) 
	{
		return aiMessageOne.m_DispatchTime < aiMessageTwo.m_DispatchTime;
	};

	class StatesContext
	{
	public:
		//=========================
		// Lifecycle Functions
		//=========================
		[[nodiscard]] bool Init();
		[[nodiscard]] bool Terminate();  

		//=========================
		// On Event Functions
		//=========================
		void OnUpdate(Timestep timeStep);

		// TODO: THESE FUNCTIONS BELONG IN THE AICOMPONENT!!!!!!
		//=========================
		// Query Entity's State
		//=========================
		bool IsGlobalState(UUID entityID, Assets::AssetHandle queryAIStateHandle);
		bool IsCurrentState(UUID entityID, Assets::AssetHandle queryAIStateHandle);
		bool IsPreviousState(UUID entityID, Assets::AssetHandle queryAIStateHandle);

		//=========================
		// Manage Entity's State
		//=========================
		void ChangeGlobalState(UUID entityID, Assets::AssetHandle newAIStateHandle);
		void ChangeCurrentState(UUID entityID, Assets::AssetHandle newAIStateHandle);
		void RevertPreviousState(UUID entityID);
		void ClearGlobalState(UUID entityID);
		void ClearCurrentState(UUID entityID);
		void ClearPreviousState(UUID entityID);
		void ClearAllStates(UUID entityID);

		//=========================
		// Send AIMessages
		//=========================
		void SendAIMessage(uint32_t messageType, UUID senderEntity, UUID receiverEntity, float delayTime);
	private:
		//=========================
		// Internal Functionality
		//=========================
		void HandleAIMessage(const AIMessage& messageToHandle);
		void HandleDelayedMessages();
		
	private:
		std::priority_queue<AIMessage, std::vector<AIMessage>, decltype(k_MessageQueueComparisonFunctor)> m_MessageQueue{};
	};

	class AIService // TODO: REMOVE EWWWWWWW
	{
	public:
		//==============================
		// Create StateMachines Context
		//==============================
		static void CreateStatesContext()
		{
			// Initialize StatesContext
			if (!s_StatesContext)
			{
				s_StatesContext = CreateRef<States::StatesContext>();
			}

			// Verify init is successful
			KG_VERIFY(s_StatesContext, "AI Service System Initiated");
		}
		static void RemoveStatesContext()
		{
			// Clear StatesContext
			s_StatesContext.reset();
			s_StatesContext = nullptr;

			// Verify terminate is successful
			KG_VERIFY(!s_StatesContext, "AI Service System Initiated");
		}
		//==============================
		// Getters/Setters
		//==============================
		static StatesContext& GetActiveContext() { return *s_StatesContext; }
		static bool IsContextActive() { return (bool)s_StatesContext; }
	private:
		//==============================
		// Internal Fields
		//==============================
		static inline Ref<StatesContext> s_StatesContext{ nullptr };
	};
}
