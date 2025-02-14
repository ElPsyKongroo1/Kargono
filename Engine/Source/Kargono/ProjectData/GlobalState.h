#pragma once
#include "Kargono/Core/FixedString.h"
#include "Kargono/Core/WrappedData.h"
#include "Kargono/Core/Buffer.h"

#include <cstdint>
#include <vector>
#include <string>

namespace Kargono::ProjectData
{
	struct GlobalState
	{
		FixedString32 m_Name;
		std::vector<WrappedVarType> m_DataTypes;
		std::vector<size_t> m_DataLocations;
		std::vector<FixedString32> m_DataNames;
		Buffer m_DataBuffer;

	public:
		~GlobalState()
		{
			m_DataBuffer.Release();
		}
	};

	class GlobalStateService
	{
	public:
		//=========================
		// Query Global State
		//=========================
		static bool DoesGlobalStateContainName(GlobalState* globalState, const char* queryName);
		static size_t CalculateBufferSize(GlobalState* globalState);
		
		template <typename FieldType>
		static FieldType* GetGlobalStateField(GlobalState* globalState, const char* queryName)
		{
			KG_ASSERT(globalState);
			KG_ASSERT(queryName);

			// Find the index for the field whose name matches the queryName
			size_t iteration{ 0 };
			for (const FixedString32& dataName : globalState->m_DataNames)
			{
				// Compare each c-string
				if (strcmp(dataName.CString(), queryName) == 0)
				{
					break;
				}
				iteration++;
			}

			// Ensure the index is within the bounds of the fields vector
			KG_ASSERT(iteration < globalState->m_DataLocations.size());

			// Get the data pointer for the field
			uint8_t* fieldPtr = globalState->m_DataBuffer.Data + globalState->m_DataLocations.at(iteration);
			KG_ASSERT(fieldPtr);
			
			return (FieldType*)fieldPtr;
		}

		template <typename FieldType>
		static FieldType* GetGlobalStateField(GlobalState* globalState, size_t fieldIndex)
		{
			KG_ASSERT(globalState);
			KG_ASSERT(fieldIndex < globalState->m_DataLocations.size());

			// Get the data pointer for the field
			uint8_t* fieldPtr = globalState->m_DataBuffer.Data + globalState->m_DataLocations.at(fieldIndex);
			KG_ASSERT(fieldPtr);

			return (FieldType*)fieldPtr;
		}
		//=========================
		// Modify Global State
		//=========================
		static bool AddFieldToGlobalState(GlobalState* globalState, const char* newName, WrappedVarType newType);

		template <typename FieldType>
		static bool SetGlobalStateField(GlobalState* globalState, const char* queryName, WrappedVarType newType, FieldType* newValue)
		{
			KG_ASSERT(globalState);
			KG_ASSERT(queryName);
			KG_ASSERT(newType != WrappedVarType::None && newType != WrappedVarType::Void);
			KG_ASSERT(newValue);

			// Find the index for the field whose name matches the queryName
			size_t iteration{ 0 };
			for (const FixedString32& dataName : globalState->m_DataNames)
			{
				// Compare each c-string
				if (strcmp(dataName.CString(), queryName) == 0)
				{
					break;
				}
				iteration++;
			}

			// Ensure the index is within the bounds of the fields vector
			KG_ASSERT(iteration < globalState->m_DataLocations.size());

			// Get the data pointer for the field
			uint8_t* fieldPtr = globalState->m_DataBuffer.Data + globalState->m_DataLocations.at(iteration);
			KG_ASSERT(fieldPtr);

			Utility::TransferDataForWrappedVarBuffer(newType, newValue, fieldPtr);
			return true;
		}

		template <typename FieldType>
		static bool SetGlobalStateField(GlobalState* globalState, size_t fieldIndex, WrappedVarType newType, FieldType* newValue)
		{
			KG_ASSERT(globalState);
			KG_ASSERT(newType != WrappedVarType::None && newType != WrappedVarType::Void);
			KG_ASSERT(newValue);

			// Ensure the index is within the bounds of the fields vector
			KG_ASSERT(fieldIndex < globalState->m_DataLocations.size());

			// Get the data pointer for the field
			uint8_t* fieldPtr = globalState->m_DataBuffer.Data + globalState->m_DataLocations.at(fieldIndex);
			KG_ASSERT(fieldPtr);

			// Transfer the data
			Utility::TransferDataForWrappedVarBuffer(newType, newValue, fieldPtr);
			return true;
		}
	};
}
