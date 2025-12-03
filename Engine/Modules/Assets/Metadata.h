#pragma once

#include "Kargono/Core/WrappedData.h"
#include "Kargono/Core/FixedBufferString.h"
#include "Modules/Assets/AssetsCommon.h"
#include "Modules/Assets/Concepts/MetadataExtensionConcept.h"
#include "Modules/Assets/Concepts/AssetConcept.h"
#include "Modules/Assets/Concepts/AssetFileConcepts.h"
#include "Modules/FileSystem/FileSystem.h"
#include "Kargono/Projects/Project.h"
#include "Modules/Core/MetaProgramming/MetaProgrammingTools.h"
#include "Modules/Modules/InspectModuleType.h"
#include "Modules/Modules/Concepts/IsModuleType.h"

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
			return m_Handle.IsValid();
		}
	public:
		//=============================
		// Generate File Paths
		//=============================
		std::filesystem::path GetAssetFullFilePath() requires HasFileLocation<t_AssetType>
		{
			Projects::ProjectPaths& projectPaths{ Projects::ProjectService::GetActiveContext().GetProjectPaths() };

			if (m_IsHidden)
			{
				return projectPaths.GetIntermediateDirectory() / 
					GetAssetRelativeHiddenPath();
			}
			else
			{
				return projectPaths.GetAssetDirectory() / GetAssetRelativeFilePath();
			}
		}
		std::filesystem::path GetAssetRelativeFilePath() requires HasFileLocation<t_AssetType>
		{
			KG_ASSERT(!m_Name.IsEmpty());
			KG_ASSERT(!m_FileDirectory.empty());
			return m_FileDirectory / (m_Name.String() + t_AssetType::GetFileExtension().String());
		}
		std::filesystem::path GetAssetFullIntermediatePath(std::string_view extension = {}) requires HasIntermediates<t_AssetType>
		{
			Projects::ProjectPaths& projectPaths{ Projects::ProjectService::GetActiveContext().GetProjectPaths() };

			return projectPaths.GetIntermediateDirectory().string() /
				GetAssetRelativeIntermediatePath(extension);
		}
		std::filesystem::path GetAssetRelativeIntermediatePath(std::string_view extension) requires HasIntermediates<t_AssetType>
		{
			if (!extension.empty())
			{
				KG_ASSERT(ValidateExtension(extension));
			}

			return std::filesystem::path(Modules::GetModuleName<t_AssetType>()) / Modules::GetTypeName<t_AssetType>() / CreateFileName(extension);
		}
		std::filesystem::path GetAssetFullHiddenFolder() requires HasIntermediates<t_AssetType>
		{
			Projects::ProjectPaths& projectPaths{ Projects::ProjectService::GetActiveContext().GetProjectPaths() };

			return projectPaths.GetIntermediateDirectory().string() /
				GetAssetRelativeHiddenFolder();
		}
		std::filesystem::path GetAssetRelativeHiddenFolder()
		{
			return std::filesystem::path(Modules::GetModuleName<t_AssetType>()) / Modules::GetTypeName<t_AssetType>();
		}

		std::filesystem::path GetAssetRelativeHiddenPath() requires HasFileLocation<t_AssetType>
		{
			return GetAssetRelativeHiddenFolder() / CreateFileName(t_AssetType::GetFileExtension().String());
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

		std::filesystem::path CreateFileName(std::string_view fileExtension)
		{
			// Specify the file name based on whether or not name is empty
			std::filesystem::path fileName;
			if (m_Name.IsEmpty())
			{
				fileName = std::string(m_Handle);
			}
			else
			{
				fileName = m_Name.String() + "_" + std::string(m_Handle);
			}

			// Optionally add the extension
			if (!fileExtension.empty())
			{
				fileName += std::string(fileExtension);
			}

			return fileName;
		}
	public:
		//==============================
		// Getters/Setters
		//==============================
		MetadataExtension_t<t_AssetType>& GetMetadataExtension() requires HasMetadataExtension<t_AssetType>
		{
			return m_MetadataExtension;
		}
		void SetMetadataExtension(MetadataExtension_t<t_AssetType>& newMetaData) requires HasMetadataExtension<t_AssetType>
		{
			m_MetadataExtension = newMetaData;
		}
	public:
		//==============================
		// Public Fields
		//==============================
		FixedBufStr32 m_Name{};
		std::filesystem::path m_FileDirectory{};
		AssetHandle m_Handle{ Assets::k_EmptyHandle };
		Utility::SHA256Hash m_Hash{};
		bool m_IsHidden{ false };
	private:
		//==============================
		// Internal Field(s)
		//==============================
		MetadataExtension_t<t_AssetType> m_MetadataExtension{};
	};
}
