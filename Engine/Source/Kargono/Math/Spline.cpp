#include "kgpch.h"

#include "Kargono/Math/Spline.h"

namespace Kargono::Math
{
	// TODO: Hey, I didn't finish implementing splines. Here is the video where I left off: https://www.youtube.com/watch?v=DzjtU4WLYNs
	Math::vec3 SplineService::GetSplinePoint(const Spline& spline, float time)
	{
		float adjustTimeFactor{ spline.m_Looped ? 0.0001f : 3.0001f };

		if (time > (float)spline.m_Points.size() - adjustTimeFactor)
		{
			time = (float)spline.m_Points.size() - adjustTimeFactor;
		}

		int point0;
		int point1;
		int point2;
		int point3;

		if (spline.m_Looped)
		{
			point1 = (int)time;
			point2 = (point1 + 1) % (int)spline.m_Points.size();
			point3 = (point2 + 1) % (int)spline.m_Points.size();
			point0 = point1 >= 1 ? point1 - 1 : (int)spline.m_Points.size() - 1;
		}
		else
		{
			point1 = (int)time + 1;
			point2 = point1 + 1;
			point3 = point2 + 1;
			point0 = point1 - 1;
		}
		

		time -= (int)time; // Get remainder (0.0f - 1.0f)

		float timeSquared = time * time;
		float timeCubed = time * time * time;

		float constant1 = -timeCubed + 2.0f * timeSquared - time;
		float constant2 = 3.0f * timeCubed - 5.0f * timeSquared + 2.0f;
		float constant3 = -3.0f * timeCubed + 4.0f * timeSquared + time;
		float constant4 = timeCubed - timeSquared;

		float xValue = spline.m_Points[point0].x * constant1 +
			spline.m_Points[point1].x * constant2 +
			spline.m_Points[point2].x * constant3 +
			spline.m_Points[point3].x * constant4;
		float yValue = spline.m_Points[point0].y * constant1 +
			spline.m_Points[point1].y * constant2 +
			spline.m_Points[point2].y * constant3 +
			spline.m_Points[point3].y * constant4;
		float zValue = spline.m_Points[point0].z * constant1 +
			spline.m_Points[point1].z * constant2 +
			spline.m_Points[point2].z * constant3 +
			spline.m_Points[point3].z * constant4;

		return Math::vec3(xValue, yValue, zValue) * 0.5f;
	}

	Math::vec3 SplineService::GetSplineGradient(const Spline& spline, float time)
	{
		float adjustTimeFactor{ spline.m_Looped ? 0.0001f : 3.0001f };

		if (time > (float)spline.m_Points.size() - adjustTimeFactor)
		{
			time = (float)spline.m_Points.size() - adjustTimeFactor;
		}

		int point0;
		int point1;
		int point2;
		int point3;

		if (spline.m_Looped)
		{
			point1 = (int)time;
			point2 = (point1 + 1) % (int)spline.m_Points.size();
			point3 = (point2 + 1) % (int)spline.m_Points.size();
			point0 = point1 >= 1 ? point1 - 1 : (int)spline.m_Points.size() - 1;
		}
		else
		{
			point1 = (int)time + 1;
			point2 = point1 + 1;
			point3 = point2 + 1;
			point0 = point1 - 1;
		}


		time -= (int)time; // Get remainder (0.0f - 1.0f)

		float timeSquared = time * time;
		float timeCubed = time * time * time;

		float constant1 = -3.0f * timeSquared + 4.0f * time - 1;
		float constant2 = 9.0f * timeSquared - 10.0f * time;
		float constant3 = -9.0f * timeSquared + 8.0f * time + 1.0f;
		float constant4 = 3.0f * timeSquared - 2.0f * time;

		float xValue = spline.m_Points[point0].x * constant1 +
			spline.m_Points[point1].x * constant2 +
			spline.m_Points[point2].x * constant3 +
			spline.m_Points[point3].x * constant4;
		float yValue = spline.m_Points[point0].y * constant1 +
			spline.m_Points[point1].y * constant2 +
			spline.m_Points[point2].y * constant3 +
			spline.m_Points[point3].y * constant4;
		float zValue = spline.m_Points[point0].z * constant1 +
			spline.m_Points[point1].z * constant2 +
			spline.m_Points[point2].z * constant3 +
			spline.m_Points[point3].z * constant4;

		return Math::vec3(xValue, yValue, zValue) * 0.5f;
	}
}
