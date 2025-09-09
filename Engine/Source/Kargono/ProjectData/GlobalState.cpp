#include "kgpch.h"
#include "GlobalState.h"

namespace Kargono::ProjectData
{

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
		for (const FixedString32& dataName : m_DataNames)
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
		uint8_t* fieldPtr = m_DataBuffer.Data + m_DataLocations.at(iteration);
		KG_ASSERT(fieldPtr);

		return fieldPtr;
	}

	uint8_t* GlobalState::GetFieldRaw(size_t fieldIndex)
	{
		KG_ASSERT(fieldIndex < m_DataLocations.size());

		// Get the data pointer for the field
		uint8_t* fieldPtr = m_DataBuffer.Data + m_DataLocations.at(fieldIndex);
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
		size_t originalBufferSize = m_DataBuffer.Size;

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
		if (m_DataBuffer.Size > 0)
		{
			KG_ASSERT(newBuffer.Size >= m_DataBuffer.Size);
			memcpy(newBuffer.Data, m_DataBuffer.Data, m_DataBuffer.Size);
		}

		// Get the data pointer for the new field
		uint8_t* newFieldPtr = newBuffer.Data + originalBufferSize;

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

