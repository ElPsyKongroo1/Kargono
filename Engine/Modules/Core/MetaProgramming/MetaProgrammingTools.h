#pragma once

#include "type_traits"

namespace Kargono
{
    template<typename t_ExpectedType, typename t_ActualType>
    consteval void EnforceTypesMatch()
    {
        static_assert(std::is_same_v<std::remove_cvref_t<t_ActualType>, t_ExpectedType>, "Type mismatch");
    }
}