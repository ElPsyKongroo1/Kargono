#pragma once

#include "Modules/Assets/Module/AssetTag.h"
#include "Modules/Scripting/Module/ScriptingModule.h"
#include "Kargono/Core/FixedBufferString.h"

#include <vector>

namespace Kargono::Scripting
{
	struct CustomEnumMetaData
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		CustomEnumMetaData() = default;
		~CustomEnumMetaData() = default;
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
		FixedBufStr16 m_Name{};
	};

	struct CustomEnum
	{
	public:
		//==============================
		// Metaprogramming Info
		//==============================
		using Metadata = CustomEnumMetaData;
	public:
		//==============================
		// Static Asset Functions
		//==============================
		constexpr static Assets::AssetConfig GetAssetConfig()
		{
			Assets::AssetConfig config{};
			config.m_Identifier = Assets::GetAssetIdentifier<CustomEnum>();
			config.m_Name = "Custom Enum";
			config.m_FileExtension = ".kgenum";
			config.m_RegistryPath = "CustomEnum/CustomEnumRegistry.kgreg";
			config.m_Flags.SetFlag(Assets::AssetFlags::HasAssetCache);
			config.m_Flags.ClearFlag(Assets::AssetFlags::HasIntermediateLocation);
			config.m_Flags.SetFlag(Assets::AssetFlags::HasFileLocation);
			config.m_Flags.ClearFlag(Assets::AssetFlags::HasFileImporting);
			config.m_Flags.SetFlag(Assets::AssetFlags::HasAssetSaving);
			config.m_Flags.SetFlag(Assets::AssetFlags::HasAssetCreationFromName);
			return config;
		}

		static void CreateAssetFromName(Assets::Metadata& metadata, std::string_view name,
			 std::filesystem::path& assetPath);

	public:
		//=========================
		// Constructors/Destructors
		//=========================
		CustomEnum() = default;
		~CustomEnum() = default;
	public:
		//==============================
		// Serialization
		//==============================
		void Serialize(void* context);
		void Deserialize(void* context);
	public:
		//=========================
		// Query State
		//=========================
		bool DoesContainIdentifier(const char* queryName);
	public:
		//=========================
		// Modify State
		//=========================
		bool RemoveIdentifier(const char* queryName);
		bool RemoveIdentifier(size_t indexToDelete);
		bool RenameIdentifier(size_t indexToModify, const char* newName);
	public:
		//=========================
		// Public Fields
		//=========================
		FixedBufStr32 m_EnumName{};
		std::vector<FixedBufStr32> m_EnumIdentifiers{};
	};

	Register_Module_Type(CustomEnum, Assets::AssetTag)
}
