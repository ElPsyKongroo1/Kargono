#pragma once

#include "Modules/AI/AIService.h"
#include "Modules/Assets/Asset.h"
#include "Modules/AI/Module/AIModule.h"
#include "Modules/ECSInternal/Module/ComponentTag.h"

namespace Kargono::AI
{
	struct AIStateComponent
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		AIStateComponent() = default;
		~AIStateComponent() = default;

		//==============================
		// Copy Function(s)
		//==============================
		void CopyTo(AIStateComponent* dst)
		{
			AIStateComponent* destination = (AIStateComponent*)dst;
			destination->m_CurrentStateHandle = m_CurrentStateHandle;
			destination->m_CurrentStateReference = m_CurrentStateReference;
			destination->m_PreviousStateHandle = m_PreviousStateHandle;
			destination->m_PreviousStateReference = m_PreviousStateReference;
			destination->m_GlobalStateHandle = m_GlobalStateHandle;
			destination->m_GlobalStateReference = m_GlobalStateReference;
		}

	public:
		//==============================
		// Public Fields
		//==============================
		// Main state that undergoes state transitions
		Assets::AssetHandle m_CurrentStateHandle{ Assets::EmptyHandle };
		Ref<AI::AIState> m_CurrentStateReference{ nullptr };
		// This state allows transitioning into a temporary state for the CurrentState and easily reverting back
		Assets::AssetHandle m_PreviousStateHandle{ Assets::EmptyHandle };
		Ref<AI::AIState> m_PreviousStateReference{ nullptr };
		// Globally held state that is ubiquitously available and ran in the OnUpdate() function
		Assets::AssetHandle m_GlobalStateHandle{ Assets::EmptyHandle };
		Ref<AI::AIState> m_GlobalStateReference{ nullptr };
	};

	Register_Module_Type(AIStateComponent, ECSInternal::ComponentTag)
}