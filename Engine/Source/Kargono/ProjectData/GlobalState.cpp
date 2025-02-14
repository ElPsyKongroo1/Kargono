#include "kgpch.h"
#include "GlobalState.h"

bool Kargono::ProjectData::GlobalStateService::DoesGlobalStateContainName(GlobalState* globalState, const char* queryName)
{
	KG_ASSERT(globalState);

	// Check every field name. Ensure each name does not match the query name
	for (const FixedString32& dataName : globalState->m_DataNames)
	{
		// Compare each c-string
		if (strcmp(dataName.CString(), queryName) == 0)
		{
			return true;
		}
	}
	return false;
}

size_t Kargono::ProjectData::GlobalStateService::CalculateBufferSize(GlobalState* globalState)
{
	KG_ASSERT(globalState);

	size_t returnValue{ 0 };

	// Sum the sizes of each data type
	for (WrappedVarType type : globalState->m_DataTypes)
	{
		returnValue += Utility::WrappedVarTypeToDataSizeBytes(type);
	}
	return returnValue;
}

bool Kargono::ProjectData::GlobalStateService::AddFieldToGlobalState(GlobalState* globalState, const char* newName, WrappedVarType newType)
{
	KG_ASSERT(globalState);

	// Ensure a duplicate field name does not exist
	if (DoesGlobalStateContainName(globalState, newName))
	{
		return false;
	}

	// Store the original buffer size for later calculations
	size_t originalBufferSize = globalState->m_DataBuffer.Size;

	// Add the field to the back of the field list
	globalState->m_DataNames.emplace_back(newName);
	globalState->m_DataTypes.emplace_back(newType);
	globalState->m_DataLocations.emplace_back(originalBufferSize);
	
	// Calculate new buffer size
	size_t newBufferSize = CalculateBufferSize(globalState);

	// Create/allocate new buffer
	Buffer newBuffer;
	newBuffer.Allocate(newBufferSize);
	newBuffer.SetDataToByte(0);

	// Transfer original buffer data to new buffer
	if (globalState->m_DataBuffer.Size > 0)
	{
		KG_ASSERT(newBuffer.Size >= globalState->m_DataBuffer.Size);
		memcpy(newBuffer.Data, globalState->m_DataBuffer.Data, globalState->m_DataBuffer.Size);
	}

	// Get the data pointer for the new field
	uint8_t* newFieldPtr = newBuffer.Data + originalBufferSize;

	// Add default values for the new field
	Utility::InitializeDataForWrappedVarBuffer(newType, newFieldPtr);

	// Store the buffer and clear original buffer
	globalState->m_DataBuffer.Release();
	globalState->m_DataBuffer = newBuffer;

	return true;
}
