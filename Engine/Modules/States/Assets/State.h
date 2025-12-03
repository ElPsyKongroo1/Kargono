#pragma once

#include "Modules/States/Module/StatesModule.h"
#include "Modules/Assets/AssetsCommon.h"
#include "Modules/Assets/Metadata.h"
#include "Modules/Scripting/ScriptModuleBinder.h"
#include "Modules/Assets/Module/AssetTag.h"
#include "Modules/Assets/AssetReference.h"

namespace Kargono::States
{
	struct State
	{
	public:
		//==============================
		// Asset Config Info
		//==============================
		constexpr static FixedBufStr32 GetDisplayName()
		{
			return "State";
		}

		constexpr static Assets::AssetFlags GetAssetFlags()
		{
			Assets::AssetFlags flags{};
			flags.SetFlag(Assets::AssetFlag::HasAssetCache);
			flags.SetFlag(Assets::AssetFlag::RequireUniqueName);
			return flags;
		}
		constexpr static FixedBufStr16 GetFileExtension()
		{
			return ".kgstate";
		}
		static void CreateFromName(Assets::Metadata<State>& metadata);

	public:
		//==============================
		// Constructors/Destructors
		//==============================
		State() = default;
		~State() = default;

	public:
		//==============================
		// Validation
		//==============================
		void ValidateDelete(Assets::Metadata<State>& metadata);
		bool RemoveScript(Assets::AssetHandle scriptHandle);
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
		// Called each frame if entity has AIComponent
		Assets::AssetRef<Scripting::Script> m_OnUpdate{};
		// Called when this state is entered
		Assets::AssetRef<Scripting::Script> m_OnEnterState{};
		// Called when this state is exited
		Assets::AssetRef<Scripting::Script> m_OnExitState{};
		// Called when a message is received
		Assets::AssetRef<Scripting::Script> m_OnMessage{};
	};

	Register_Module_Type(State, Assets::AssetTag)
}