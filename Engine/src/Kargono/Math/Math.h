#pragma once
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

//============================================================
// Math Namespace
//============================================================
// This namespace provides a central location to add simple/complex math
//		functionality tools for use by the rest of the engine.
namespace Kargono::Math
{
	//==============================
	// Math Type Aliases
	//==============================

	// These type aliases provide a central location to manage the
	//		underlying math library implementation. This makes
	//		the Kargono API much simpler to use.

	using vec2 = glm::vec2;
	using vec3 = glm::vec3;
	using vec4 = glm::vec4;

	using uvec2 = glm::uvec2;
	using uvec3 = glm::uvec3;
	using uvec4 = glm::uvec4;

	using ivec2 = glm::ivec2;
	using ivec3 = glm::ivec3;
	using ivec4 = glm::ivec4;

	using mat2 = glm::mat2;
	using mat3 = glm::mat3;
	using mat4 = glm::mat4;

	using quat = glm::quat;

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
