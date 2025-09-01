#pragma once

#include <concepts>

namespace Kargono
{
    template <typename t_Type>
    concept Copyable = requires (t_Type * src, t_Type * dest)
    {
        { src->CopyTo(dest) } -> std::same_as<void>;
    };

    using CopyFunc = void (*)(void* src, void* dest);

    template<Copyable t_Type>
    void TypeErasedCopy(void* src, void* dst)
    {
        static_cast<t_Type*>(src)->CopyTo(static_cast<t_Type*>(dst));
    }
}