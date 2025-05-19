#pragma once
#include "Kargono/Math/MathAliases.h"

#if defined(KG_DEBUG)
namespace Kargono::Utility
{
	struct DebugGlobals
	{
	public:
		 bool m_TestBool_1{ false };
		 bool m_TestBool_2{ false };
		 bool m_TestBool_3{ false };

		 float m_TestFloat_1{ 60.0f };
		 float m_TestFloat_2{ 15.0f };
		 float m_TestFloat_3{ 0.0f };

		 int m_TestInt_1{ 0 };
		 int m_TestInt_2{ 0 };
		 int m_TestInt_3{ 0 };

		 unsigned int m_TestUInt_1{ 15 };
		 unsigned int m_TestUInt_2{ 0 };
		 unsigned int m_TestUInt_3{ 0 };

		 Math::vec2 m_TestVec2_1{ 0.0f };
		 Math::vec2 m_TestVec2_2{ 0.0f };
		 Math::vec2 m_TestVec2_3{ 0.0f };

		 Math::vec3 m_TestVec3_1{ 0.0f };
		 Math::vec3 m_TestVec3_2{ 0.0f };
		 Math::vec3 m_TestVec3_3{ 0.0f };

		 Math::vec4 m_TestVec4_1{ 0.0f };
		 Math::vec4 m_TestVec4_2{ 0.0f };
		 Math::vec4 m_TestVec4_3{ 0.0f };
	};

	class DebugGlobalsService
	{
	public:
		static DebugGlobals& GetDebugGlobals()
		{
			return s_Globals;
		}
	private:
		static inline DebugGlobals s_Globals;
	};
}

#endif
