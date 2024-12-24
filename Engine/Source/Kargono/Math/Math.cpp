#include "kgpch.h"

#include "Kargono/Math/Math.h"

#include "API/Math/glmBackendAPI.h"

namespace Kargono::Math
{

	bool DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale)
	{
		// From glm::decompose in matrix_decompose.inl

		mat4 LocalMatrix(transform);

		// Normalize the matrix.
		if (glm::epsilonEqual(LocalMatrix[3][3], (float)0, glm::epsilon<float>()))
		{
			return false;
		}

		// First, isolate perspective.  This is the messiest.
		if (
			glm::epsilonNotEqual(LocalMatrix[0][3], (float)0, glm::epsilon<float>()) ||
			glm::epsilonNotEqual(LocalMatrix[1][3], (float)0, glm::epsilon<float>()) ||
			glm::epsilonNotEqual(LocalMatrix[2][3], (float)0, glm::epsilon<float>()))
		{
			// Clear the perspective partition
			LocalMatrix[0][3] = LocalMatrix[1][3] = LocalMatrix[2][3] = static_cast<float>(0);
			LocalMatrix[3][3] = (float)1;
		}

		// Next take care of translation (easy).
		translation = vec3(LocalMatrix[3]);
		LocalMatrix[3] = vec4(0, 0, 0, LocalMatrix[3].w);

		vec3 Row[3];

		// Now get scale and shear.
		for (glm::length_t i = 0; i < 3; ++i)
		{
			for (glm::length_t j = 0; j < 3; ++j)
			{
				Row[i][j] = LocalMatrix[i][j];
			}
		}
			
		// Compute X scale factor and normalize first row.
		scale.x = length(Row[0]);
		Row[0] = glm::detail::scale(Row[0], (float)1);
		scale.y = length(Row[1]);
		Row[1] = glm::detail::scale(Row[1], (float)1);
		scale.z = length(Row[2]);
		Row[2] = glm::detail::scale(Row[2], (float)1);

		// At this point, the matrix (in rows[]) is orthonormal.
		// Check for a coordinate system flip.  If the determinant
		// is -1, then negate the matrix and the scaling factors.

		rotation.y = asin(-Row[0][2]);
		if (cos(rotation.y) != 0) {
			rotation.x = atan2(Row[1][2], Row[2][2]);
			rotation.z = atan2(Row[0][1], Row[0][0]);
		}
		else {
			rotation.x = atan2(-Row[2][0], Row[1][1]);
			rotation.z = 0;
		}

		return true;
	}

}
