#include "kgpch.h"

#include "Kargono/Utility/FileSystem.h"
#include "Kargono/Rendering/Texture.h"

#include "API/Cryptography/hashlibraryAPI.h"
#include "API/ImageProcessing/stbAPI.h"

namespace Kargono::Utility
{
	bool FileSystem::HasFileExtension(const std::filesystem::path& path)
	{
		return !path.extension().empty();
	}
	void FileSystem::RenameFile(const std::filesystem::path& oldPath, std::string newName)
	{
		std::filesystem::path newPath = oldPath.parent_path() / newName;
		
		std::filesystem::rename(oldPath, newPath);
	}
	bool FileSystem::CopySingleFile(const std::filesystem::path& sourceFile, const std::filesystem::path& destinationFile)
	{
		// Check if the sourceFile path exists and is a regular file
		if (!std::filesystem::exists(sourceFile) || !std::filesystem::is_regular_file(sourceFile)) 
		{
			KG_WARN("Failed to copy file. Either could not locate file or file is unfamiliar format!");
			return false;
		}

		// Create the destinationFile directory if it does not exist
		if (!std::filesystem::exists(destinationFile.parent_path())) 
		{
			std::filesystem::create_directories(destinationFile.parent_path());
		}

		// Copy the file to the destinationFile path
		std::filesystem::copy_file(sourceFile, destinationFile, std::filesystem::copy_options::overwrite_existing);

		return true;
	}
	Buffer FileSystem::ReadFileBinary(const std::filesystem::path& filepath)
	{
		std::ifstream stream(filepath, std::ios::binary | std::ios::ate);

		if (!stream)
		{
			// Failed to open the file
			KG_ERROR("Failed to open file in ReadFileBinary: {}", filepath.string());
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

	bool FileSystem::DeleteSelectedFile(const std::filesystem::path& filepath)
	{
		return std::filesystem::remove(filepath);
	}

	void FileSystem::DeleteSelectedDirectory(const std::filesystem::path& filepath)
	{
		std::filesystem::remove_all(filepath);
	}

	bool FileSystem::WriteFileBinary(const std::filesystem::path& filepath, Buffer buffer)
	{
		CreateNewDirectory(filepath.parent_path());
		std::ofstream output_file(filepath, std::ios::binary);
		if (!output_file)
		{
			KG_ERROR("Failed to write binary data to file");
			return false;
		}

		output_file.write(buffer.As<const char>(), buffer.Size);
		return true;
	}

	bool FileSystem::WriteFileBinary(const std::filesystem::path& filepath, std::vector<Buffer>& buffers)
	{
		CreateNewDirectory(filepath.parent_path());
		std::ofstream output_file(filepath, std::ios::binary);
		if (!output_file)
		{
			KG_ERROR("Failed to write binary data to file");
			return false;
		}
		for (auto& buffer : buffers)
		{
			output_file.write(buffer.As<const char>(), buffer.Size);
		}
		return true;
	}

	bool FileSystem::WriteFileString(const std::filesystem::path& filepath, const std::string& string)
	{
		CreateNewDirectory(filepath.parent_path());
		std::ofstream output_file(filepath);
		if (!output_file)
		{
			KG_ERROR("Failed to write binary data to file");
			return false;
		}
		output_file << string;
		output_file.close();
		return true;
	}

	void FileSystem::MoveFileToDirectory(const std::filesystem::path& filepath, const std::filesystem::path& newDirectory)
	{
		// Ensure duplicate path is not provided
		if (!static_cast<bool>(filepath.compare(newDirectory))) { return; }
		// Copy File over
		std::error_code ec {};
		std::filesystem::copy(filepath, newDirectory, ec);

		// Delete file in original directory if copy was successful!
		if (!ec)
		{
			FileSystem::DeleteSelectedFile(filepath);
		}
	}

	bool FileSystem::WriteFileImage(const std::filesystem::path& filepath, uint8_t* buffer, uint32_t width, uint32_t height, FileTypes fileType)
	{
		uint32_t channels{ 0 };
		std::filesystem::path outputPath = filepath;

		switch (fileType)
		{
		case FileTypes::png:
			{
			channels = 4;
			outputPath.replace_extension(".png");
			stbi_write_png(outputPath.string().c_str(), width, height, channels, buffer, width * channels);
			return true;
			}
		case FileTypes::bmp:
		{
			channels = 1;
			outputPath.replace_extension(".bmp");
			stbi_write_bmp(outputPath.string().c_str(), width, height, channels, buffer);
			return true;
		}
		}
		KG_ERROR("Invalid FileType enum provided to WriteFileImage function");
		return false;
	}


	std::string FileSystem::ReadFileString(const std::filesystem::path& filepath)
	{
		std::string result;
		std::ifstream in(filepath, std::ios::in | std::ios::binary); // ifstream closes itself due to RAII
		if (in)
		{
			in.seekg(0, std::ios::end);
			size_t size = in.tellg();
			if (size != -1)
			{
				result.resize(size);
				in.seekg(0, std::ios::beg);
				in.read(&result[0], size);
			}
			else
			{
				KG_ERROR("Could not read from file '{0}'", filepath);
			}
		}
		else
		{
			KG_ERROR("Could not open file '{0}'", filepath);
		}

		return result;
	}


	bool FileSystem::DoesPathContainSubPath(const std::filesystem::path& base, const std::filesystem::path& full)
	{
		return !(full.root_path() != base.root_path()) && std::equal(base.begin(), base.end(), full.begin());
	}

	std::filesystem::path FileSystem::GetRelativePath(const std::filesystem::path& base,
	                                                  const std::filesystem::path& full)
	{
		// Ensure full path starts with the base path.
		KG_ASSERT(DoesPathContainSubPath(base, full), "Get Relative Path Failed. Base is not a subpath of full!")

		auto subPath = relative(full, base);
		return relative(full, base);
	}

	std::string FileSystem::ChecksumFromFile(const std::filesystem::path& filepath)
	{
		// Open the file to be read in binary mode
		std::ifstream file(filepath, std::ios::binary);

		// Error Checking input stream
		if (!file)
		{
			KG_ERROR("Failed to generate checksum from file");
			return {};
		}

		SHA256 sha256stream;
		
		// Read the file in chunks
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

	std::string FileSystem::ChecksumFromString(const std::string& inputString)
	{
		SHA256 sha256stream;

		// Add the input string to the SHA256 context
		sha256stream.add(inputString.c_str(), inputString.size());

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

	uint32_t FileSystem::CRCFromString(const std::string& inputString)
	{
		CRC32 crc;
		return crc.CalculateHash(inputString.data(), inputString.size()); // Note that string.size() works here assuming ascii character sizes
	}

	void FileSystem::CreateNewDirectory(const std::filesystem::path& filepath)
	{
		if (filepath == "")
		{
			return;
		}
		if (!std::filesystem::exists(filepath))
		{
			std::filesystem::create_directories(filepath);
		}
	}

	bool FileSystem::CopyDirectory(const std::filesystem::path& sourceDirectory, const std::filesystem::path& destinationDirectory)
	{
		// Check if the source path exists and is a directory
		if (!std::filesystem::exists(sourceDirectory) || !std::filesystem::is_directory(sourceDirectory)) 
		{
			KG_WARN("Failed to copy directory since source directory does not exist or is not a directory");
			return false;
		}

		// Create the destination directory if it does not exist
		if (!std::filesystem::exists(destinationDirectory)) 
		{
			std::filesystem::create_directories(destinationDirectory);
		}

		// Iterate through the source directory
		for (const auto& entry : std::filesystem::recursive_directory_iterator(sourceDirectory)) 
		{
			const auto& path = entry.path();
			auto relative_path = std::filesystem::relative(path, sourceDirectory);
			auto destination_path = destinationDirectory / relative_path;

			// Create directory or file
			if (std::filesystem::is_directory(path)) 
			{
				std::filesystem::create_directories(destination_path);
			}
			// Copy the file to the destination path
			else if (std::filesystem::is_regular_file(path)) 
			{
				std::filesystem::copy_file(path, destination_path, std::filesystem::copy_options::overwrite_existing);
			}
			else 
			{
				KG_WARN("Skipping entry that is neither a directory nor a regular file!");
			}
		}

		return true;
	}

	std::filesystem::path FileSystem::FindFileWithExtension(const std::filesystem::path& directory, const std::string& extension)
	{
		// Check if the provided path is a directory
		if (!std::filesystem::is_directory(directory)) 
		{
			KG_WARN("Invalid directory provided!");
			return {};
		}

		for (const auto& entry : std::filesystem::directory_iterator(directory)) 
		{
			// Check if the current entry is a regular file and has the specified extension
			if (entry.is_regular_file() && entry.path().extension() == extension) 
			{
				return entry.path();
			}
		}

		// Return an empty path if no file with the specified extension is found
		return {};
	}

	std::filesystem::path FileSystem::ConvertToUnixStylePath(const std::filesystem::path& path)
	{
		std::string result;
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
