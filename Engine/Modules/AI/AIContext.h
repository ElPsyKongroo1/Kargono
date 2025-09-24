#pragma once
#include "Kargono/Core/Timestep.h"
#include "Modules/Scripting/ScriptModuleBinder.h"
#include "Kargono/Core/UUID.h"

#include <cstdint>
#include <vector>
#include <map>
#include <unordered_set>
#include <queue>

namespace Kargono::AI
{
	struct AIMessage
	{
		uint32_t m_MessageType{};
		UUID m_SenderEntity{ Assets::k_EmptyHandle };
		UUID m_ReceiverEntity{ Assets::k_EmptyHandle };
		float m_DispatchTime{ 0.0f };
	};

	// Comparison functor for sorting AIMessages inside AIContext's MessageQueue
	inline auto k_MessageQueueComparisonFunctor = [](const AIMessage& aiMessageOne, const AIMessage& aiMessageTwo) 
	{
		return aiMessageOne.m_DispatchTime < aiMessageTwo.m_DispatchTime;
	};

	class AIContext
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
		// Query Entity's AIState
		//=========================
		bool IsGlobalState(UUID entityID, Assets::AssetHandle queryAIStateHandle);
		bool IsCurrentState(UUID entityID, Assets::AssetHandle queryAIStateHandle);
		bool IsPreviousState(UUID entityID, Assets::AssetHandle queryAIStateHandle);

		//=========================
		// Manage Entity's AIState
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
		// Create AI Context
		//==============================
		static void CreateAIContext()
		{
			// Initialize AIContext
			if (!s_AIContext)
			{
				s_AIContext = CreateRef<AI::AIContext>();
			}

			// Verify init is successful
			KG_VERIFY(s_AIContext, "AI Service System Initiated");
		}
		static void RemoveAIContext()
		{
			// Clear AIContext
			s_AIContext.reset();
			s_AIContext = nullptr;

			// Verify terminate is successful
			KG_VERIFY(!s_AIContext, "AI Service System Initiated");
		}
		//==============================
		// Getters/Setters
		//==============================
		static AIContext& GetActiveContext() { return *s_AIContext; }
		static bool IsContextActive() { return (bool)s_AIContext; }
	private:
		//==============================
		// Internal Fields
		//==============================
		static inline Ref<AIContext> s_AIContext{ nullptr };
	};
}
