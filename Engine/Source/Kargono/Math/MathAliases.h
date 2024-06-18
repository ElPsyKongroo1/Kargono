#pragma once
#include "API/Math/glmAPI.h"

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
}
