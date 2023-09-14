#pragma once
#include "Kargono/Core/Buffer.h"

namespace Kargono
{
	class FileSystem
	{
	public:
		static Buffer ReadFileBinary(const std::filesystem::path& filepath);
	};
}
