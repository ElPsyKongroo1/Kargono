#pragma once
#include "Kargono/Core/Base.h"

#include <string_view>
#include <array>
#include <cstddef>

#if !defined(KG_COMPILER_MSVC)
#error "Only MSVC compiler is defined for this code"
#endif

namespace Kargono::Utility
{
    template<typename... t_Args>
    using ReturnTemplateNames = std::array<std::string_view, sizeof...(t_Args)>;

    class CompilerInfo
    {
    public:
        //=========================
        // Get Template Info
        //=========================
        template <typename... t_ArgPack>
        constexpr static ReturnTemplateNames<t_ArgPack...> GetTemplateArgumentNames()
        {
            constexpr size_t k_TempArgsCount{ sizeof...(t_ArgPack) };

            // Do not call w/ 0 arguments
            static_assert(k_TempArgsCount > 0);

            // Get full function signature from MSVC
            // Ex: "constexpr auto __cdecl GetTemplateArgumentNames<int,float>(void)"
            std::string_view fullSignature{ __FUNCSIG__ };

            // Get rid of all text before template argument list
            // Ex: GetTemplateArgumentNames<int,float>(void)
            // TODO: Note that another instance of GetTemplateArgumentNames in the definition could be an edge-case 
            size_t funcNameStart{ fullSignature.find("GetTemplateArgumentNames")};
            std::string_view stripRetValSig{ fullSignature.substr(funcNameStart, fullSignature.size() - funcNameStart)};

            // Extract the template arguments as string
            // Ex: -> "int, float"
            size_t startBracket { stripRetValSig.find('<') + 1 };
            size_t endBracket { stripRetValSig.rfind('>') };
            std::string_view tempArgsStr{ stripRetValSig.substr(startBracket, endBracket - startBracket) };

            // Split into array of string_views
            std::array<std::string_view, k_TempArgsCount> returnNames{ ParseTempArgs<k_TempArgsCount>(tempArgsStr)};

            return returnNames;
        }

    private:
        // Helper(s)
        constexpr static size_t GetTempArgsCount(std::string_view text)
        {
            // Early out if nothing provided
            if (text.empty())
            {
                return 0;
            }

            // Use number of commas as the arg count
            size_t count{ 1 };
            for (char ch : text)
            {
                if (ch == ',')
                {
                    count++;
                }
            }

            return count;
        }

        template<size_t t_ArgCount>
        constexpr static std::array<std::string_view, t_ArgCount> ParseTempArgs(std::string_view text)
        {
            // Local declarations
            std::array<std::string_view, t_ArgCount> returnArray{};
            // Indexes into the text
            size_t charIndex{ 0 };
            size_t previousCharIndex{ 0 };
            // Argument count index(s)
            size_t argumentIndex{ 0 };

            while (charIndex <= text.size())
            {
                // Check for end of string or a new argument
                if (charIndex == text.size() || text[charIndex] == ',')
                {
                    size_t argumentStartPos{ previousCharIndex };
                    size_t argumentEndPos{ charIndex };

                    // Find the starting position (ignore whitespace)
                    while (argumentStartPos < charIndex && text[argumentStartPos] == ' ')
                    {
                        argumentStartPos++;
                    }

                    // Find the ending position (ignore whitespace)
                    while (argumentEndPos > argumentStartPos && text[argumentEndPos - 1] == ' ')
                    {
                        argumentEndPos--;
                    }

                    // Isolate the argument Ex: class Kargono::Utility::identifier
                    std::string_view fullQualifiedArg{ text.substr(argumentStartPos, argumentEndPos - argumentStartPos) };

                    // Remove class/struct qualifier Ex: -> Kargono::Utility::identifier
                    std::string_view typeStripArg{ fullQualifiedArg };
                    size_t classQualifier{ typeStripArg.find("class ") };
                    size_t structQualifier{ typeStripArg.find("struct ") };
                    if (classQualifier == 0)
                    {
                        constexpr size_t k_ClassStrLength{ 6 };
                        typeStripArg = typeStripArg.substr(k_ClassStrLength, typeStripArg.size() - k_ClassStrLength);
                    }
                    else if (structQualifier == 0)
                    {
                        constexpr size_t k_StructStrLength{ 7 };
                        typeStripArg = typeStripArg.substr(k_StructStrLength, typeStripArg.size() - k_StructStrLength);
                    }

                    // Check for namespaces Ex: (Kargono::Utility::)identifier
                    size_t terminalName{ typeStripArg.rfind("::") };
                    std::string_view namespaceStripArg{ typeStripArg };

                    // Isolate the final identifier if necessary Ex: Kargono::Utility::identifier -> identifier
                    if (terminalName != typeStripArg.npos)
                    {
                        namespaceStripArg = typeStripArg.substr(terminalName + 2, typeStripArg.size() - terminalName + 2);
                    }

                    // Store the trimmed argument
                    returnArray[argumentIndex] = namespaceStripArg;
                    argumentIndex++;

                    // Store previous argument index
                    previousCharIndex = charIndex + 1;
                }

                // Progress forward
                charIndex++;
            }

            return returnArray;
        }
    };
    
}