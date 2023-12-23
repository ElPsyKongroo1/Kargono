#pragma once
#include "Shader.h"

namespace Kargono
{
	class ShaderBuilder
	{
	public:
		static std::tuple<ShaderSource, InputBufferLayout, UniformBufferList> BuildShader(const ShaderSpecification& shaderSpec);
	};
	
}
