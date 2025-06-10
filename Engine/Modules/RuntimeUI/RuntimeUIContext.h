#pragma once

#include "Modules/Rendering/EditorPerspectiveCamera.h"
#include "Modules/Scripting/ScriptService.h"
#include "Kargono/Core/Base.h"
#include "Modules/RuntimeUI/Font.h"
#include "Modules/Assets/Asset.h"
#include "Kargono/Math/Math.h"
#include "Modules/RuntimeUI/RuntimeUICommon.h"
#include "Kargono/Core/Window.h"
#include "Kargono/Utility/Random.h"
#include "Modules/Rendering/Shader.h"

#include "Modules/RuntimeUI/RuntimeUIUserInterface.h"

#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include <tuple>

namespace Kargono::RuntimeUI
{
	struct RuntimeUIContext
	{
	public:
		//==============================
		// Lifecycle Functions
		//==============================
		void Init();
		void Terminate();
	public:
		//==============================
		// Getters/Setters
		//==============================
		// Set UI
		void SetActiveUI(Ref<UserInterface> userInterface, Assets::AssetHandle uiHandle);
		void SetActiveUIFromHandle(Assets::AssetHandle uiHandle);
		// Query UI
		bool IsUIActiveFromHandle(Assets::AssetHandle uiHandle);
		// Get UI
		Ref<UserInterface> GetActiveUI();
		Assets::AssetHandle GetActiveUIHandle();
		// Modify UI
		void ClearActiveUI();
	public:
		//==============================
		// Public Fields
		//==============================
		// Active UI
		Ref<UserInterface> m_ActiveUI{ nullptr };
		Assets::AssetHandle m_ActiveUIHandle{ Assets::EmptyHandle };
		// Default values
		Ref<Font> m_DefaultFont{ nullptr };
		// Rendering data
		Rendering::RendererInputSpec m_BackgroundInputSpec{};
		Rendering::RendererInputSpec m_ImageInputSpec{};

	private:
		//==============================
		// Internal Fields
		//==============================
		bool m_Active{ false };
	};

	class RuntimeUIService // EWWWWW
	{
	public:
		//==============================
		// Create RuntimeUI Context
		//==============================
		static void CreateRuntimeUIContext()
		{
			// Initialize RuntimeUIWorld
			if (!s_RuntimeUIContext)
			{
				s_RuntimeUIContext = CreateRef<RuntimeUIContext>();
			}

			// Verify init is successful
			KG_VERIFY(s_RuntimeUIContext, "RuntimeUI Service System Initiated");
		}
		static void RemoveRuntimeUIContext()
		{
			// Clear RuntimeUIWorld
			s_RuntimeUIContext.reset();
			s_RuntimeUIContext = nullptr;

			// Verify terminate is successful
			KG_VERIFY(!s_RuntimeUIContext, "RuntimeUI Service System Initiated");
		}
		//==============================
		// Getters/Setters
		//==============================
		static RuntimeUIContext& GetActiveContext() { return *s_RuntimeUIContext; }
		static bool IsContextActive() { return (bool)s_RuntimeUIContext; }
	private:
		//==============================
		// Internal Fields
		//==============================
		static inline Ref<RuntimeUIContext> s_RuntimeUIContext{ nullptr };
	};
}
