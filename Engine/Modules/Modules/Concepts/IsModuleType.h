#pragma once
#include "Modules/Modules/RegisterModule.h"
#include "Modules/Modules/InspectModuleType.h"

namespace Kargono::Modules
{
	template<typename t_Type>
	concept IsModuleType = GetTagCount<t_Type>() > 0;
}