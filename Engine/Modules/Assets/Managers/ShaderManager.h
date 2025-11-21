#pragma once

#include "Modules/Assets/AssetManager.h"
#include "Modules/Rendering/Assets/Shader.h"

namespace Kargono::Assets
{
	static inline AssetManager<Rendering::Shader> s_ShaderManager{};
}