#pragma once
#include "Modules/ECSInternal/Module/ECSInternalModule.h"
#include "Modules/Core/Concepts/Copyable.h"
#include "Modules/FileSystem/FileSystem.h"
#include "Modules/Core/Module.h"

#include <cstdint>
#include <limits>

namespace Kargono::ECSInternal
{
	// Component Tag
    template <typename t_Type>
    concept ComponentConcept = Copyable<t_Type>;

    Register_Module_Tag(Component, ComponentConcept<t_Type>)

	// Component Identifier
	using ComponentIdentifier = ModuleTypeIdentifier;
	constexpr ComponentIdentifier k_InvalidComponentIdentifier
	{
		std::numeric_limits<ComponentIdentifier>::max()
	};

	template<size_t t_NumComponents>
	using ComponentIDList = std::array<ComponentIdentifier, t_NumComponents>;

	template<ComponentConcept t_ComponentType>
	consteval ComponentIdentifier GetComponentIdentifier()
	{
		return GetModuleTypeIdentifier<t_ComponentType>();
	}
}