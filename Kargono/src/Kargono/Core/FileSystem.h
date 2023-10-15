#pragma once

#include "Kargono/Core/Buffer.h"

#include <filesystem>
#include <string>

namespace Kargono
{
	class FileSystem
	{
	public:
		static Buffer ReadFileBinary(const std::filesystem::path& filepath);

		static std::filesystem::path GetRelativePath(const std::filesystem::path& base, const std::filesystem::path& full);

		static bool WriteFileBinary(const std::filesystem::path& filepath, Buffer buffer);

		static bool WriteFileString(const std::filesystem::path& filepath, std::string& string);

		static std::string ReadFileString(const std::filesystem::path& filepath);

		static std::string ChecksumFromFile(const std::filesystem::path& filepath);

		static std::string ChecksumFromString(const std::string& inputString);

		static std::string ChecksumFromBuffer(Buffer buffer);

		static void CreateNewDirectory(const std::filesystem::path& filepath);
	};
}
