#pragma once

#ifdef KG_PLATFORM_WINDOWS
	#ifdef KG_BUILD_DLL
		#define KG_API __declspec(dllexport)	
	#else
		#define KG_API __declspec(dllimport)
	#endif
#else
	#error Kargono currently only supports Windows
#endif

#define BIT(x) (1 << x)