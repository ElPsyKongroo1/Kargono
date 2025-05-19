#include "kgpch.h"

#include "Modules/FileSystem/FileSystem.h"
#include "Modules/Rendering/Texture.h"

#include "API/ImageProcessing/stbAPI.h"

namespace Kargono::Utility
{
	bool FileSystem::PathExists(const std::filesystem::path& path) noexcept
	{
		std::error_code ec;
		bool exists = std::filesystem::exists(path, ec);

		// Check for an error code
		if (ec)
		{
			KG_WARN("Error occured while checking the existence of a path: {}", ec.message());
			return false;
		}

		// Return result
		return exists;
	}
	bool FileSystem::IsRegularFile(const std::filesystem::path& path) noexcept
	{
		std::error_code ec;
		bool exists = std::filesystem::is_regular_file(path, ec);

		// Check for an error code
		if (ec)
		{
			KG_WARN("Error occured while checking the existence of a path: {}", ec.message());
			return false;
		}

		// Return result
		return exists;
	}

	bool FileSystem::IsDirectory(const std::filesystem::path& path) noexcept
	{
		std::error_code ec;
		bool isDirectory = std::filesystem::is_directory(path, ec);

		// Check for an error code
		if (ec)
		{
			KG_WARN("Error occured while checking the existence of a path: {}", ec.message());
			return false;
		}

		// Return result
		return isDirectory;
	}

	std::filesystem::path FileSystem::GetAbsolutePath(const std::filesystem::path& path) noexcept
	{
		std::error_code ec;
		std::filesystem::path returnPath = std::filesystem::absolute(path);

		// Check for an error code
		if (ec)
		{
			KG_WARN("Error occured while checking whether a path is absolute: {}", ec.message());
			return {};
		}

		// Return result
		return returnPath;
	}

	bool FileSystem::PathsEquivalent(const std::filesystem::path& filePath, const std::filesystem::path& otherPath) noexcept
	{
		std::error_code ec;
		bool success = std::filesystem::equivalent(filePath, otherPath, ec);

		if (ec)
		{
			KG_WARN("Error occured while checking if two files are equivalent: {}", ec.message());
			return false;
		}

		return success;
	}

