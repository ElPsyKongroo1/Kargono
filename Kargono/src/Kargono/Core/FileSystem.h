#pragma once

#include "Kargono/Core/Buffer.h"

#include <filesystem>

namespace Kargono
{
	class FileSystem
	{
	public:
		static Buffer ReadFileBinary(const std::filesystem::path& filepath);

		static std::filesystem::path GetRelativePath(const std::filesystem::path& base, const std::filesystem::path& full);
	};
}
