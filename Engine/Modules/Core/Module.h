#pragma once

#include "Kargono/Utility/Operations.h"
#include "Kargono/Utility/CompilerInfo.h"

#include <sstream>
#include <type_traits>
#include <concepts>
#include <tuple>

namespace Kargono
{
#define Register_Module(moduleName)                                     \
    struct moduleName##Tag;                                             \
    template <typename T>                                               \
    struct TypeInfo                                                     \
    {                                                                   \
        constexpr static std::string_view TypeName{ "Default Name" };               \
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


#define Register_Module_Type(type, ...)                                        \
    template <>                                                         \
    struct TypeInfo<type>                                               \
    {                                                                   \
        constexpr static std::string_view TypeName{ #type };            \
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
    std::string_view GetTypeName()
    {
        return ModuleTypeTraits(t_Type)::TypeName;
    }

    template<typename t_Type>
    constexpr auto GetUniqueIdentifier()
    {
        constexpr std::string_view moduleName{ ModuleTypeTraits(t_Type)::ModuleName };
        constexpr std::string_view delimiter{ "::" };
        constexpr std::string_view identifier{ ModuleTypeTraits(t_Type)::TypeName };

        constexpr size_t bufferSize{ moduleName.size() + delimiter.size() + identifier.size() + 1 };

        FixedBufferString<bufferSize> returnValue{};

        returnValue.Append(moduleName.data());
        returnValue.Append(delimiter.data());
        returnValue.Append(identifier.data());

        return returnValue;
    }

#undef ModuleTypeTraits
}

