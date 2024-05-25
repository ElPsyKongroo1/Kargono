#pragma once
#ifdef KARGONO_EXPORTS
#define KARGONO_API __declspec(dllexport)
#else
#define KARGONO_API __declspec(dllimport)
#endif
#include <functional>
#include <string>
#include "Kargono/Math/MathAliases.h"
namespace Kargono
{
extern "C"
	{
KARGONO_API void AddVoidNone(const std::string& funcName, std::function<void()> funcPtr);
KARGONO_API void AddVoidString(const std::string& funcName, std::function<void(const std::string&)> funcPtr);
KARGONO_API void AddVoidStringBool(const std::string& funcName, std::function<void(const std::string&, bool)> funcPtr);
KARGONO_API void AddVoidStringVoidPtr(const std::string& funcName, std::function<void(const std::string&, void*)> funcPtr);
KARGONO_API void AddVoidStringString(const std::string& funcName, std::function<void(const std::string&, const std::string&)> funcPtr);
KARGONO_API void AddVoidStringStringBool(const std::string& funcName, std::function<void(const std::string&, const std::string&, bool)> funcPtr);
KARGONO_API void AddVoidStringStringString(const std::string& funcName, std::function<void(const std::string&, const std::string&, const std::string&)> funcPtr);
KARGONO_API void AddVoidStringStringVec4(const std::string& funcName, std::function<void(const std::string&, const std::string&, Math::vec4)> funcPtr);
KARGONO_API void AddUInt16None(const std::string& funcName, std::function<uint16_t()> funcPtr);
		KARGONO_API void UpdateSessionUserSlot(uint16_t a);
		KARGONO_API void UpdateOnlineCount(uint32_t a);
		KARGONO_API void UserLeftSession(uint16_t a);
		KARGONO_API void OnCurrentSessionInit();
		KARGONO_API void ApproveJoinSession(uint16_t a);
		KARGONO_API void OnStartSession();
		KARGONO_API void OnConnectionTerminated();
		KARGONO_API void OpenMainMenu();
	}
}
