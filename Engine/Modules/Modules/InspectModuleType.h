#pragma once

#include "Kargono/Utility/Operations.h"
#include "Kargono/Utility/CompilerInfo.h"
#include "Modules/FileSystem/FileSystem.h"
#include "Kargono/Core/FixedBufferString.h"

#include "Modules/Modules/ModulesCommon.h"

#define Module_Type_Traits(type) decltype(GetModuleTraits(std::declval<type>()))

namespace Kargono::Modules
{
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
    constexpr auto GetUniqueTypeName()
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

    template<typename t_Type>
    consteval TypeIdentifier GetTypeIdentifier()
    {
        constexpr auto name{ GetUniqueTypeName<t_Type>() };
        constexpr TypeIdentifier identifier 
        { 
            Utility::FileSystem::CRCFromString(name.CString()) 
        };
        return identifier;
    }

}

#undef Module_Type_Traits