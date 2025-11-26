#pragma once

#include "Modules/Modules/RegisterModule.h"

#include <xutility>

namespace Kargono::Modules
{
	template<typename t_Tag>
	concept IsModuleTag = Detail::TagCheckConcept<t_Tag, std::monostate>;
}