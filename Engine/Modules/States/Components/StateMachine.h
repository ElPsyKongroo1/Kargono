#pragma once

#include "Modules/Assets/AssetsCommon.h"
#include "Modules/States/Assets/State.h"
#include "Modules/States/Module/StatesModule.h"
#include "Modules/ECSInternal/Module/ComponentTag.h"

namespace Kargono::States
{
	struct StateMachine
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		StateMachine() = default;
		~StateMachine() = default;

	public:
		//==============================
		// Copy Function(s)
		//==============================
		void CopyTo(StateMachine* dst)
		{
			// Create the component in place
			std::construct_at<StateMachine>(dst);

			dst->m_CurrentStateReference = m_CurrentStateReference;
			dst->m_PreviousStateReference = m_PreviousStateReference;
			dst->m_GlobalStateReference = m_GlobalStateReference;
		}
	public:
		//==============================
		// Serialization
		//==============================
		void Serialize(void* context);
		void Deserialize(void* context);
	public:
		//==============================
		// Public Fields
		//==============================
		// Main state that undergoes state transitions
		Assets::TAssetRef<State> m_CurrentStateReference{};
		// This state allows transitioning into a temporary state for the CurrentState and easily reverting back
		Assets::TAssetRef<State> m_PreviousStateReference{};
		// Globally held state that is ubiquitously available and ran in the OnUpdate() function
		Assets::TAssetRef<State> m_GlobalStateReference{};
	};

	Register_Module_Type(StateMachine, ECSInternal::ComponentTag)
}