	bool FileSystem::HasFileExtension(const std::filesystem::path& path) noexcept
	{
		return !path.extension().empty();
	}
	bool FileSystem::RenameFile(const std::filesystem::path& oldPath, std::string newName) noexcept
	{
		std::filesystem::path newPath = oldPath.parent_path() / newName;
		
		std::error_code ec;
		std::filesystem::rename(oldPath, newPath, ec);

		// Check for an error code
		if (ec)
		{
			KG_WARN("Error occured while renaming a file: {}", ec.message());
			return false;
		}

		return true;
	}
	bool FileSystem::CopySingleFile(const std::filesystem::path& sourceFile, const std::filesystem::path& destinationFile) noexcept
	{
		// Check if the source File path exists
		std::error_code ec;
		if (!std::filesystem::exists(sourceFile, ec))
		{
			KG_WARN("Failed to copy file. Source file does not exist!");
			return false;
		}

		if (ec)
		{
			KG_WARN("Error occurred while copying a file: {}", ec.message());
			return false;
		}

		// Ensure source file is regular
		if (!std::filesystem::is_regular_file(sourceFile, ec)) 
		{
			KG_WARN("Failed to copy file. File is an unfamiliar format!");
			return false;
		}

		if (ec)
		{
			KG_WARN("Error occurred while copying a file: {}", ec.message());
			return false;
		}

		// Create the destination File directory if it does not exist
		if (!std::filesystem::exists(destinationFile.parent_path(), ec)) 
		{
			if (ec)
			{
				KG_WARN("Error occurred while copying a file: {}", ec.message());
				return false;
			}

			std::filesystem::create_directories(destinationFile.parent_path(), ec);
		}

		if (ec)
		{
			KG_WARN("Error occurred while copying a file: {}", ec.message());
			return false;
		}

		// Copy the file to the destinationFile path
		std::filesystem::copy_file(sourceFile, destinationFile, std::filesystem::copy_options::overwrite_existing, ec);

		if (ec)
		{
			KG_WARN("Error occurred while copying a file: {}", ec.message());
			return false;
		}

		return true;
	}
	Buffer FileSystem::ReadFileBinary(const std::filesystem::path& filepath) noexcept
	{
		std::ifstream stream(filepath, std::ios::binary | std::ios::ate);

		if (!stream)
		{
			// Failed to open the file
			KG_WARN("Failed to open file inputStream ReadFileBinary: {}", filepath.string());
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

	bool FileSystem::DeleteSelectedFile(const std::filesystem::path& filepath) noexcept
	{
		std::error_code ec;
		bool success = std::filesystem::remove(filepath, ec);

		if (ec)
		{
			KG_WARN("Error occurred while deleting a file: {}", ec.message());
			return false;
		}

		return success;
	}

	bool FileSystem::DeleteSelectedDirectory(const std::filesystem::path& filepath) noexcept
	{
		std::error_code ec;
		uintmax_t deleteCount = std::filesystem::remove_all(filepath, ec);

		if (ec)
		{
			KG_WARN("Error occurred while deleting a directory: {}", ec.message());
			return false;
		}

		return deleteCount > 0;

	}

	bool FileSystem::WriteFileBinary(const std::filesystem::path& filepath, Buffer buffer) noexcept
	{
		CreateNewDirectory(filepath.parent_path());
		std::ofstream output_file(filepath, std::ios::binary);
		if (!output_file)
		{
			KG_WARN("Failed to write binary data to file. Could not initialize output stream!");
			return false;
		}

		output_file.write(buffer.As<const char>(), buffer.Size);
		return true;
	}

	bool FileSystem::WriteFileBinary(const std::filesystem::path& filepath, std::vector<Buffer>& buffers) noexcept
	{
		CreateNewDirectory(filepath.parent_path());
		std::ofstream output_file(filepath, std::ios::binary);
		if (!output_file)
		{
			KG_WARN("Failed to write binary data to file");
			return false;
		}
		for (Buffer& buffer : buffers)
		{
			output_file.write(buffer.As<const char>(), buffer.Size);
		}
		return true;
	}

	bool FileSystem::WriteFileString(const std::filesystem::path& filepath, const std::string& string) noexcept
	{
		CreateNewDirectory(filepath.parent_path());
		std::ofstream output_file(filepath);
		if (!output_file)
		{
			KG_WARN("Failed to write binary data to file");
			return false;
		}
		output_file << string;
		output_file.close();
		return true;
	}

	bool FileSystem::MoveFileToDirectory(const std::filesystem::path& filepath, const std::filesystem::path& newDirectory) noexcept
	{
		// Ensure duplicate path is not provided
		if (!static_cast<bool>(filepath.compare(newDirectory))) 
		{
			KG_WARN("Failed to move file to directory. Filepath and directory are identical");
			return false; 
		}

		// Copy File over
		std::error_code ec {};
		std::filesystem::copy(filepath, newDirectory, ec);

		if (ec)
		{
			KG_WARN("Failed to move file to directory: {}", ec.message());
			return false;
		}

		// Delete file inputStream original directory if copy was successful!
		if (!FileSystem::DeleteSelectedFile(filepath))
		{
			KG_WARN("Failed to move file to directory. Could not delete original file");
			return false;
		}

		return true;
	}

	bool FileSystem::WriteFileImage(const std::filesystem::path& filepath, uint8_t* buffer, uint32_t width, uint32_t height, FileTypes fileType) noexcept
	{
		uint16_t channels{ 0 };
		std::filesystem::path outputPath{ filepath };

		int stbStatus{ 0 };
		switch (fileType)
		{
		case FileTypes::png:
			channels = 4;
			outputPath.replace_extension(".png");
			stbStatus = stbi_write_png(outputPath.string().c_str(), width, height, channels, buffer, width * channels);
			if (stbStatus == 0)
			{
				KG_WARN("Failed to write image to disk");
				return false;
			}
			return true;
		case FileTypes::bmp:
			channels = 1;
			outputPath.replace_extension(".bmp");
			stbStatus = stbi_write_bmp(outputPath.string().c_str(), width, height, channels, buffer);
			if (stbStatus == 0)
			{
				KG_WARN("Failed to write image to disk");
				return false;
			}
			return true;
		}

		KG_WARN("Invalid FileType enum provided to WriteFileImage function");
		return false;
	}


	std::string FileSystem::ReadFileString(const std::filesystem::path& filepath) noexcept
	{
		// ifstream closes itself due to RAII
		std::string result;
		std::ifstream inputStream(filepath, std::ios::in | std::ios::binary); 
		if (!inputStream)
		{
			KG_WARN("Could not open file '{0}'", filepath);
			return result;
		}
		
		inputStream.seekg(0, std::ios::end);
		size_t size = inputStream.tellg();

		if (size == -1)
		{
			KG_WARN("Could not read from file '{0}'", filepath);
			return result;
		}

		result.resize(size);
		inputStream.seekg(0, std::ios::beg);
		inputStream.read(&result[0], size);
		return result;
	}


	bool FileSystem::DoesPathContainSubPath(const std::filesystem::path& base, const std::filesystem::path& full) noexcept
	{
		return !(full.root_path() != base.root_path()) && std::equal(base.begin(), base.end(), full.begin());
	}

	std::filesystem::path FileSystem::GetRelativePath(const std::filesystem::path& base,
	                                                  const std::filesystem::path& full) noexcept
	{
		std::error_code ec;
		std::filesystem::path subPath;

		subPath = std::filesystem::relative(full, base, ec);

		if (ec)
		{
			KG_WARN("Failed to get relative path: {}", ec.message());
		}

		return subPath;
	}

	std::string FileSystem::ChecksumFromFile(const std::filesystem::path& filepath)
	{
		// Open the file to be read inputStream binary mode
		std::ifstream file(filepath, std::ios::binary);

		// Error Checking input stream
		if (!file)
		{
			KG_ERROR("Failed to generate checksum from file");
			return {};
		}

		SHA256 sha256stream;
		
		// Read the file inputStream chunks
		std::vector<char> buffer(4096);
		std::streamsize bytesRead = 0;

		while (file.read(buffer.data(), buffer.size()))
		{
			bytesRead = file.gcount();
			// Update the SHA256 context with the buffer content
			sha256stream.add(buffer.data(), bytesRead);
		}

		return sha256stream.getHash();
	}

	std::string FileSystem::ChecksumFromString(const char* inputString)
	{
		SHA256 sha256stream;

		// Add the input string to the SHA256 context
		sha256stream.add(inputString, std::strlen(inputString));

		return sha256stream.getHash();
	}

	std::string FileSystem::ChecksumFromBuffer(Buffer buffer)
	{
		SHA256 sha256stream;

		// Add the input string to the SHA256 context
		sha256stream.add(buffer.As<char>(), buffer.Size);

		return sha256stream.getHash();
	}

	uint32_t FileSystem::CRCFromBuffer(void* bufferPointer, uint64_t bufferSize)
	{
		CRC32 crc;
		return crc.CalculateHash(bufferPointer, bufferSize);
	}
	

	bool FileSystem::CreateNewDirectory(const std::filesystem::path& filepath) noexcept
	{
		std::error_code ec;
		if (std::filesystem::exists(filepath, ec))
		{
			return true;
		}

		if (ec)
		{
			KG_WARN("Failed to create new directory: {}", ec.message());
			return false;
		}

		bool success = std::filesystem::create_directories(filepath, ec);

		if (ec)
		{
			KG_WARN("Failed to create new directory: {}", ec.message());
			return false;
		}

		return success;
		
	}

	bool FileSystem::CopyDirectory(const std::filesystem::path& sourceDirectory, const std::filesystem::path& destinationDirectory) noexcept
	{
		// Check if the source path exists and is a directory
		std::error_code ec;
		if (!std::filesystem::exists(sourceDirectory, ec) || !std::filesystem::is_directory(sourceDirectory, ec)) 
		{
			KG_WARN("Failed to copy directory since source directory does not exist or is not a directory");
			return false;
		}

		if (ec)
		{
			KG_WARN("Failed to copy directory: {}", ec.message());
			return false;
		}

		// Create the destination directory if it does not exist
		if (!std::filesystem::exists(destinationDirectory)) 
		{
			std::filesystem::create_directories(destinationDirectory);
		}

		if (ec)
		{
			KG_WARN("Failed to copy directory: {}", ec.message());
			return false;
		}

		// Iterate through the source directory
		for (const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(sourceDirectory))
		{
			const std::filesystem::path& path = entry.path();
			std::filesystem::path relative_path = std::filesystem::relative(path, sourceDirectory);
			std::filesystem::path destination_path = destinationDirectory / relative_path;

			// Create directory or file
			if (std::filesystem::is_directory(path, ec)) 
			{
				std::filesystem::create_directories(destination_path, ec);
			}
			// Copy the file to the destination path
			else if (std::filesystem::is_regular_file(path, ec)) 
			{
				std::filesystem::copy_file(path, destination_path, std::filesystem::copy_options::overwrite_existing, ec);
			}
			else 
			{
				KG_WARN("Skipping entry that is neither a directory nor a regular file!");
			}

			if (ec)
			{
				KG_WARN("Failed to copy directory: {}", ec.message());
				return false;
			}
		}

		return true;
	}

	std::filesystem::path FileSystem::FindFileWithExtension(const std::filesystem::path& directory, const std::string& extension) noexcept
	{
		// Check if the provided path is a directory
		std::error_code ec;
		if (!std::filesystem::is_directory(directory, ec)) 
		{
			KG_WARN("Invalid directory provided!");
			return {};
		}

		if (ec)
		{
			KG_WARN("Failed to find file inside directory w/ extension: {}", ec.message());
			return {};
		}

		// Search directory for file w/ matching extension
		for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(directory)) 
		{
			if (entry.is_regular_file(ec) && entry.path().extension() == extension) 
			{
				return entry.path();
			}
			
			if (ec)
			{
				KG_WARN("File inside directory has issues: {}", ec.message());
				return {};
			}
		}

		// Return an empty path if no file with the specified extension is found
		return {};
	}

	std::filesystem::path FileSystem::ConvertToUnixStylePath(const std::filesystem::path& path) noexcept
	{
		std::string result;
		result.reserve(path.native().size());
		for (const std::filesystem::path& pathSegment : path) 
		{
			if (!result.empty()) 
			{
				result += '/';
			}
			result += pathSegment.string();
		}

		// Return modified path
		return result;
	}
}
