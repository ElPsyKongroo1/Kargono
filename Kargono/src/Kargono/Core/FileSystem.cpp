#include "kgpch.h"

#include "Kargono/Core/FileSystem.h"

namespace Kargono
{
	Buffer FileSystem::ReadFileBinary(const std::filesystem::path& filepath)
	{
		std::ifstream stream(filepath, std::ios::binary | std::ios::ate);

		if (!stream)
		{
			// Failed to open the file
			KG_CORE_ERROR("Failed to open file in ReadFileBinary!");
			return {};
		}

		std::streampos end = stream.tellg();
		stream.seekg(0, std::ios::beg);
		uint64_t size = end - stream.tellg();

		if (size == 0)
		{
			// File is empty
			return {};
		}

		Buffer buffer(size);
		stream.read(buffer.As<char>(), size);
		stream.close();
		return buffer;
	}

	std::filesystem::path FileSystem::GetRelativePath(const std::filesystem::path& base,
	                                                  const std::filesystem::path& full)
	{
		// Ensure full path starts with the base path.
		KG_CORE_ASSERT(!(full.root_path() != base.root_path()) && std::equal(base.begin(), base.end(), full.begin()), "Get Relative Path Failed. Base is not a subpath of full!")

		auto subPath = relative(full, base);
		return relative(full, base);
	}
}
