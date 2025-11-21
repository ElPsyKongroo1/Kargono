#pragma once

#include "Modules/Assets/AssetManager.h"
#include "Modules/ECSInternal/Assets/CustomComponent.h"

namespace Kargono::Assets
{
	static inline AssetManager<ECSInternal::CustomComponent> s_CustomComponentManager{};
}