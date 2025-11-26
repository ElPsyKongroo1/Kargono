#pragma once

#include "Modules/Modules/ModulesCommon.h"

namespace Kargono::Modules
{
// Use to register a module in a same-named namespace
#define Register_Module(moduleName)                                     \
    struct moduleName##Tag;                                             \
    template <typename t_ModuleType>                                               \
    struct TypeInfo                                                     \
    {                                                                   \
        constexpr static std::string_view TypeName{ "Default Name" };   \
        using Tags = std::tuple<>;                                      \
    };                                                                  \
    template <typename t_ModuleType>                                               \
    struct ModuleTypeTraits                                             \
    {                                                                   \
        constexpr static std::string_view ModuleName { #moduleName };   \
        constexpr static std::string_view TypeName{ TypeInfo<t_ModuleType>::TypeName }; \
        using ModuleTag = moduleName##Tag ;                             \
        using Tags = typename TypeInfo<t_ModuleType>::Tags;                        \
    };                                                                  \
    template <typename t_ModuleType>                                               \
    auto GetModuleTraits(const t_ModuleType& dummy) -> decltype(auto)              \
    {                                                                   \
        return ModuleTypeTraits<t_ModuleType>{};                                   \
    };
// Use to register a module tag that can be used to constrain module types
#define Register_Module_Tag(name, validTagExpression) \
    struct name##Tag \
    { \
        template<typename t_ModuleType> \
        consteval static bool Check() \
        { \
            return validTagExpression; \
        } \
    };

// Use to register a module type with associated tags and within the current module namespace
#define Register_Module_Type(type, ...)                                 \
    template <>                                                         \
    struct TypeInfo<type>                                               \
    {                                                                   \
        constexpr static std::string_view TypeName{ #type };            \
        using Tags = std::tuple<__VA_ARGS__>;                           \
        static consteval bool Validate() { return Kargono::Detail::ValidateTags<type, Tags>(); } }; \
    static_assert(TypeInfo<type>::Validate(), "Failed to satisfy tag contraints");
}