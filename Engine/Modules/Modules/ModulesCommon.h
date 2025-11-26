#pragma once

#include <type_traits>
#include <concepts>
#include <tuple>
#include <limits>
#include <cstdint>

namespace Kargono::Detail
{
    template <typename t_Tag, typename t_QueryType>
    concept TagCheckConcept = requires
    {
        { t_Tag::template Check<t_QueryType>() } -> std::convertible_to<bool>;
    };

    // Detection for tags with a Check<T>() member
    template<typename t_Tag, typename t_QueryType, typename = void> requires TagCheckConcept<t_Tag, t_QueryType>
    struct TagCheck : std::true_type {};

    template<typename t_Tag, typename t_QueryType> requires TagCheckConcept<t_Tag, t_QueryType>
    struct TagCheck<t_Tag, t_QueryType, std::void_t<decltype(t_Tag::template Check<t_QueryType>())>>
        : std::bool_constant<t_Tag::template Check<t_QueryType>()> {
    };

    template<typename t_Tag, typename t_QueryType>
    inline constexpr bool TagCheck_v = TagCheck<t_Tag, t_QueryType>::value;

    // Validate all tags in a tuple
    template<typename t_QueryType, typename t_Tuple, size_t... t_Index>
    consteval bool ValidateTagsImpl(std::index_sequence<t_Index...>)
    {
        return (TagCheck_v<std::tuple_element_t<t_Index, t_Tuple>, t_QueryType> && ...);
    }

    template<typename t_QueryType, typename t_Tuple>
    consteval bool ValidateTags()
    {
        return ValidateTagsImpl<t_QueryType, t_Tuple>(
            std::make_index_sequence<std::tuple_size_v<t_Tuple>>{}
        );
    }
}

namespace Kargono::Modules
{
    using TypeIdentifier = uint32_t;
    constexpr TypeIdentifier k_InvalidModuleTypeID
    {
        std::numeric_limits<TypeIdentifier>::max()
    };
}