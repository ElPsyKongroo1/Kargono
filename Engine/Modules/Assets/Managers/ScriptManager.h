#pragma once

#include "Modules/Assets/AssetManager.h"
#include "Modules/Scripting/Assets/Script.h"

namespace Kargono::Assets
{
	static inline AssetManager<Scripting::Script> s_ScriptManager{};
}