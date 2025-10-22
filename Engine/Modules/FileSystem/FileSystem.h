#pragma once

#include "Kargono/Core/Buffer.h"
#include "Kargono/Core/FixedBufferString.h"
#include "Kargono/Utility/Operations.h"

#include "API/Cryptography/hashlibraryAPI.h"

#include <filesystem>
#include <vector>
#include <string>


namespace Kargono::Utility
{
	enum class FileTypes
	{
		None = 0, png, bmp
	};

	using SHA256Hash = FixedBufferString<65>; // 64 chars + null terminator

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
		static bool PathsEquivalent(const std::filesystem::path& filePath, const std::filesystem::path& otherPath) noexcept;
		static bool HasFileExtension(const std::filesystem::path& path) noexcept;
		static bool HasFileName(const std::filesystem::path& path) noexcept;
		static bool PathExists(const std::filesystem::path& path) noexcept;
		static bool IsRegularFile(const std::filesystem::path& path) noexcept;
		static bool IsDirectory(const std::filesystem::path& path) noexcept; 
		static std::filesystem::path GetPathDirectory(const std::filesystem::path& path) noexcept;
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
		static SHA256Hash SHA256HashFromFile(const std::filesystem::path& filepath);
		static SHA256Hash SHA256HashFromString(const char* inputString);
		static SHA256Hash SHA256HashFromBuffer(Buffer buffer);
		static uint32_t CRCFromBuffer(void* bufferPointer, uint64_t bufferSize);
		constexpr static uint32_t CRCFromString(const char* inputString)
		{
			return Hashing::CalculateHash(inputString, Utility::Operations::GetStringLength(inputString));
		}

		constexpr static uint32_t CRCFromString(std::string_view inputString)
		{
			return Hashing::CalculateHash(inputString.data(), inputString.size());
		}

	};
}
