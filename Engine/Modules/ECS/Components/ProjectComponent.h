#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Core/WrappedData.h"
#include "Modules/ECSInternal/ECSInternalCommon.h"
#include "Modules/ECS/Module/ECSModule.h"
#include "Modules/ECSInternal/Module/ComponentTag.h"

#include <array>
#include <cstdint>
#include <vector>
#include <string>
#include <limits>
#include <algorithm>

namespace Kargono::ECS
{
	struct ProjectComponent
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		ProjectComponent() = default;
		~ProjectComponent() = default;
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
		// Getters/Setters
		//==============================
		ECSInternal::ComponentIdentifier RevalidateIdentifier();
		ECSInternal::ComponentMetadata GenerateMetadata() const;
		size_t RevalidateAlignment();
	public:
		//==============================
		// Public Fields
		//==============================
		FixedBufStr32 m_Name{};
		size_t m_ComponentSize{ 0 };
		size_t m_ComponentAlignment{ 1 };
		ECSInternal::ComponentIdentifier m_Identifier{ ECSInternal::k_InvalidComponentIdentifier };
		std::vector<WrappedVarType> m_DataTypes;
		std::vector<size_t> m_DataOffsets;
		std::vector<FixedBufStr32> m_DataNames;
	};



	Register_Module_Type(ProjectComponent)


}
