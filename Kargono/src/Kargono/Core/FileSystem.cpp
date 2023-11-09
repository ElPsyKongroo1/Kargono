#include "kgpch.h"

#include "Kargono/Core/FileSystem.h"

#include <sha256.h>
#include "stb_image_write.h"

namespace Kargono
{
	void FileSystem::RenameFile(const std::filesystem::path& oldPath, std::string newName)
	{
		std::filesystem::path newPath = oldPath.parent_path() / newName;
		
		std::filesystem::rename(oldPath, newPath);
	}
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
	bool FileSystem::WriteFileBinary(const std::filesystem::path& filepath, Buffer buffer)
	{
		CreateNewDirectory(filepath.parent_path());
		std::ofstream output_file(filepath, std::ios::binary);
		if (!output_file)
		{
			KG_CORE_ERROR("Failed to write binary data to file");
			return false;
		}

		output_file.write(buffer.As<const char>(), buffer.Size);
		return true;
	}
	bool FileSystem::WriteFileBinary(const std::filesystem::path& filepath, ScopedBuffer buffer)
	{
		CreateNewDirectory(filepath.parent_path());
		std::ofstream output_file(filepath, std::ios::binary);
		if (!output_file)
		{
			KG_CORE_ERROR("Failed to write binary data to file");
			return false;
		}

		output_file.write(buffer.As<const char>(), buffer.Size());
		return true;
	}

	bool FileSystem::WriteFileString(const std::filesystem::path& filepath, std::string& string)
	{
		CreateNewDirectory(filepath.parent_path());
		std::ofstream output_file(filepath);
		if (!output_file)
		{
			KG_CORE_ERROR("Failed to write binary data to file");
			return false;
		}
		output_file << string;
		output_file.close();
		return true;
	}

	bool FileSystem::WriteFileImage(const std::filesystem::path& filepath, uint8_t* buffer, uint32_t width, uint32_t height, FileSystem::FileTypes fileType)
	{
		uint32_t channels{ 0 };
		std::filesystem::path outputPath = filepath;

		switch (fileType)
		{
		case FileSystem::FileTypes::png:
			{
			channels = 4;
			outputPath.replace_extension(".png");
			stbi_write_png(outputPath.string().c_str(), width, height, channels, buffer, width * channels);
			return true;
			}
		case FileSystem::FileTypes::bmp:
		{
			channels = 1;
			outputPath.replace_extension(".bmp");
			stbi_write_bmp(outputPath.string().c_str(), width, height, channels, buffer);
			return true;
		}
		}
		KG_CORE_ASSERT(false, "Invalid FileType enum provided to WriteFileImage function");
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
				KG_CORE_ERROR("Could not read from file '{0}'", filepath);
			}
		}
		else
		{
			KG_CORE_ERROR("Could not open file '{0}'", filepath);
		}

		return result;
	}


	std::filesystem::path FileSystem::GetRelativePath(const std::filesystem::path& base,
	                                                  const std::filesystem::path& full)
	{
		// Ensure full path starts with the base path.
		KG_CORE_ASSERT(!(full.root_path() != base.root_path()) && std::equal(base.begin(), base.end(), full.begin()), "Get Relative Path Failed. Base is not a subpath of full!")

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
			KG_CORE_ERROR("Failed to generate checksum from file");
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

	void FileSystem::CreateNewDirectory(const std::filesystem::path& filepath)
	{
		if (!std::filesystem::exists(filepath))
		{
			std::filesystem::create_directories(filepath);
		}
	}

}
