#pragma once
#ifdef KARGONO_EXPORTS
#define KARGONO_API __declspec(dllexport)
#else
#define KARGONO_API __declspec(dllimport)
#endif
#include <functional>
#include <string>
#include "F:\Charles_Files\Personal_Projects\Personal_Games\Engine/src/Kargono/Math/MathAliases.h"
namespace Kargono
{
extern "C"
	{
KARGONO_API void AddVoidNone(const std::string& funcName, std::function<void()> funcPtr);
KARGONO_API void AddVoidString(const std::string& funcName, std::function<void(const std::string&)> funcPtr);
KARGONO_API void AddVoidStringBool(const std::string& funcName, std::function<void(const std::string&, bool)> funcPtr);
KARGONO_API void AddVoidStringString(const std::string& funcName, std::function<void(const std::string&, const std::string&)> funcPtr);
KARGONO_API void AddVoidStringStringBool(const std::string& funcName, std::function<void(const std::string&, const std::string&, bool)> funcPtr);
KARGONO_API void AddVoidStringStringString(const std::string& funcName, std::function<void(const std::string&, const std::string&, const std::string&)> funcPtr);
KARGONO_API void AddVoidStringStringVec4(const std::string& funcName, std::function<void(const std::string&, const std::string&, Math::vec4)> funcPtr);
KARGONO_API void AddUInt16None(const std::string& funcName, std::function<uint16_t()> funcPtr);
		KARGONO_API void KG_FUNC_3176244785148247992(uint16_t a);
		KARGONO_API void KG_FUNC_18163504705534252383(uint32_t a);
		KARGONO_API void KG_FUNC_7445822592925037095(uint16_t a);
		KARGONO_API void KG_FUNC_8444652895326507655();
		KARGONO_API void KG_FUNC_1819971799574496652();
		KARGONO_API void KG_FUNC_4342390925410131221();
		KARGONO_API void KG_FUNC_14683932765512752045();
	}
}
