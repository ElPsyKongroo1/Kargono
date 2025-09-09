#pragma once
#include "Kargono/Core/FixedBufferString.h"
#include "Kargono/Core/WrappedData.h"
#include "Kargono/Core/Buffer.h"

#include <cstdint>
#include <vector>
#include <string>

namespace Kargono::ProjectData
{
	struct GlobalState
	{
	public:
		//=========================
		// Constructors/Destructors
		//=========================
		GlobalState() = default;
		~GlobalState();
	public:
		//=========================
		// Query State
		//=========================
		template <typename t_FieldType>
		t_FieldType* GetField(const char* queryName)
		{
			return (t_FieldType*)GetFieldRaw(queryName);
		}
		uint8_t* GetFieldRaw(const char* queryName);

		template <typename t_FieldType>
		t_FieldType* GetField(size_t fieldIndex)
		{
			return (t_FieldType*)GetFieldRaw(fieldIndex);
		}
		uint8_t* GetFieldRaw(size_t fieldIndex);
		bool DoesContainName(const char* queryName);
	public:
		//=========================
		// Modify State
		//=========================
		bool AddField(const char* newName, WrappedVarType newType);

		template <typename t_FieldType>
		bool SetField(const char* queryName, WrappedVarType newType, t_FieldType* newValue)
		{
			KG_ASSERT(newType != WrappedVarType::None && newType != WrappedVarType::Void);
			KG_ASSERT(newValue);

			uint8_t* fieldPtr = GetFieldRaw(queryName);
			KG_ASSERT(fieldPtr);

			// Get the data pointer for the field
			Utility::TransferDataForWrappedVarBuffer(newType, newValue, fieldPtr);
			return true;
		}

		template <typename t_FieldType>
		bool SetField(size_t fieldIndex, WrappedVarType newType, t_FieldType* newValue)
		{
			KG_ASSERT(newType != WrappedVarType::None && newType != WrappedVarType::Void);
			KG_ASSERT(newValue);

			uint8_t* fieldPtr = GetFieldRaw(fieldIndex);
			KG_ASSERT(fieldPtr);

			// Transfer the data
			Utility::TransferDataForWrappedVarBuffer(newType, newValue, fieldPtr);
			return true;
		}
	private:
		// Helper(s)
		size_t CalculateBufferSize();
	public:
		//=========================
		// Public Fields
		//=========================
		FixedString32 m_Name;
		std::vector<WrappedVarType> m_DataTypes;
		std::vector<size_t> m_DataLocations;
		std::vector<FixedString32> m_DataNames;
		Buffer m_DataBuffer;
	};
}
