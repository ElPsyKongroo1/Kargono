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

	using u8vec2 = glm::u8vec2;
	using u8vec3 = glm::u8vec3;
	using u8vec4 = glm::u8vec4;

	using u16vec2 = glm::u16vec2;
	using u16vec3 = glm::u16vec3;
	using u16vec4 = glm::u16vec4;

	using u32vec2 = glm::u32vec2;
	using u32vec3 = glm::u32vec3;
	using u32vec4 = glm::u32vec4;

	using u64vec2 = glm::u64vec2;
	using u64vec3 = glm::u64vec3;
	using u64vec4 = glm::u64vec4;

	using ivec2 = glm::ivec2;
	using ivec3 = glm::ivec3;
	using ivec4 = glm::ivec4;

	using mat2 = glm::mat2;
	using mat3 = glm::mat3;
	using mat4 = glm::mat4;

	using quat = glm::quat;
}
