#pragma once

#include "Kargono/Utility/Operations.h"
#include "Kargono/Utility/CompilerInfo.h"
#include "Modules/FileSystem/FileSystem.h"

#include <sstream>
#include <type_traits>
#include <concepts>
#include <tuple>

namespace Kargono
{
    template <typename t_Tag, typename t_QueryType>
    concept ModuleTag = requires
    {
        { t_Tag::template Check<t_QueryType>() } -> std::convertible_to<bool>;
    };

    // Detection for tags with a Check<T>() member
    template<typename t_Tag, typename t_QueryType, typename = void> requires ModuleTag<t_Tag, t_QueryType>
    struct TagCheck : std::true_type {};

    template<typename t_Tag, typename t_QueryType> requires ModuleTag<t_Tag, t_QueryType>
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

#define Register_Module(moduleName)                                     \
    struct moduleName##Tag;                                             \
    template <typename T>                                               \
    struct TypeInfo                                                     \
    {                                                                   \
        constexpr static std::string_view TypeName{ "Default Name" };   \
        using Tags = std::tuple<>;                                      \
    };                                                                  \
    template <typename T>                                               \
    struct ModuleTypeTraits                                             \
    {                                                                   \
        constexpr static std::string_view ModuleName { #moduleName };   \
        constexpr static std::string_view TypeName{ TypeInfo<T>::TypeName }; \
        using ModuleTag = moduleName##Tag ;                             \
        using Tags = typename TypeInfo<T>::Tags;                        \
    };                                                                  \
    template <typename T>                                               \
    auto GetModuleTraits(const T& dummy) -> decltype(auto)              \
    {                                                                   \
        return ModuleTypeTraits<T>{};                                   \
    };

#define Register_Module_Tag(name, validTagExpression) \
    struct name##Tag \
    { \
        template<typename t_Type> \
        consteval static bool Check() \
        { \
            return validTagExpression; \
        } \
    };


#define Register_Module_Type(type, ...)                                 \
    template <>                                                         \
    struct TypeInfo<type>                                               \
    {                                                                   \
        constexpr static std::string_view TypeName{ #type };            \
        using Tags = std::tuple<__VA_ARGS__>;                           \
        static consteval bool Validate() { return Kargono::ValidateTags<type, Tags>(); } }; \
    static_assert(TypeInfo<type>::Validate(), "Failed to satisfy tag contraints");

#define Module_Type_Traits(type) decltype(GetModuleTraits(std::declval<type>()))

    template<typename t_Type, typename... t_QueryTags>
    constexpr bool HasTags()
    {
        return Utility::tuple_has_any_v<typename Module_Type_Traits(t_Type)::Tags, t_QueryTags...>;
    }

    template<typename t_Type>
    constexpr size_t GetTagCount()
    {
        return std::tuple_size_v<typename Module_Type_Traits(t_Type)::Tags>;
    }

    template<typename t_Type>
    constexpr std::string_view GetModuleName()
    {
        return Module_Type_Traits(t_Type)::ModuleName;
    }

    template<typename t_Type>
    std::string_view GetTypeName()
    {
        return Module_Type_Traits(t_Type)::TypeName;
    }

    template<typename t_Type>
    constexpr auto GetUniqueIdentifier()
    {
        constexpr std::string_view moduleName{ Module_Type_Traits(t_Type)::ModuleName };
        constexpr std::string_view delimiter{ "::" };
        constexpr std::string_view identifier{ Module_Type_Traits(t_Type)::TypeName };

        constexpr size_t bufferSize{ moduleName.size() + delimiter.size() + identifier.size() + 1 };

        FixedBufferString<bufferSize> returnValue{};

        returnValue.Append(moduleName.data());
        returnValue.Append(delimiter.data());
        returnValue.Append(identifier.data());

        return returnValue;
    }

    // Type Identifiers
    using ModuleTypeIdentifier = uint32_t;
    constexpr ModuleTypeIdentifier k_InvalidModuleTypeID
    {
        std::numeric_limits<ModuleTypeIdentifier>::max()
    };

    template<typename t_Type>
    consteval ModuleTypeIdentifier GetModuleTypeIdentifier()
    {
        constexpr auto name{ GetUniqueIdentifier<t_Type>() };
        constexpr ModuleTypeIdentifier identifier
        { Utility::FileSystem::CRCFromString(name.CString()) };
        return identifier;
    }

#undef Module_Type_Traits
}

