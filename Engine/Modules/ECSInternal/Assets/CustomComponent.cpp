#include "kgpch.h"

#include "Modules/ECSInternal/Assets/CustomComponent.h"
#include "Kargono/Projects/Project.h"

namespace Kargono::ECSInternal
{
	void CustomComponentMetaData::Serialize(void* context)
	{
		// Get asset context
		KG_ASSERT(context, "Context cannot be null");
		Assets::SerializeMetaDataContext& metadataContext = *(Assets::SerializeMetaDataContext*)context;
		// Get context fields
		YAML::Emitter& emitter = *metadataContext.m_Serializer;
		emitter << YAML::Key << "Name" << YAML::Value << m_Name;
	}

	void CustomComponentMetaData::Deserialize(void* context)
	{
		// Get asset context
		KG_ASSERT(context, "Context cannot be null");
		Assets::DeserializeMetaDataContext& assetContext = *(Assets::DeserializeMetaDataContext*)context;
		// Get context fields
		YAML::Node& metadataNode = *assetContext.m_Node;
		m_Name = metadataNode["Name"].as<std::string>();
	}

	bool CustomComponent::AddField(WrappedVarType fieldType, const char* fieldName)
	{
		KG_ASSERT(m_DataNames.size() == m_DataOffsets.size() &&
			m_DataOffsets.size() == m_DataTypes.size(), "Project component is malformed");

		// Ensure fieldType is valid
		if (fieldType == WrappedVarType::None || fieldType == WrappedVarType::Void)
		{
			KG_WARN("Could not add field to custom component. Cannot instantiate a custom component field with \"None\" type");
			return false;
		}

		// Ensure duplicate name is not provided
		bool foundDuplicateName{ false };
		for (const char* name : m_DataNames)
		{
			if (fieldName == name)
			{
				foundDuplicateName = true;
				break;
			}
		}
		if (foundDuplicateName)
		{
			KG_WARN("Could not add field to custom component. Duplicate field name provided");
			return false;
		}

		// Store Field Name and Field Type
		m_DataNames.push_back(fieldName);
		m_DataTypes.push_back(fieldType);

		// Recalculate alignment
		size_t alignment = RevalidateAlignment();

		// Recalculate data locations
		RecalculateDataLocations();

		return true;
	}
	void CustomComponent::DeleteField(size_t fieldIndex)
	{
		KG_ERROR("Make sure to take into account alignment");

		KG_ASSERT(fieldIndex < m_DataNames.size());

		KG_ASSERT(m_DataNames.size() == m_DataOffsets.size() &&
			m_DataOffsets.size() == m_DataTypes.size(), "Project component is malformed");

		// Delete index from all vectors
		m_DataNames.erase(m_DataNames.begin() + fieldIndex);
		m_DataTypes.erase(m_DataTypes.begin() + fieldIndex);
		m_DataOffsets.erase(m_DataOffsets.begin() + fieldIndex);

		// Revalidate alignment and data locations
		RevalidateAlignment();
		RecalculateDataLocations();
	}
	bool CustomComponent::EditField(size_t fieldIndex, const char* fieldName, WrappedVarType fieldType)
	{
		KG_ERROR("Make sure to take into account alignment");
		KG_ASSERT(m_DataNames.size() == m_DataOffsets.size() &&
			m_DataOffsets.size() == m_DataTypes.size(), "Project component is malformed");

		// Ensure fieldType is valid
		if (fieldType == WrappedVarType::None || fieldType == WrappedVarType::Void)
		{
			KG_WARN("Could not edit custom component. Cannot instantiate a custom component field with \"None\" type");
			return false;
		}

		// Edit Field
		m_DataNames.at(fieldIndex) = fieldName;
		m_DataTypes.at(fieldIndex) = fieldType;

		// Revalidate alignment and data locations
		RevalidateAlignment();
		RecalculateDataLocations();

		return true;
	}
	void CustomComponent::RecalculateDataLocations()
	{
		m_DataOffsets.clear();
		m_DataOffsets.reserve(m_DataTypes.size());
		size_t currentMaximumLocation{ 0 };
		for (WrappedVarType type : m_DataTypes)
		{
			size_t remainder = currentMaximumLocation % m_ComponentAlignment;
			if (remainder != 0)
			{
				currentMaximumLocation += m_ComponentAlignment - remainder;
			}
			m_DataOffsets.push_back(currentMaximumLocation);
			currentMaximumLocation += Utility::WrappedVarTypeToDataSizeBytes(type);
		}

		m_ComponentSize = currentMaximumLocation;
	}
	ECSInternal::ComponentIdentifier CustomComponent::RevalidateIdentifier()
	{
		KG_ASSERT(m_Name.StringLength() > 0);
		std::string identifierStr{ "CustomComponent" "::" + m_Name };
		ECSInternal::ComponentIdentifier identifier =
			Utility::FileSystem::CRCFromString(identifierStr.c_str());

		m_Identifier = identifier;

		return identifier;
	}
	ECSInternal::ComponentMetadata CustomComponent::GenerateMetadata(Assets::AssetHandle compHandle) const
	{
		KG_ASSERT(compHandle != Assets::k_EmptyHandle);

		ECSInternal::ComponentMetadata metadata{};
		metadata.m_ComponentSize = m_ComponentSize;
		metadata.m_ComponentAlignment = m_ComponentAlignment;
		metadata.m_CompFunctors.m_Copy = CustomComponentCopyTo;
		metadata.m_CustomComponentHandle = compHandle;
		return metadata;
	}
	size_t CustomComponent::RevalidateAlignment()
	{
		size_t maxAlignment{ 1 };
		for (WrappedVarType type : m_DataTypes)
		{
			size_t typeAlignment{ Utility::WrappedVarTypeAlignment(type) };
			if (typeAlignment > maxAlignment)
			{
				maxAlignment = typeAlignment;
			}
		}
		m_ComponentAlignment = maxAlignment;
		return maxAlignment;
	}

