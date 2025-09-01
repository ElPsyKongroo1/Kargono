#pragma once
#include "Modules/ECSInternal/Module/ECSInternalModule.h"
#include "Modules/Core/Concepts/Copyable.h"

namespace Kargono::ECSInternal
{
    template <typename t_Type>
    concept ComponentConcept = Copyable<t_Type>;

    Register_Module_Tag(Component, ComponentConcept<t_Type>)
}