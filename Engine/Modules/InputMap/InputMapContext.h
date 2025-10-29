#pragma once

#include "Modules/InputMap/Assets/InputMap.h"
#include "Modules/Assets/AssetsCommon.h"
#include "Kargono/Core/Base.h"

#include <vector>

namespace Kargono::InputMap
{
	class InputMapContext
	{
	public:
		//=========================
		// Lifecycle Functions
		//=========================
		[[nodiscard]] bool Init();
		[[nodiscard]] bool Terminate();

		//=========================
		// On Event Functionality
		//=========================
		void OnUpdate(Timestep ts);
		bool OnKeyPressed(Events::KeyPressedEvent event);

		//=========================
		// Active Input Map API
		//=========================
		bool IsPollingSlotPressed(uint16_t slot);
		std::vector<Ref<InputActionBinding>>& GetActiveOnUpdate();
		std::vector<Ref<InputActionBinding>>& GetActiveOnKeyPressed();

		//=========================
		// Getter/Setter
		//=========================
		void ClearActiveInputMap();
		void SetActiveInputMap(Ref<InputMap> newInput, Assets::AssetHandle newHandle);
		void SetActiveInputMapFromHandle(Assets::AssetHandle inputMapHandle);
		Ref<InputMap> GetActiveInputMap()
		{
			return m_ActiveInputMap;
		}
		Assets::AssetHandle GetActiveInputMapHandle()
		{
			return m_ActiveInputMapHandle;
		}
	private:
		//=========================
		// Current Input Mode
		//=========================
		Ref<InputMap> m_ActiveInputMap{ nullptr };
		Assets::AssetHandle m_ActiveInputMapHandle{ Assets::k_EmptyHandle };
	};

	class InputMapService // TODO: REMOVE EWWWWWWW
	{
	public:
		//==============================
		// Create InputMap Context
		//==============================
		static void CreateInputMapContext()
		{
			// Initialize InputMapContext
			if (!s_InputMapContext)
			{
				s_InputMapContext = CreateRef<InputMapContext>();
			}

			// Verify init is successful
			KG_VERIFY(s_InputMapContext, "InputMap Service System Initiated");
		}
		static void RemoveInputMapContext()
		{
			// Clear InputMapContext
			s_InputMapContext.reset();
			s_InputMapContext = nullptr;

			// Verify terminate is successful
			KG_VERIFY(!s_InputMapContext, "InputMap Service System Initiated");
		}
		//==============================
		// Getters/Setters
		//==============================
		static InputMapContext& GetActiveContext() { return *s_InputMapContext; }
		static bool IsContextActive() { return (bool)s_InputMapContext; }
	private:
		//==============================
		// Internal Fields
		//==============================
		static inline Ref<InputMapContext> s_InputMapContext{ nullptr };
	};
}