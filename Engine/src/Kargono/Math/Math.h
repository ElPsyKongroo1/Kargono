#pragma once
#include "Kargono/Math/MathAliases.h"

//============================================================
// Math Namespace
//============================================================
// This namespace provides a central location to add simple/complex math
//		functionality tools for use by the rest of the engine.
namespace Kargono::Math
{
	//==============================
	// Matrix Functions
	//==============================

	// This function provides a method for decomposing the provided matrix
	//		into a translation, rotation, and scale. The results of the operation
	//		are placed inside the provided translation, rotation, and scale
	//		variables. These is a boolean to indicate if the operation was
	//		successful.
	bool DecomposeTransform(const glm::mat4& transform, glm::vec3& translation,
			glm::vec3& rotation, glm::vec3& scale);
	
}
