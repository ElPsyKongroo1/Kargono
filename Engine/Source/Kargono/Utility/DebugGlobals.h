#pragma once
#include "Kargono/Math/MathAliases.h"

#if defined(KG_DEBUG)
namespace Kargono::Utility
{
	class DebugGlobals
	{
	public:
		inline static bool s_TestBool_1{ false };
		inline static bool s_TestBool_2{ false };
		inline static bool s_TestBool_3{ false };

		inline static float s_TestFloat_1{ 60.0f };
		inline static float s_TestFloat_2{ 15.0f };
		inline static float s_TestFloat_3{ 0.0f };

		inline static int s_TestInt_1{ 0 };
		inline static int s_TestInt_2{ 0 };
		inline static int s_TestInt_3{ 0 };

		inline static unsigned int s_TestUInt_1{ 15 };
		inline static unsigned int s_TestUInt_2{ 0 };
		inline static unsigned int s_TestUInt_3{ 0 };

		inline static Math::vec2 s_TestVec2_1{ 0.0f };
		inline static Math::vec2 s_TestVec2_2{ 0.0f };
		inline static Math::vec2 s_TestVec2_3{ 0.0f };

		inline static Math::vec3 s_TestVec3_1{ 0.0f };
		inline static Math::vec3 s_TestVec3_2{ 0.0f };
		inline static Math::vec3 s_TestVec3_3{ 0.0f };

		inline static Math::vec4 s_TestVec4_1{ 0.0f };
		inline static Math::vec4 s_TestVec4_2{ 0.0f };
		inline static Math::vec4 s_TestVec4_3{ 0.0f };
	};
}

#endif
