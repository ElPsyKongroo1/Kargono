#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Core/WrappedData.h"
#include "Modules/ECSInternal/ECSInternalCommon.h"
#include "Modules/ECSInternal/Module/ECSInternalModule.h"
#include "Modules/ECSInternal/Module/ComponentTag.h"
#include "Modules/Assets/Concepts/OptionalAssetConcepts.h"
#include "Modules/Assets/Module/AssetTag.h"

#include <array>
#include <cstdint>
#include <vector>
#include <string>
#include <limits>
#include <algorithm>

namespace Kargono::Scenes { class Scene; }

namespace Kargono::ECSInternal
{
	constexpr size_t k_NewAllocationIndex{ std::numeric_limits<size_t>().max() };

	struct FieldReallocationInstructions
	{
		std::vector<size_t> m_FieldTransferDirections;
		std::vector<WrappedVarType> m_OldDataTypes;
		std::vector<WrappedVarType> m_NewDataTypes;
		std::vector<uint64_t> m_OldDataLocations;
		std::vector<uint64_t> m_NewDataLocations;
		size_t m_NewDataSize;
		std::vector<Ref<Scenes::Scene>> m_OldScenes;
		std::vector<Assets::AssetHandle> m_OldSceneHandles;
	};

	struct CustomComponentMetaData
	{
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

	struct CustomComponent
	{
	public:
		//==============================
		// Metaprogramming Info
		//==============================
		using Metadata = CustomComponentMetaData;
	public:
		//==============================
		// Static Asset Functions
		//==============================
		constexpr static Assets::AssetConfig GetAssetConfig()
		{
			Assets::AssetConfig config{};
			config.m_Identifier = Assets::GetAssetIdentifier<CustomComponent>();
			config.m_Name = "Custom Component";
			config.m_FileExtension = ".kgcomponent";
			config.m_ImportExtensions = {};
			config.m_RegistryPath = "CustomComponent/CustomComponentRegistry.kgreg";
			config.m_IntermediateExtension = "";
			config.m_Flags.SetFlag(Assets::AssetFlags::HasAssetCache);
			config.m_Flags.ClearFlag(Assets::AssetFlags::HasIntermediateLocation);
			config.m_Flags.SetFlag(Assets::AssetFlags::HasFileLocation);
			config.m_Flags.ClearFlag(Assets::AssetFlags::HasFileImporting);
			config.m_Flags.SetFlag(Assets::AssetFlags::HasAssetSaving);
			config.m_Flags.SetFlag(Assets::AssetFlags::HasAssetCreationFromName);
			return config;
		}
		static void CreateAssetFileFromName(std::string_view name,
			Assets::Metadata& metadata, std::filesystem::path& assetPath);
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		CustomComponent() = default;
		~CustomComponent() = default;
	public:
		//==============================
		// Interact w/ Fields
		//==============================
		bool AddField(WrappedVarType fieldType, const char* fieldName);
		void DeleteField(size_t fieldIndex);
		bool EditField(size_t fieldIndex, const char* fieldName, WrappedVarType fieldType);
	private:
		// Helpers
		void RecalculateDataLocations();
	public:
		//==============================
		// Copy Function(s)
		//==============================
		void CopyTo(void* src, void* dst)
		{
			KG_ASSERT(src);
			KG_ASSERT(dst);
			KG_ASSERT(m_DataNames.size() == m_DataTypes.size());
			KG_ASSERT(m_DataTypes.size() == m_DataOffsets.size());

			for (size_t i{ 0 }; i < m_DataTypes.size(); i++)
			{
				Utility::TransferDataForWrappedVarBuffer
				(
					m_DataTypes[i],
					(uint8_t*)src + m_DataOffsets[i],
					(uint8_t*)dst + m_DataOffsets[i]
				);
			}
		}
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
		Ref<void> SaveValidation(Assets::AssetReference<CustomComponent> newAssetRef, Assets::Metadata& metadata);
		void DeleteValidation(Assets::Metadata& metadata);
	public:
		//==============================
		// Getters/Setters
		//==============================
		ComponentIdentifier RevalidateIdentifier();
		ComponentMetadata GenerateMetadata(Assets::AssetHandle compHandle) const;
		size_t RevalidateAlignment();
	public:
		//==============================
		// Public Fields
		//==============================
		FixedBufStr32 m_Name{};
		size_t m_ComponentSize{ 0 };
		size_t m_ComponentAlignment{ 1 };
		ComponentIdentifier m_Identifier{ k_InvalidComponentIdentifier };
		std::vector<WrappedVarType> m_DataTypes;
		std::vector<size_t> m_DataOffsets;
		std::vector<FixedBufStr32> m_DataNames;
	};

	Register_Module_Type(CustomComponent, Assets::AssetTag)

	inline void CustomComponentCopyTo(void* src, void* dst, void* customComp)
	{
		KG_ASSERT(src);
		KG_ASSERT(dst);
		KG_ASSERT(customComp);

		CustomComponent* customComponent{(CustomComponent*)customComp};

		customComponent->CopyTo(src, dst);
	}

}
