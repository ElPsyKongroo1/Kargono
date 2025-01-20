#pragma once
#include "Kargono/Math/MathAliases.h"

#include <vector>
namespace Kargono::Math
{
	
	struct Spline
	{
		std::vector<Math::vec3> m_Points;
		bool m_Looped{ false };
	};

	class SplineService
	{
	public:
		static Math::vec3 GetSplinePoint(const Spline& spline, float time);
		static Math::vec3 GetSplineGradient(const Spline& spline, float time);
	};

}
