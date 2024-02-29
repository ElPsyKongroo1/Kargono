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
		KARGONO_API void KG_FUNC_753079771072191180();
		KARGONO_API void KG_FUNC_7448269885476125662();
	}
}
