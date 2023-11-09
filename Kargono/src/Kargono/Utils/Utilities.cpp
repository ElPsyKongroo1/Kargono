#include "kgpch.h"

#include "Utilities.h"

#include <codecvt>
#include <locale>

namespace Kargono::Utils
{
	std::wstring StringToWString(const std::string& string)
	{
		return std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(string);
	}
}


