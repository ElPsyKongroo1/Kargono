#pragma once

#include "Kargono/Core/WrappedData.h"
#include "Modules/Core/Module.h"
#include "Kargono/Core/FixedBufferString.h"
#include "Modules/Assets/AssetsCommon.h"
#include "Modules/Assets/Concepts/MetadataConcept.h"
#include "Modules/Assets/Concepts/AssetConcept.h"
#include "Modules/Assets/Concepts/AssetFileConcepts.h"
#include "Modules/FileSystem/FileSystem.h"
#include "Kargono/Projects/Project.h"
#include "Modules/Core/MetaProgramming/MetaProgrammingTools.h"

#include <filesystem>
#include <vector>
#include <utility>
#include <cstdint>
#include <array>
#include <type_traits>

namespace Kargono::Assets
{
	template<typename t_AssetType>
	struct Metadata
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		Metadata() = default;
		~Metadata() = default;
	public:
		//==============================
		// Metadata State
		//==============================
		bool IsValid()
		{
			return m_Handle.IsValid() &&
				m_TypeIdentifier != k_InvalidAssetIdentifier;
		}
	public:
		//=============================
		// Generate File Paths
		//=============================
		std::filesystem::path GetAssetFullFilePath() requires HasFileLocation<t_AssetType>
		{
			Projects::ProjectPaths& projectPaths{ Projects::ProjectService::GetActiveContext().GetProjectPaths() };

			return projectPaths.GetAssetDirectory() / GetAssetRelativeFilePath();
		}
		std::filesystem::path GetAssetRelativeFilePath() requires HasFileLocation<t_AssetType>
		{
			KG_ASSERT(!m_Name.IsEmpty());
			KG_ASSERT(!m_FileDirectory.empty());
			return m_FileDirectory / (m_Name.String() + t_AssetType::GetFileExtension().String());
		}
		std::filesystem::path GetAssetFullIntermediatePath(std::string_view extension) requires HasIntermediates<t_AssetType>
		{
			Projects::ProjectPaths& projectPaths{ Projects::ProjectService::GetActiveContext().GetProjectPaths() };

			return projectPaths.GetIntermediateDirectory().string() /
				GetAssetRelativeIntermediatePath(extension);
		}
		std::filesystem::path GetAssetRelativeIntermediatePath(std::string_view extension) requires HasIntermediates<t_AssetType>
		{
			KG_ASSERT(!m_Name.IsEmpty());
			KG_ASSERT(ValidateExtension(extension));

			std::stringstream pathWithoutExtension;
			pathWithoutExtension << GetModuleName<t_AssetType>() << "/" <<
				GetTypeName<t_AssetType>() << "/" << m_Name;
			return { std::filesystem::path(pathWithoutExtension.str()).replace_extension(extension)};
		}
		std::filesystem::path GetAssetFullHiddenFolder() requires HasIntermediates<t_AssetType>
		{
			Projects::ProjectPaths& projectPaths{ Projects::ProjectService::GetActiveContext().GetProjectPaths() };

			return projectPaths.GetIntermediateDirectory().string() /
				GetAssetRelativeHiddenFolder();
		}
		std::filesystem::path GetAssetRelativeHiddenFolder() requires HasIntermediates<t_AssetType>
		{
			std::stringstream folderPath;
			folderPath << GetModuleName<t_AssetType>() << "/" <<
				GetTypeName<t_AssetType>();
			return { folderPath.str() };
		}
	private:
		// Helper(s)
		bool ValidateExtension(std::string_view queryExtension) requires HasIntermediates<t_AssetType>
		{
			std::span<const FixedBufStr16> validExtensions{ t_AssetType::GetIntermediateExtensions() };
			for (const FixedBufStr16& extension : validExtensions)
			{
				if (extension.StringView() == queryExtension)
				{
					return true;
				}
			}
			return false;
		}
	public:
		//==============================
		// Getters/Setters
		//==============================
		auto* GetSpecificMetaData() requires HasMetadata<t_AssetType>
		{
			KG_ASSERT(m_SpecificMetaData);
			return static_cast<typename t_AssetType::Metadata*>(m_SpecificMetaData);
		}

		void SetSpecificMetaData(auto* newMetaData) requires HasMetadata<t_AssetType>
		{
			// Ensure provided type matches expected metadata type
			EnforceTypesMatch<typename t_AssetType::Metadata*, decltype(newMetaData)>();

			KG_ASSERT(newMetaData);
			m_SpecificMetaData = static_cast<void*>(newMetaData);
		}
	public:
		//==============================
		// Public Fields
		//==============================
		FixedBufStr16 m_Name{};
		std::filesystem::path m_FileDirectory{};
		AssetHandle m_Handle{ Assets::k_EmptyHandle };
		AssetIdentifier m_TypeIdentifier{ k_InvalidAssetIdentifier };
		Utility::SHA256Hash m_Hash{};
		bool m_IsHidden{ false };
	private:
		//==============================
		// Internal Field(s)
		//==============================
		void* m_SpecificMetaData{ nullptr };
	};
}
