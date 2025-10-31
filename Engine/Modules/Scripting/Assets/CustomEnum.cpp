#include "kgpch.h"

#include "Modules/Scripting/Assets/CustomEnum.h"

namespace Kargono::Scripting
{
	void CustomEnum::Serialize(void* context)
	{
		// Get asset context
		Assets::SerializeAssetContext* assetContext = (Assets::SerializeAssetContext*)context;
		KG_ASSERT(assetContext, "Context cannot be null");
		Assets::Metadata* metadata{ assetContext->m_AssetMetadata };
		KG_ASSERT(metadata, "Metadata cannot be null");

		// Get asset path
		const std::filesystem::path& assetPath{ metadata->GetAssetFullFilePath<CustomEnum>() };

		YAML::Emitter out;
		out << YAML::BeginMap; // Start of file map
		// Save name
		out << YAML::Key << "Name" << YAML::Value << m_EnumName;

		// Save data types
		out << YAML::Key << "Identifiers" << YAML::Value;
		out << YAML::BeginSeq; // Start of enum data sequence
		for (FixedBufStr32 enumerationName : m_EnumIdentifiers)
		{
			out << YAML::Value << enumerationName;
		}
		out << YAML::EndSeq; // End of enum data sequence

		out << YAML::EndMap; // End of file map

		std::ofstream fout(assetPath);
		fout << out.c_str();
	}

	void CustomEnum::Deserialize(void* context)
	{
		KG_ASSERT(context, "Context cannot be null");

		// Get asset context
		Assets::DeserializeAssetContext& assetContext = *(Assets::DeserializeAssetContext*)context;
		KG_ASSERT(assetContext.m_AssetMetadata, "Metadata cannot be null");
		Assets::Metadata* metadata{ assetContext.m_AssetMetadata };
		KG_ASSERT(metadata, "Metadata cannot be null");

		// Get asset path
		const std::filesystem::path& assetPath{ metadata->GetAssetFullFilePath<CustomEnum>() };

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

		// Get name
		m_EnumName = data["Name"].as<std::string>();

		// Get Data Types
		YAML::Node enumDataNode = data["Identifiers"];
		if (enumDataNode)
		{
			std::vector<FixedBufStr32>& newTypesList = m_EnumIdentifiers;
			for (const YAML::Node& enumerationNameNode : enumDataNode)
			{
				newTypesList.push_back(enumerationNameNode.as<std::string>().c_str());
			}
		}
	}

	void CustomEnum::CreateAssetFromName(Assets::Metadata& metadata)
	{
		// Create new custom component
		CustomEnum tempEnum{};
		tempEnum.m_EnumName = metadata.m_Name;

		// Save into File
		Assets::SerializeAssetContext context{ &metadata };
		tempEnum.Serialize((void*)&context);
	}

	bool CustomEnum::DoesContainIdentifier(const char* queryName)
	{
		KG_ASSERT(queryName);

		for (const FixedBufStr32& currentIdentifier : m_EnumIdentifiers)
		{
			if (strcmp(currentIdentifier.CString(), queryName) == 0) // TODO: Fix this
			{
				return true;
			}
		}
		return false;
	}
	bool CustomEnum::RemoveIdentifier(const char* queryName)
	{
		constexpr size_t k_InvalidIdentifierIndex{ std::numeric_limits<size_t>().max() };

		KG_ASSERT(queryName);
		size_t indexToDelete{ k_InvalidIdentifierIndex };
		size_t iteration{ 0 };

		// Search through identifiers for matching name
		for (const FixedBufStr32& currentIdentifier : m_EnumIdentifiers)
		{
			if (strcmp(currentIdentifier.CString(), queryName) == 0) // TODO: Fix this
			{
				indexToDelete = iteration;
				break;
			}
			iteration++;
		}

		// Check if no index was found
		if (indexToDelete == k_InvalidIdentifierIndex)
		{
			KG_WARN("Failed to remove identifier from the indicated enum");
			return false;
		}

		// Ensure found index is not out of bounds
		KG_ASSERT(indexToDelete < m_EnumIdentifiers.size());

		// Remove the identifier
		m_EnumIdentifiers.erase(m_EnumIdentifiers.begin() + indexToDelete);

		return true;
	}
	bool CustomEnum::RemoveIdentifier(size_t indexToDelete)
	{
		constexpr size_t k_InvalidIdentifierIndex{ std::numeric_limits<size_t>().max() };

		// Ensure provided index is valid
		KG_ASSERT(indexToDelete != k_InvalidIdentifierIndex);
		KG_ASSERT(indexToDelete < m_EnumIdentifiers.size());


		// Remove the identifier
		m_EnumIdentifiers.erase(m_EnumIdentifiers.begin() + indexToDelete);

		return true;
	}
	bool CustomEnum::RenameIdentifier(size_t indexToModify, const char* newName)
	{
		constexpr size_t k_InvalidIdentifierIndex{ std::numeric_limits<size_t>().max() };

		// Ensure the argument values are valid
		KG_ASSERT(newName);
		KG_ASSERT(indexToModify != k_InvalidIdentifierIndex);
		KG_ASSERT(indexToModify < m_EnumIdentifiers.size());
		size_t iteration{ 0 };

		// Search through identifiers and ensure no duplicate name is found
		for (const FixedBufStr32& currentIdentifier : m_EnumIdentifiers)
		{
			if (strcmp(currentIdentifier.CString(), newName) == 0)
			{
				KG_WARN("Failed to rename an identifier. Duplicate name found!");
				return false;
			}
			iteration++;
		}

		// Get the indicated identifier and modify its name
		FixedBufStr32& indicatedIdentifier = m_EnumIdentifiers.at(indexToModify);
		indicatedIdentifier = newName;

		return true;
	}
}
