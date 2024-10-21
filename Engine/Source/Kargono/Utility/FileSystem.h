#pragma once

#include "Kargono/Core/Buffer.h"

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
		// Data Data To Disk
		//==============================
		static bool WriteFileBinary(const std::filesystem::path& filepath, Buffer buffer);
		static bool WriteFileBinary(const std::filesystem::path& filepath, std::vector<Buffer>& buffers);
		static bool WriteFileString(const std::filesystem::path& filepath, const std::string& string);
		static bool WriteFileImage(const std::filesystem::path& filepath, uint8_t* buffer, uint32_t width, uint32_t height, FileTypes fileType);
		//==============================
		// Read Data From Disk
		//==============================
		static Buffer ReadFileBinary(const std::filesystem::path& filepath);
		static std::string ReadFileString(const std::filesystem::path& filepath);
		//==============================
		// Manage Files/Directories
		//==============================
		static void RenameFile(const std::filesystem::path& oldPath, std::string newName);
		static bool CopySingleFile(const std::filesystem::path& sourceFile, const std::filesystem::path& destinationFile);
		static bool DeleteSelectedFile(const std::filesystem::path& filepath);
		static void DeleteSelectedDirectory(const std::filesystem::path& filepath);
		static void MoveFileToDirectory(const std::filesystem::path& filepath, const std::filesystem::path& newDirectory);
		static void CreateNewDirectory(const std::filesystem::path& filepath);
		static bool CopyDirectory(const std::filesystem::path& sourceDirectory, const std::filesystem::path& destinationDirectory);
		static std::filesystem::path FindFileWithExtension(const std::filesystem::path& directory, const std::string& extension);
		//==============================
		// Management Functions for std::filesystem::path
		//==============================
		static bool DoesPathContainSubPath(const std::filesystem::path& base, const std::filesystem::path& full);
		static std::filesystem::path GetRelativePath(const std::filesystem::path& base, const std::filesystem::path& full);

		//==============================
		// Create Checksum
		//==============================
		static std::string ChecksumFromFile(const std::filesystem::path& filepath);
		static std::string ChecksumFromString(const std::string& inputString);
		static std::string ChecksumFromBuffer(Buffer buffer);
		static uint32_t CRCFromBuffer(void* bufferPointer, uint64_t bufferSize);
		static uint32_t CRCFromString(const std::string& inputString);

	};
}
