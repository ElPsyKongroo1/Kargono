#pragma once

#include "Kargono/Core/Buffer.h"

#include <filesystem>
#include <vector>
#include <string>

namespace Kargono
{
	class FileSystem
	{
	public:

		static void RenameFile(const std::filesystem::path& oldPath, std::string newName);

		static Buffer ReadFileBinary(const std::filesystem::path& filepath);
		static void DeleteSelectedFile(const std::filesystem::path& filepath);
		static void DeleteSelectedDirectory(const std::filesystem::path& filepath);

		static bool DoesPathContainSubPath(const std::filesystem::path& base, const std::filesystem::path& full);
		static std::filesystem::path GetRelativePath(const std::filesystem::path& base, const std::filesystem::path& full);

		static bool WriteFileBinary(const std::filesystem::path& filepath, Buffer buffer);
		static bool WriteFileBinary(const std::filesystem::path& filepath, std::vector<Buffer>& buffers);
		static bool WriteFileBinary(const std::filesystem::path& filepath, ScopedBuffer buffer);

		static bool WriteFileString(const std::filesystem::path& filepath, std::string& string);

		static void MoveFileToDirectory(const std::filesystem::path& filepath, const std::filesystem::path& newDirectory);

		enum class FileTypes
		{
			None = 0, png, bmp
		};

		static bool WriteFileImage(const std::filesystem::path& filepath, uint8_t* buffer, uint32_t width, uint32_t height, FileSystem::FileTypes fileType);

		static std::string ReadFileString(const std::filesystem::path& filepath);

		static std::string ChecksumFromFile(const std::filesystem::path& filepath);

		static std::string ChecksumFromString(const std::string& inputString);

		static std::string ChecksumFromBuffer(Buffer buffer);

		static void CreateNewDirectory(const std::filesystem::path& filepath);
	};
}
