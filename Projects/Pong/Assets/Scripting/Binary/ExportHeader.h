#pragma once
#ifdef KARGONO_EXPORTS
#define KARGONO_API __declspec(dllexport)
#else
#define KARGONO_API __declspec(dllimport)
#endif
#include <functional>
#include <string>
namespace Kargono
{
extern "C"
	{
KARGONO_API void AddVoidNone(const std::string& funcName, std::function<void()> funcPtr);
KARGONO_API void AddVoidString(const std::string& funcName, std::function<void(const std::string&)> funcPtr);
KARGONO_API void AddVoidStringStringString(const std::string& funcName, std::function<void(const std::string&, const std::string&, const std::string&)> funcPtr);
		KARGONO_API void KG_FUNC_7448269885476125662();
		KARGONO_API void KG_FUNC_3176244785148247992(uint16_t a);
		KARGONO_API void KG_FUNC_7445822592925037095(uint16_t a);
	}
}
