#pragma once

#include <concepts>

namespace Kargono
{
    template <typename t_Type>
    concept IsCopyable = requires (t_Type * src, t_Type * dest)
    {
        { src->CopyTo(dest) } -> std::same_as<void>;
    };

    using CopyFunc = void (*)(void* src, void* dest, void* customData);

    template<IsCopyable t_Type>
    void TypeErasedCopy(void* src, void* dst, void* /*customData*/)
    {
        static_cast<t_Type*>(src)->CopyTo(static_cast<t_Type*>(dst));
    }
}