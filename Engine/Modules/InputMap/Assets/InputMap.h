#pragma once

#include "Kargono/Core/KeyCodes.h"
#include "Kargono/Core/Base.h"
#include "Modules/Assets/AssetsCommon.h"
#include "Modules/Scripting/ScriptModuleBinder.h"
#include "Modules/Events/KeyEvent.h"
#include "Kargono/Core/Timestep.h"
#include "Modules/Assets/Metadata.h"

#include "Modules/InputMap/InputActionBinders/InputActionBinder.h"
#include "Modules/InputMap/InputActionBinders/KeyboardActionBinder.h"
#include "Modules/InputMap/Module/InputMapModule.h"

#include <vector>
#include <unordered_map>
#include <tuple>
#include <functional>
#include <string>

namespace Kargono::InputMap
{
	class InputMap
	{
	public:
		//==============================
		// Asset Config Info
		//==============================
		constexpr static FixedBufStr32 GetAssetName()
		{
			return "Input Map";
		}

		constexpr static Assets::AssetFlags GetAssetFlags()
		{
			Assets::AssetFlags flags{};
			flags.ClearFlag(Assets::AssetFlag::HasAssetCache);
			flags.SetFlag(Assets::AssetFlag::RequireUniqueName);
			return flags;
		}

		constexpr static FixedBufStr16 GetFileExtension()
		{
			return ".kginput";
		}

		static void CreateFromName(Assets::Metadata<InputMap>& metadata);
	public:
		//=========================
		// Constructors/Destructors
		//=========================
		InputMap() = default;
		~InputMap() = default;
	public:
		//==============================
		// Serialization
		//==============================
		void Serialize(void* context);
		void Deserialize(void* context);

	public:
		//==============================
		// Validation
		//==============================
		bool RemoveScript(Assets::AssetHandle scriptHandle);
	public:
		//=========================
		// Getters/Setters
		//=========================
		std::vector<Ref<InputActionBinding>>& GetOnUpdateBindings()
		{
			return m_OnUpdateBindings;
		}
		std::vector<Ref<InputActionBinding>>& GetOnKeyPressedBindings()
		{
			return m_OnKeyPressedBindings;
		}
		std::vector<KeyCode>& GetKeyboardPolling()
		{
			return m_KeyboardPolling;
		}
	public:
		//=========================
		// Public Fields
		//=========================
		// Maps Vector Locations to Engine KeyCodes
		std::vector<KeyCode> m_KeyboardPolling {};
		// OnEvent -> Action Bindings
		std::vector<Ref<InputActionBinding>> m_OnUpdateBindings{};
		std::vector<Ref<InputActionBinding>> m_OnKeyPressedBindings{};
	};

	Register_Module_Type(InputMap, Assets::AssetTag)
}

namespace Kargono::Utility
{
	inline const char* InputActionTypeToString(InputMap::InputActionTypes type)
	{
		switch (type)
		{
		case InputMap::InputActionTypes::KeyboardAction: return "KeyboardAction";
		case InputMap::InputActionTypes::None: return "None";
		}
		KG_ERROR("Unknown Type of InputMap::InputActionTypes.");
		return "";
	}

	inline InputMap::InputActionTypes StringToInputActionType(std::string_view type)
	{
		if (type == "KeyboardAction") { return InputMap::InputActionTypes::KeyboardAction; }
		if (type == "None") { return InputMap::InputActionTypes::None; }

		KG_ERROR("Unknown Type of InputMap::InputActionTypes String.");
		return InputMap::InputActionTypes::None;
	}
}
