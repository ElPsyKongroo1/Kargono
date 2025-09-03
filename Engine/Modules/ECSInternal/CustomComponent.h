#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Core/WrappedData.h"
#include "Modules/ECSInternal/ECSInternalCommon.h"
#include "Modules/ECSInternal/Module/ECSInternalModule.h"
#include "Modules/ECSInternal/Module/ComponentTag.h"

#include <array>
#include <cstdint>
#include <vector>
#include <string>
#include <limits>
#include <algorithm>

namespace Kargono::ECSInternal
{
	struct CustomComponent
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		CustomComponent() = default;
		~CustomComponent() = default;
	public:
		//==============================
		// Interact w/ Fields
		//==============================
		bool AddField(WrappedVarType fieldType, const char* fieldName);
		void DeleteField(size_t fieldIndex);
		bool EditField(size_t fieldIndex, const char* fieldName, WrappedVarType fieldType);
	private:
		// Helpers
		void RecalculateDataLocations();

	public:
		//==============================
		// Copy Function(s)
		//==============================
		void CopyTo(void* src, void* dst)
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
	public:
		//==============================
		// Getters/Setters
		//==============================
		ComponentIdentifier RevalidateIdentifier();
		ComponentMetadata GenerateMetadata() const;
		size_t RevalidateAlignment();
	public:
		//==============================
		// Public Fields
		//==============================
		FixedBufStr32 m_Name{};
		size_t m_ComponentSize{ 0 };
		size_t m_ComponentAlignment{ 1 };
		ComponentIdentifier m_Identifier{ k_InvalidComponentIdentifier };
		std::vector<WrappedVarType> m_DataTypes;
		std::vector<size_t> m_DataOffsets;
		std::vector<FixedBufStr32> m_DataNames;
	};

	Register_Module_Type(CustomComponent)

	inline void CustomComponentCopyTo(void* src, void* dst, void* customComp)
	{
		KG_ASSERT(src);
		KG_ASSERT(dst);
		KG_ASSERT(customComp);

		CustomComponent* customComponent{(CustomComponent*)customComp};

		customComponent->CopyTo(src, dst);
	}

}
