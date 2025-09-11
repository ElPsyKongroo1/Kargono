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

	public:
		//==============================
		// Copy Function(s)
		//==============================
		void CopyTo(AIStateComponent* dst)
		{
			// Create the component in place
			std::construct_at<AIStateComponent>(dst);

			dst->m_CurrentStateHandle = m_CurrentStateHandle;
			dst->m_CurrentStateReference = m_CurrentStateReference;
			dst->m_PreviousStateHandle = m_PreviousStateHandle;
			dst->m_PreviousStateReference = m_PreviousStateReference;
			dst->m_GlobalStateHandle = m_GlobalStateHandle;
			dst->m_GlobalStateReference = m_GlobalStateReference;
		}

	public:
		//==============================
		// Public Fields
		//==============================
		// Main state that undergoes state transitions
		Assets::AssetHandle m_CurrentStateHandle{ Assets::k_EmptyHandle };
		Ref<AI::AIState> m_CurrentStateReference{ nullptr };
		// This state allows transitioning into a temporary state for the CurrentState and easily reverting back
		Assets::AssetHandle m_PreviousStateHandle{ Assets::k_EmptyHandle };
		Ref<AI::AIState> m_PreviousStateReference{ nullptr };
		// Globally held state that is ubiquitously available and ran in the OnUpdate() function
		Assets::AssetHandle m_GlobalStateHandle{ Assets::k_EmptyHandle };
		Ref<AI::AIState> m_GlobalStateReference{ nullptr };
	};

	Register_Module_Type(AIStateComponent, ECSInternal::ComponentTag)
}