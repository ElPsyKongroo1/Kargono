#pragma once
#include "Modules/Rendering/Shader.h"

namespace Kargono::Rendering
{
	class ShaderBuilder
	{
	public:
		static std::tuple<ShaderSource, InputBufferLayout, UniformBufferList> BuildShader(const ShaderSpecification& shaderSpec);
	};
	
}
