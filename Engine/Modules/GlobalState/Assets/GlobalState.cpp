#include "kgpch.h"

#include "Modules/GlobalState/Assets/GlobalState.h"

namespace Kargono::GlobalState
{

	void GlobalState::CreateAssetFromName(Assets::Metadata& metadata)
	{
		// Create default global state
		GlobalState defaultGlobalState{};
		defaultGlobalState.m_Name = metadata.m_Name;

		// Write default global state to asset file
		Assets::SerializeAssetContext assetContext{};
		assetContext.m_AssetMetadata = &metadata;
		defaultGlobalState.Serialize((void*)&assetContext);
	}
	void GlobalState::Serialize(void* context)
	{
		// Get context
		Assets::SerializeAssetContext* serializeContext = (Assets::SerializeAssetContext*)context;
		KG_ASSERT(serializeContext);
		Assets::Metadata* metadata{ serializeContext->m_AssetMetadata };
		KG_ASSERT(metadata);

		// Get asset path
		const std::filesystem::path& assetPath{ metadata->GetAssetFullFilePath<GlobalState>() };

		YAML::Emitter out;
		out << YAML::BeginMap; // Start of File Map
		// Save name
		out << YAML::Key << "Name" << YAML::Value << m_Name;

		// Save size information
		out << YAML::Key << "BufferSize" << YAML::Value << m_DataBuffer.m_Size;

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
		for (size_t location : m_DataLocations)
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

		out << YAML::Key << "Data" << YAML::BeginMap; // Start data map
		size_t iteration{ 0 };
		for (WrappedVarType type : m_DataTypes)
		{
			Utility::SerializeWrappedVarType
			(
				out,
				type,
				m_DataNames.at(iteration),
				m_DataBuffer.m_Data + m_DataLocations.at(iteration)
			);
			iteration++;
		}
		out << YAML::EndMap; // End data map

		out << YAML::EndMap; // Start of File Map

		std::ofstream fout(assetPath);
		fout << out.c_str();
		KG_INFO("Successfully Serialized GlobalState at {}", assetPath);
	}
	void GlobalState::Deserialize(void* context)
	{
		KG_ASSERT(context, "Context cannot be null");

		// Get asset context
		Assets::DeserializeAssetContext* assetContext = (Assets::DeserializeAssetContext*)context;
		KG_ASSERT(assetContext, "Context cannot be null");
		Assets::Metadata* metadata{ assetContext->m_AssetMetadata };
		KG_ASSERT(metadata, "Metadata cannot be null");

		// Get context fields
		const std::filesystem::path& assetPath{ metadata->GetAssetFullFilePath<GlobalState>() };

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
		m_Name = data["Name"].as<std::string>();

		// Get component size information
		m_DataBuffer.m_Size = data["BufferSize"].as<uint64_t>();

		size_t dataSize{ 0 };
		// Get Data Types
		YAML::Node dataTypesNode = data["DataTypes"];
		if (dataTypesNode)
		{
			std::vector<WrappedVarType>& newTypesList = m_DataTypes;
			for (const YAML::Node& dataTypeNode : dataTypesNode)
			{
				WrappedVarType type = newTypesList.emplace_back(Utility::StringToWrappedVarType(dataTypeNode.as<std::string>()));
				dataSize += Utility::WrappedVarTypeToDataSizeBytes(type);
			}
		}

		// Get data locations
		YAML::Node dataLocationsNode = data["DataLocations"];
		if (dataLocationsNode)
		{
			std::vector<size_t>& newLocationsList = m_DataLocations;
			for (const YAML::Node& dataLocationNode : dataLocationsNode)
			{
				newLocationsList.push_back(dataLocationNode.as<size_t>());
			}
		}

		// Get data names
		YAML::Node dataNamesNode = data["DataNames"];
		if (dataNamesNode)
		{
			std::vector<FixedBufStr32>& newNamesList = m_DataNames;
			for (const YAML::Node& dataNameNode : dataNamesNode)
			{
				newNamesList.push_back(dataNameNode.as<std::string>().c_str());
			}
		}

		// Allocate buffer for new data
		if (dataSize > 0)
		{
			m_DataBuffer.Allocate(dataSize);
		}

		// Get data
		YAML::Node dataNode = data["Data"];
		if (dataNamesNode)
		{
			size_t iteration{ 0 };
			for (FixedBufStr32& name : m_DataNames)
			{
				Utility::DeserializeWrappedVarType
				(
					dataNode,
					m_DataTypes.at(iteration),
					name.CString(),
					m_DataBuffer.m_Data + m_DataLocations.at(iteration)
				);
				iteration++;
			}
		}
	}

