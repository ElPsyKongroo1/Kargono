#pragma once

#include "Modules/Assets/AssetManager.h"
#include "Modules/Scripting/Assets/CustomEnum.h"

namespace Kargono::Assets
{
	extern AssetManager<Scripting::CustomEnum> s_CustomEnumManager;
}