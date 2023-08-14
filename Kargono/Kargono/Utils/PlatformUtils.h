#pragma once
#include <string>
#include <optional>

namespace Kargono
{
	class FileDialogs
	{
	public:
		// These return empty string if cancelled
		static std::optional<std::string> OpenFile(const char* filter);
		static std::optional<std::string> SaveFile(const char* filter);
	};
}