	bool GlobalState::DoesContainName(const char* queryName)
	{
		// Check every field name. Ensure each name does not match the query name
		for (const FixedBufStr32& dataName : m_DataNames)
		{
			// Compare each c-string
			if (strcmp(dataName.CString(), queryName) == 0) // TODO: Careful of the strcmp
			{
				return true;
			}
		}
		return false;
	}

	uint8_t* GlobalState::GetFieldRaw(const char* queryName)
	{
		KG_ASSERT(queryName);

		// Find the index for the field whose name matches the queryName
		size_t iteration{ 0 };
		for (const FixedBufStr32& dataName : m_DataNames)
		{
			// Compare each c-string
			if (strcmp(dataName.CString(), queryName) == 0)
			{
				break;
			}
			iteration++;
		}

		// Ensure the index is within the bounds of the fields vector
		KG_ASSERT(iteration < m_DataLocations.size());

		// Get the data pointer for the field
		uint8_t* fieldPtr = m_DataBuffer.m_Data + m_DataLocations.at(iteration);
		KG_ASSERT(fieldPtr);

		return fieldPtr;
	}

	uint8_t* GlobalState::GetFieldRaw(size_t fieldIndex)
	{
		KG_ASSERT(fieldIndex < m_DataLocations.size());

		// Get the data pointer for the field
		uint8_t* fieldPtr = m_DataBuffer.m_Data + m_DataLocations.at(fieldIndex);
		KG_ASSERT(fieldPtr);

		return fieldPtr;
	}

	size_t GlobalState::CalculateBufferSize()
	{
		size_t returnValue{ 0 };

		// Sum the sizes of each data type
		for (WrappedVarType type : m_DataTypes)
		{
			returnValue += Utility::WrappedVarTypeToDataSizeBytes(type);
		}
		return returnValue;
	}

	bool GlobalState::AddField(const char* newName, WrappedVarType newType)
	{
		// Ensure a duplicate field name does not exist
		if (DoesContainName(newName))
		{
			return false;
		}

		// Store the original buffer size for later calculations
		size_t originalBufferSize = m_DataBuffer.m_Size;

		// Add the field to the back of the field list
		m_DataNames.emplace_back(newName);
		m_DataTypes.emplace_back(newType);
		m_DataLocations.emplace_back(originalBufferSize);

		// Calculate new buffer size
		size_t newBufferSize = CalculateBufferSize();

		// Create/allocate new buffer
		Buffer newBuffer;
		newBuffer.Allocate(newBufferSize);
		newBuffer.SetDataToByte(0);

		// Transfer original buffer data to new buffer
		if (m_DataBuffer.m_Size > 0)
		{
			KG_ASSERT(newBuffer.m_Size >= m_DataBuffer.m_Size);
			memcpy(newBuffer.m_Data, m_DataBuffer.m_Data, m_DataBuffer.m_Size);
		}

		// Get the data pointer for the new field
		uint8_t* newFieldPtr = newBuffer.m_Data + originalBufferSize;

		// Add default values for the new field
		Utility::InitializeDataForWrappedVarBuffer(newType, newFieldPtr);

		// Store the buffer and clear original buffer
		m_DataBuffer.Release();
		m_DataBuffer = newBuffer;

		return true;
	}

	GlobalState::~GlobalState()
	{
		m_DataBuffer.Release();
	}
}