	void CustomComponent::Serialize(void* context)
	{
		// Get asset context
		Assets::SerializeAssetContext* assetContext = (Assets::SerializeAssetContext*)context;
		KG_ASSERT(assetContext, "Context cannot be null");
		Assets::Metadata* metadata { assetContext->m_AssetMetadata };
		KG_ASSERT(metadata, "Metadata cannot be null");

		// Get context fields
		const std::filesystem::path& assetPath{ metadata->GetAssetFullFilePath<CustomComponent>() };

		YAML::Emitter out;
		out << YAML::BeginMap; // Start of File Map
		// Save name/identifier
		out << YAML::Key << "Name" << YAML::Value << m_Name;
		out << YAML::Key << "Identifier" << YAML::Value << m_Identifier;

		// Save size information
		out << YAML::Key << "ComponentSize" << YAML::Value << m_ComponentSize;
		out << YAML::Key << "ComponentAlignment" << YAML::Value << m_ComponentAlignment;

		// Save data types
		out << YAML::Key << "DataTypes" << YAML::Value;
		out << YAML::BeginSeq; // Start of Data Type Sequence
		for (WrappedVarType type : m_DataTypes)
		{
			out << YAML::Value << Utility::WrappedVarTypeToString(type);
		}
		out << YAML::EndSeq; // End of Data Type Sequence

		// Save data locations
		out << YAML::Key << "DataLocations" << YAML::Value;
		out << YAML::BeginSeq; // Start of Data Locations Sequence
		for (uint64_t location : m_DataOffsets)
		{
			out << YAML::Value << location;
		}
		out << YAML::EndSeq; // End of Data Locations Sequence

		// Save data names
		out << YAML::Key << "DataNames" << YAML::Value;
		out << YAML::BeginSeq; // Start of Data Names Sequence
		for (FixedBufStr32& name : m_DataNames)
		{
			out << YAML::Value << name.CString();
		}
		out << YAML::EndSeq; // End of Data Names Sequence

		out << YAML::EndMap; // Start of File Map

		std::ofstream fout(assetPath);
		fout << out.c_str();
	}

	void CustomComponent::Deserialize(void* context)
	{
		// Get context & metadata
		Assets::DeserializeAssetContext* deserializeContext = (Assets::DeserializeAssetContext*)context;
		KG_ASSERT(deserializeContext, "Context cannot be null");
		Assets::Metadata* metadata = deserializeContext->m_AssetMetadata;
		KG_ASSERT(metadata, "Metadata cannot be null");

		// Get file path
		const std::filesystem::path& assetPath{ metadata->GetAssetFullFilePath<CustomComponent>() };

		// Deserialize
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(assetPath.string());
		}
		catch (YAML::ParserException e)
		{
			KG_WARN("Failed to load .kgui file '{0}'\n     {1}", assetPath, e.what());
			return;
		}

		// Get name / identifier
		m_Name = data["Name"].as<std::string>();
		m_Identifier = data["Identifier"].as<ECSInternal::ComponentIdentifier>();

		// Get component size information
		m_ComponentSize = data["ComponentSize"].as<uint64_t>();
		m_ComponentAlignment = data["ComponentAlignment"].as<uint64_t>();

		// Get Data Types
		YAML::Node dataTypesNode = data["DataTypes"];
		if (dataTypesNode)
		{
			std::vector<WrappedVarType>& newTypesList = m_DataTypes;
			for (const YAML::Node& dataTypeNode : dataTypesNode)
			{
				newTypesList.push_back(Utility::StringToWrappedVarType(dataTypeNode.as<std::string>()));
			}
		}

