#pragma once

#include "Modules/AI/Module/AIModule.h"
#include "Modules/Assets/AssetsCommon.h"
#include "Modules/Assets/Metadata.h"
#include "Modules/Scripting/ScriptModuleBinder.h"
#include "Modules/Assets/Module/AssetTag.h"

namespace Kargono::AI
{
	struct AIState
	{
	public:
		//==============================
		// Asset Config Info
		//==============================
		constexpr static FixedBufStr32 GetAssetName()
		{
			return "AI State";
		}

		constexpr static Assets::AssetFlags GetAssetFlags()
		{
			Assets::AssetFlags flags{};
			flags.SetFlag(Assets::AssetFlag::HasAssetCache);
			flags.SetFlag(Assets::AssetFlag::RequireUniqueName);
			flags.SetFlag(Assets::AssetFlag::AllowDefaultUpdateAsset);
			return flags;
		}
		constexpr static FixedBufStr16 GetFileExtension()
		{
			return ".kgaistate";
		}
		static void CreateFromName(Assets::Metadata& metadata);

	public:
		//==============================
		// Constructors/Destructors
		//==============================
		AIState() = default;
		~AIState() = default;

	public:
		//==============================
		// Validation
		//==============================
		void ValidateDelete(Assets::Metadata& metadata);
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
		Assets::AssetHandle m_OnUpdateHandle{ Assets::k_EmptyHandle };
		Ref<Scripting::Script> m_OnUpdate{ nullptr };
		// Called when this state is entered
		Assets::AssetHandle m_OnEnterStateHandle{ Assets::k_EmptyHandle };
		Ref<Scripting::Script> m_OnEnterState{ nullptr };
		// Called when this state is exited
		Assets::AssetHandle m_OnExitStateHandle{ Assets::k_EmptyHandle };
		Ref<Scripting::Script> m_OnExitState{ nullptr };
		// Called when a message is received
		Assets::AssetHandle m_OnMessageHandle{ Assets::k_EmptyHandle };
		Ref<Scripting::Script> m_OnMessage{ nullptr };
	};

	Register_Module_Type(AIState, Assets::AssetTag)
}