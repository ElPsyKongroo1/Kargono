#pragma once

#include "Modules/Rendering/Shader.h"
#include "Kargono/Core/WrappedData.h"
#include "Modules/Scripting/ScriptingCommon.h"
#include "Kargono/Core/UUID.h"
#include "Modules/Core/Module.h"
#include "Kargono/Core/FixedBufferString.h"
#include "Modules/Assets/AssetsCommon.h"
#include "Modules/Assets/Concepts/MetadataConcept.h"
#include "Modules/Assets/Concepts/AssetConcept.h"
#include "Modules/Assets/Concepts/AssetFileConcepts.h"
#include "Modules/FileSystem/FileSystem.h"
#include "Kargono/Projects/Project.h"

#include <filesystem>
#include <vector>
#include <utility>
#include <cstdint>
#include <array>

namespace Kargono::Assets
{
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
			return m_Handle != Assets::k_EmptyHandle &&
				m_TypeIdentifier != k_InvalidAssetIdentifier;
		}
	public:
		//=============================
		// Generate File Paths
		//=============================
		template<AssetConcept t_AssetType> requires HasFileLocation<t_AssetType>
		std::filesystem::path GetAssetFullFilePath()
		{
			Projects::ProjectPaths& projectPaths{ Projects::ProjectService::GetActiveContext().GetProjectPaths() };

			return projectPaths.GetAssetDirectory() / GetAssetRelativeFilePath<t_AssetType>();
		}
		template<AssetConcept t_AssetType> requires HasFileLocation<t_AssetType>
		std::filesystem::path GetAssetRelativeFilePath()
		{
			KG_ASSERT(!m_Name.IsEmpty());
			KG_ASSERT(!m_FileDirectory.empty());
			return m_FileDirectory / m_Name + t_AssetType::GetFileExtension();
		}
		template<AssetConcept t_AssetType> requires HasIntermediates<t_AssetType>
		std::filesystem::path GetAssetFullIntermediatePath(std::string_view extension)
		{
			Projects::ProjectPaths& projectPaths{ Projects::ProjectService::GetActiveContext().GetProjectPaths() };

			return projectPaths.GetIntermediateDirectory().string() /
				GetAssetRelativeIntermediatePath<t_AssetType>(extension);
		}
		template<AssetConcept t_AssetType> requires HasIntermediates<t_AssetType>
		std::filesystem::path GetAssetRelativeIntermediatePath(std::string_view extension)
		{
			KG_ASSERT(!m_Name.IsEmpty());
			KG_ASSERT(ValidateExtension(extension));

			std::stringstream pathWithoutExtension;
			pathWithoutExtension << GetModuleName<t_AssetType>() << "/" <<
				GetTypeName<t_AssetType>() << "/" << m_Name;
			return { std::filesystem::path(pathWithoutExtension.str()).replace_extension(extension)};
		}
		template<AssetConcept t_AssetType> requires HasIntermediates<t_AssetType>
		std::filesystem::path GetAssetFullHiddenFolder()
		{
			Projects::ProjectPaths& projectPaths{ Projects::ProjectService::GetActiveContext().GetProjectPaths() };

			return projectPaths.GetIntermediateDirectory().string() /
				GetAssetRelativeHiddenFolder<t_AssetType>();
		}
		template<AssetConcept t_AssetType> requires HasIntermediates<t_AssetType>
		std::filesystem::path GetAssetRelativeHiddenFolder()
		{
			std::stringstream folderPath;
			folderPath << GetModuleName<t_AssetType>() << "/" <<
				GetTypeName<t_AssetType>();
			return { folderPath.str() };
		}
	private:
		// Helper(s)
		template<AssetConcept t_AssetType> requires HasIntermediates<t_AssetType>
		bool ValidateExtension(std::string_view queryExtension)
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
		template <MetadataConcept t_MetadataType>
		t_MetadataType* GetSpecificMetaData()
		{
			KG_ASSERT(m_SpecificMetaData);
			return static_cast<t_MetadataType*>(m_SpecificMetaData);
		}

		template <MetadataConcept t_MetadataType>
		void SetSpecificMetaData(t_MetadataType* newMetaData)
		{
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

	struct ShaderMetaData
	{
		Rendering::ShaderSpecification m_ShaderSpec{};
		Rendering::UniformBufferList m_UniformList{};
		Rendering::InputBufferLayout m_InputLayout{};
	};

	struct ScriptMetaData
	{
		Scripting::ScriptType m_ScriptType {Scripting::ScriptType::None };
		FixedBufStr16 m_SectionLabel{};
		WrappedFuncType m_FunctionType{};
		Scripting::ExplicitFuncType m_ExplicitFuncType{};
	};
}
