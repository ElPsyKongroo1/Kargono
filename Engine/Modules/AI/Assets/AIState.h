#pragma once

#include "Modules/AI/Module/AIModule.h"
#include "Modules/Assets/AssetsCommon.h"
#include "Modules/Scripting/ScriptModuleBinder.h"

namespace Kargono::AI
{
	struct AIState
	{
	public:
		//==============================
		// Static Asset Functions
		//==============================
		constexpr static Assets::AssetConfig GetAssetConfig()
		{
			Assets::AssetConfig config{};
			config.m_Identifier = Assets::GetAssetIdentifier<AIState>();
			config.m_Name = "AI State";
			config.m_FileExtension = ".kgaistate";
			config.m_RegistryPath = "AIState/AIStateRegistry.kgreg";
			config.m_IntermediateExtension = "";
			config.m_Flags.SetFlag(Assets::AssetFlags::HasAssetCache);
			config.m_Flags.ClearFlag(Assets::AssetFlags::HasIntermediateLocation);
			config.m_Flags.SetFlag(Assets::AssetFlags::HasFileLocation);
			config.m_Flags.ClearFlag(Assets::AssetFlags::HasFileImporting);
			config.m_Flags.SetFlag(Assets::AssetFlags::HasAssetSaving);
			config.m_Flags.SetFlag(Assets::AssetFlags::HasAssetCreationFromName);
			return config;
		}
		static void CreateAssetFileFromName(std::string_view name, Assets::Metadata& metadata, std::filesystem::path& path);

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
		void DeleteValidation(Assets::Metadata& metadata);
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