		// Get data locations
		YAML::Node dataLocationsNode = data["DataLocations"];
		if (dataLocationsNode)
		{
			std::vector<uint64_t>& newLocationsList = m_DataOffsets;
			for (const YAML::Node& dataLocationNode : dataLocationsNode)
			{
				newLocationsList.push_back(dataLocationNode.as<uint64_t>());
			}
		}

		// Get data names
		YAML::Node dataNamesNode = data["DataNames"];
		if (dataNamesNode)
		{
			std::vector<FixedBufStr32>& newNamesList{ m_DataNames };
			for (const YAML::Node& dataNameNode : dataNamesNode)
			{
				newNamesList.push_back(dataNameNode.as<std::string>().c_str());
			}
		}
	}

	Ref<void> CustomComponent::SaveValidation(Assets::AssetReference<CustomComponent> newAssetRef, Assets::Metadata& metadata)
	{
		// Get new asset reference	
		KG_ASSERT(newAssetRef.IsValid() && !newAssetRef.IsEmpty(), "Attempt to save an invalid asset reference");
		CustomComponent* newAsset{ &newAssetRef.GetAsset() };

		// Get path to asset file
		const std::filesystem::path assetPath{ metadata.GetAssetFullFilePath<CustomComponent>() };

		// Deserialize context into current asset to ensure up-to-date information
		Assets::DeserializeAssetContext assetContext{&metadata};
		Deserialize((void*)&assetContext);

		// Create reallocation instructions which stores information for transferring data from old entity components to new entity components
		Ref<FieldReallocationInstructions> newReallocationInstructions = CreateRef<FieldReallocationInstructions>();

		// Store old types/locations and new types/locations
		newReallocationInstructions->m_OldDataTypes = m_DataTypes;
		newReallocationInstructions->m_OldDataLocations = m_DataOffsets;

		newReallocationInstructions->m_NewDataTypes = newAsset->m_DataTypes;
		newReallocationInstructions->m_NewDataLocations = newAsset->m_DataOffsets;

		newReallocationInstructions->m_NewDataSize = newAsset->m_ComponentSize;
		for (auto& [sceneHandle, asset] : Assets::AssetService::GetSceneRegistry())
		{
			newReallocationInstructions->m_OldScenes.push_back(Assets::AssetService::GetScene(sceneHandle));
			newReallocationInstructions->m_OldSceneHandles.push_back(sceneHandle);
		}

		// Fill field transfer directions, which maps field data from the old component layout to the new component layout
		for (size_t OuterIteration{ 0 }; OuterIteration < newAsset->m_DataNames.size(); OuterIteration++)
		{
			// Check if identical field name exists inside old buffer and ensure similar type. If true, store the location of data in the old buffer
			bool oldBufferContainsField = false;
			for (size_t InnerIteration{ 0 }; InnerIteration < m_DataNames.size(); InnerIteration++)
			{
				if (newAsset->m_DataNames.at(OuterIteration) == m_DataNames.at(InnerIteration) &&
					newAsset->m_DataTypes.at(OuterIteration) == m_DataTypes.at(InnerIteration))
				{
					oldBufferContainsField = true;
					newReallocationInstructions->m_FieldTransferDirections.push_back(InnerIteration);
					break;
				}
			}

			// If no suitable option is found to get data from, initialize new data (use new allocation index)
			if (!oldBufferContainsField)
			{
				newReallocationInstructions->m_FieldTransferDirections.push_back(k_NewAllocationIndex);
			}

		}

		return newReallocationInstructions;
	}

	void CustomComponent::CopyTo(void* src, void* dst)
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

	void CustomComponent::CreateAssetFromName(Assets::Metadata& metadata)
	{
		// Create new custom component
		CustomComponent tempComponent{};
		tempComponent.m_Name = metadata.m_Name;

		// Get identifier
		ECSInternal::ComponentIdentifier identifier = tempComponent.RevalidateIdentifier();
		tempComponent.m_Identifier = identifier;

		// Save into file
		Assets::SerializeAssetContext serializeContext{ &metadata };
		tempComponent.Serialize((void*)&serializeContext);

		// Load data into in-memory metadata object
		CustomComponentMetaData* specificMetadata = metadata.GetSpecificMetaData<CustomComponentMetaData>();
		KG_ASSERT(specificMetadata);
		specificMetadata->m_Name = metadata.m_Name;
	}

	void CustomComponent::DeleteValidation(Assets::Metadata& metadata)
	{
		// Handle deleting the custom component by removing entity data from all scenes
		for (auto& [sceneHandle, assetInfo] : Assets::AssetService::GetSceneRegistry())
		{
			// Get scene
			Ref<Scenes::Scene> currentScene = Assets::AssetService::GetScene(sceneHandle);

			bool sceneModified = Assets::AssetService::RemoveCustomComponentFromScene(currentScene, metadata.m_Handle);

			if (sceneModified)
			{
				// Save scene asset on-disk 
				Assets::AssetService::SaveScene(sceneHandle, currentScene);
			}
		}
	}
}
