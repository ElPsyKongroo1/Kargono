#pragma once

#include "implot.h"
#include "implot_internal.h"

namespace Kargono::API
{
	inline void InitImPlot()
	{
		ImPlot::CreateContext();
	}

	inline void TerminateImPlot()
	{
		ImPlot::DestroyContext();
	}
}
