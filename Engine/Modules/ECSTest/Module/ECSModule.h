#pragma once

#include "Modules/Core/Module.h"

namespace Kargono::ECS
{
	Register_Module(ECS)

    template <typename t_Type>
    concept Copyable = requires (void* src, void* dest)
    {
        { t_Type::CopyTo(src, dest) } -> std::same_as<void>;
    };

    using CopyFunc = void (*)(void* src, void* dest);

    template <typename t_Type>
    concept ComponentConcept = Copyable<t_Type>;

    Register_Module_Tag(Component, ComponentConcept<t_Type>)
}





