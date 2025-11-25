#pragma once

#include "Modules/Assets/AssetManager.h"
#include "Modules/ECSInternal/Assets/CustomComponent.h"

namespace Kargono::Assets
{
	extern AssetManager<ECSInternal::CustomComponent> s_CustomComponentManager;
}