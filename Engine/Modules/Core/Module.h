#pragma once

#include "Kargono/Utility/Operations.h"
#include "Kargono/Utility/CompilerInfo.h"

#include <sstream>
#include <type_traits>
#include <concepts>
#include <tuple>

namespace Kargono
{

#define Register_Module(moduleName)                                    \
    struct moduleName##Tag;                                             \
    template <typename T>                                               \
    struct TypeTags                                                     \
    {                                                                   \
        using Tags { std::tuple<> };                                    \
    };                                                                  \
    template <typename T>                                               \
    struct ModuleTypeTraits                                             \
    {                                                                   \
        constexpr static std::string_view ModuleName { #moduleName };   \
        using ModuleTag { moduleName##Tag };                            \
        using Tags { typename TypeTags<T>::Tags };                      \
    };                                                                  \
    template <typename T>                                               \
    auto GetModuleTraits(const T& dummy) -> decltype(auto)              \
    {                                                                   \
        return ModuleTypeTraits<T>{};                                   \
    };


#define Register_Tags(type, ...)                                        \
    template <>                                                         \
    struct TypeTags<type>                                               \
    {                                                                   \
        using Tags = std::tuple<__VA_ARGS__>;                           \
    };

#define ModuleTypeTraits(type) decltype(GetModuleTraits(std::declval<type>()))

    template<typename t_Type, typename... t_QueryTags>
    constexpr bool HasTags()
    {
        return Utility::tuple_has_any_v<typename ModuleTypeTraits(t_Type)::Tags, t_QueryTags...>;
    }

    template<typename t_Type>
    constexpr size_t GetTagCount()
    {
        return std::tuple_size_v<typename ModuleTypeTraits(t_Type)::Tags>;
    }

    template<typename t_Type>
    constexpr std::string_view GetModuleName()
    {
        return ModuleTypeTraits(t_Type)::ModuleName;
    }

    template<typename t_Type>
    constexpr std::string GetUniqueIdentifier()
    {
        constexpr std::string_view identifier
        { 
            Utility::CompilerInfo::GetTemplateArgumentNames<t_Type>()[0] 
        };

        return { ModuleTypeTraits(t_Type)::ModuleName + "::" + identifier};
    }

#undef ModuleTypeTraits
}

