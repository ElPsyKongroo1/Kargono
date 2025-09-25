#pragma once
#include "Modules/Core/Concepts/Copyable.h"

namespace Kargono::ECSInternal
{
	template <typename t_Type>
	concept ComponentConcept = Copyable<t_Type>;
}