#pragma once

#include "Kargono/Core/Base.h"

#ifdef KG_ENABLE_ASSERTS


// Alteratively we could use the same "default" message for both "WITH_MSG" and "NO_MSG" and
// provide support for custom formatting by concatenating the formatting string instead of having the format inside the default message
#define KG_INTERNAL_ASSERT_IMPL(type, check, msg, ...) { if(!(check)) { KG##type##ERROR(msg, __VA_ARGS__); KG_DEBUGBREAK(); } }
#define KG_INTERNAL_ASSERT_WITH_MSG(type, check, ...) KG_INTERNAL_ASSERT_IMPL(type, check, "Assertion failed: {0}", __VA_ARGS__)
#define KG_INTERNAL_ASSERT_NO_MSG(type, check) KG_INTERNAL_ASSERT_IMPL(type, check, "Assertion '{0}' failed at {1}:{2}", KG_STRINGIFY_MACRO(check), std::filesystem::path(__FILE__).filename().string(), __LINE__)

#define KG_INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
#define KG_INTERNAL_ASSERT_GET_MACRO(...) KG_EXPAND_MACRO( KG_INTERNAL_ASSERT_GET_MACRO_NAME(__VA_ARGS__, KG_INTERNAL_ASSERT_WITH_MSG, KG_INTERNAL_ASSERT_NO_MSG) )

// Currently accepts at least the condition and one additional parameter (the message) being optional
#define KG_ASSERT(...) KG_EXPAND_MACRO( KG_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_, __VA_ARGS__) )
#else
#define KG_ASSERT(...)
#endif
