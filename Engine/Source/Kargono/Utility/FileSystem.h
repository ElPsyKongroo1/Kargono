#pragma once

#include "Kargono/Core/Buffer.h"
#include "Kargono/Core/FixedString.h"

#include <filesystem>
#include <vector>
#include <string>


namespace Kargono::Utility
{
	//==============================
	// FileSystem Type Definitions
	//==============================
	enum class FileTypes
	{
		None = 0, png, bmp
	};

	//==============================
	// Interact with FileSystem Class
	//==============================
	class FileSystem
	{
	public:
		
		//==============================
		// Transfer Data To/From Disk
		//==============================
		static bool WriteFileBinary(const std::filesystem::path& filepath, Buffer buffer) noexcept;
		static bool WriteFileBinary(const std::filesystem::path& filepath, std::vector<Buffer>& buffers) noexcept;
		static bool WriteFileString(const std::filesystem::path& filepath, const std::string& string) noexcept;
		static bool WriteFileImage(const std::filesystem::path& filepath, uint8_t* buffer, uint32_t width, uint32_t height, FileTypes fileType) noexcept;
		static Buffer ReadFileBinary(const std::filesystem::path& filepath) noexcept;
		static std::string ReadFileString(const std::filesystem::path& filepath) noexcept;

		//==============================
		// Query Files/Directories
		//==============================
		static bool HasFileExtension(const std::filesystem::path& path) noexcept;
		static bool PathExists(const std::filesystem::path& path) noexcept;
		static bool IsRegularFile(const std::filesystem::path& path) noexcept;
		static std::filesystem::path GetAbsolutePath(const std::filesystem::path& path) noexcept;
		static bool DoesPathContainSubPath(const std::filesystem::path& base, const std::filesystem::path& full) noexcept;

		//==============================
		// Manage Files/Directories
		//==============================
		static bool RenameFile(const std::filesystem::path& oldPath, std::string newName) noexcept;
		static bool CopySingleFile(const std::filesystem::path& sourceFile, const std::filesystem::path& destinationFile) noexcept;
		static bool DeleteSelectedFile(const std::filesystem::path& filepath) noexcept;
		static bool DeleteSelectedDirectory(const std::filesystem::path& filepath) noexcept;
		static bool MoveFileToDirectory(const std::filesystem::path& filepath, const std::filesystem::path& newDirectory) noexcept;
		static bool CreateNewDirectory(const std::filesystem::path& filepath) noexcept;
		static bool CopyDirectory(const std::filesystem::path& sourceDirectory, const std::filesystem::path& destinationDirectory) noexcept;
		static std::filesystem::path FindFileWithExtension(const std::filesystem::path& directory, const std::string& extension) noexcept;
		static std::filesystem::path ConvertToUnixStylePath(const std::filesystem::path& path) noexcept;
		static std::filesystem::path GetRelativePath(const std::filesystem::path& base, const std::filesystem::path& full) noexcept;

		//==============================
		// Hashing API
		//==============================
		static std::string ChecksumFromFile(const std::filesystem::path& filepath);
		static std::string ChecksumFromString(const char* inputString);
		static std::string ChecksumFromBuffer(Buffer buffer);
		static uint32_t CRCFromBuffer(void* bufferPointer, uint64_t bufferSize);
		static uint32_t CRCFromString(const char* inputString);

	};
}
