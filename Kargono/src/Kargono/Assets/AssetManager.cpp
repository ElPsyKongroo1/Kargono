#include "kgpch.h"

#include "Kargono/Assets/AssetManager.h"

#include "Kargono/Core/FileSystem.h"
#include "Kargono/Core/Timer.h"
#include "Kargono/Project/Project.h"
#include "Kargono/Renderer/Shader.h"
#include "API/Serialization/SerializationAPI.h"

#include "stb_image.h"
#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include <dr_wav.h>


namespace Kargono
{
#pragma once
#define OpenAL_ErrorCheck(message)\
{\
	ALenum error = alGetError();\
	if( error != AL_NO_ERROR)\
	{\
		KG_CORE_ERROR("OpenAL Error: {} with call for {}", error, #message);\
	}\
}

#define alec(FUNCTION_CALL)\
FUNCTION_CALL;\
OpenAL_ErrorCheck(FUNCTION_CALL)

	std::unordered_map<AssetHandle, Asset> AssetManager::s_TextureRegistry {};
	std::unordered_map<AssetHandle, Ref<Texture2D>> AssetManager::s_Textures {};

	void AssetManager::DeserializeTextureRegistry()
	{
		s_TextureRegistry.clear();
		KG_CORE_ASSERT(Project::GetActive(), "There is no currently loaded project to serialize from!");
		const auto& textureRegistryLocation = Project::GetAssetDirectory() / "Textures/Intermediates/TextureRegistry.kgreg";

		if (!std::filesystem::exists(textureRegistryLocation))
		{
			KG_CORE_ERROR("No .kgregistry file exists in project path!");
			return;
		}
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(textureRegistryLocation.string());
		}
		catch (YAML::ParserException e)
		{
			KG_CORE_ERROR("Failed to load .kgscene file '{0}'\n     {1}", textureRegistryLocation.string(), e.what());
			return;
		}

		if (!data["Registry"]) { return; }

		std::string registryName = data["Registry"].as<std::string>();
		KG_CORE_TRACE("Deserializing Texture Registry");

		auto assets = data["Assets"];
		if (assets)
		{
			for (auto asset : assets)
			{
				Asset newAsset{};
				newAsset.Handle = asset["AssetHandle"].as<uint64_t>();

				auto metadata = asset["MetaData"];

				newAsset.Data.CheckSum = metadata["CheckSum"].as<std::string>();
				newAsset.Data.IntermediateLocation = metadata["IntermediateLocation"].as<std::string>();
				newAsset.Data.Type = Assets::StringToAssetType(metadata["AssetType"].as<std::string>());
				if (newAsset.Data.Type == Assets::Texture)
				{
					Ref<TextureMetaData> texMetaData = CreateRef<TextureMetaData>();

					texMetaData->Height = metadata["TextureHeight"].as<int32_t>();
					texMetaData->Width = metadata["TextureWidth"].as<int32_t>();
					texMetaData->Channels = metadata["TextureChannels"].as<int32_t>();
					texMetaData->InitialFileLocation = metadata["InitialFileLocation"].as<std::string>();

					newAsset.Data.SpecificFileData = texMetaData;
				}
				s_TextureRegistry.insert({ newAsset.Handle, newAsset });

			}
		}
	}

	void AssetManager::SerializeTextureRegistry()
	{
		KG_CORE_ASSERT(Project::GetActive(), "There is no currently loaded project to serialize to!");
		const auto& textureRegistryLocation = Project::GetAssetDirectory() / "Textures/Intermediates/TextureRegistry.kgreg";
		YAML::Emitter out;

		out << YAML::BeginMap;
		out << YAML::Key << "Registry" << YAML::Value << "Untitled";
		out << YAML::Key << "Assets" << YAML::Value << YAML::BeginSeq;

		// Asset
		for (auto& [handle, asset] : s_TextureRegistry)
		{
			out << YAML::BeginMap; // Asset Map
			out << YAML::Key << "AssetHandle" << YAML::Value << static_cast<uint64_t>(handle);

			out << YAML::Key << "MetaData" << YAML::Value;
			out << YAML::BeginMap; // MetaData Map
			out << YAML::Key << "CheckSum" << YAML::Value << asset.Data.CheckSum;
			out << YAML::Key << "IntermediateLocation" << YAML::Value << asset.Data.IntermediateLocation.string();
			out << YAML::Key << "AssetType" << YAML::Value << Assets::AssetTypeToString(asset.Data.Type);

			if (asset.Data.Type == Assets::AssetType::Texture)
			{
				TextureMetaData* metadata = static_cast<TextureMetaData*>(asset.Data.SpecificFileData.get());
				out << YAML::Key << "TextureHeight" << YAML::Value << metadata->Height;
				out << YAML::Key << "TextureWidth" << YAML::Value << metadata->Width;
				out << YAML::Key << "TextureChannels" << YAML::Value << metadata->Channels;
				out << YAML::Key << "InitialFileLocation" << YAML::Value << metadata->InitialFileLocation.string();
			}

			out << YAML::EndMap; // MetaData Map

			out << YAML::EndMap; // Asset Map
		}

		out << YAML::EndSeq;
		out << YAML::EndMap;

		FileSystem::CreateNewDirectory(textureRegistryLocation.parent_path());

		std::ofstream fout(textureRegistryLocation);
		fout << out.c_str();
	}

	AssetHandle AssetManager::ImportNewTextureFromFile(const std::filesystem::path& filePath)
	{
		// Create Checksum
		std::string currentCheckSum = FileSystem::ChecksumFromFile(filePath);

		if (currentCheckSum.empty())
		{
			KG_CORE_ERROR("Failed to generate checksum from file!");
			return {};
		}

		// Compare currentChecksum to registered assets

		bool isAssetDuplicate = false;
		AssetHandle currentHandle{};
		for (const auto& [handle, asset] : s_TextureRegistry)
		{
			if (asset.Data.CheckSum == currentCheckSum)
			{
				isAssetDuplicate = true;
				currentHandle = handle;
				break;
			}
		}

		if (isAssetDuplicate)
		{
			KG_CORE_ERROR("THERE IS A DUPLICATE!");
			return currentHandle;
		}

		AssetHandle newHandle{};

		Asset newAsset{};
		newAsset.Handle = newHandle;

		CreateTextureIntermediateFromFile(filePath, newAsset);
		newAsset.Data.CheckSum = currentCheckSum;

		s_TextureRegistry.insert({ newHandle, newAsset }); // Update Registry Map in-memory
		SerializeTextureRegistry(); // Update Registry File on Disk

		s_Textures.insert({ newHandle, InstantiateTextureIntoMemory(newAsset) });

		return newHandle;

	}

	AssetHandle AssetManager::ImportNewTextureFromData(Buffer buffer, int32_t width, int32_t height, int32_t channels)
	{
		// Create Checksum
		std::string currentCheckSum = FileSystem::ChecksumFromBuffer(buffer);

		if (currentCheckSum.empty())
		{
			KG_CORE_ERROR("Failed to generate checksum from file!");
			return {};
		}

		// Compare currentChecksum to registered assets
		bool isAssetDuplicate = false;
		AssetHandle currentHandle{};
		for (const auto& [handle, asset] : s_TextureRegistry)
		{
			if (asset.Data.CheckSum == currentCheckSum)
			{
				isAssetDuplicate = true;
				currentHandle = handle;
				break;
			}
		}

		if (isAssetDuplicate)
		{
			//KG_CORE_ERROR("THERE IS A DUPLICATE!");
			return currentHandle;
		}

		AssetHandle newHandle{};

		Asset newAsset{};
		newAsset.Handle = newHandle;

		CreateTextureIntermediateFromBuffer(buffer, width, height, channels, newAsset);
		newAsset.Data.CheckSum = currentCheckSum;

		s_TextureRegistry.insert({ newHandle, newAsset }); // Update Registry Map in-memory
		SerializeTextureRegistry(); // Update Registry File on Disk

		s_Textures.insert({ newHandle, InstantiateTextureIntoMemory(newAsset) });

		return newHandle;

	}

	void AssetManager::CreateTextureIntermediateFromFile(const std::filesystem::path& filePath, Asset& newAsset)
	{
		// Create Texture Binary Intermediate
		int32_t width, height, channels;
		stbi_set_flip_vertically_on_load(1);
		Buffer buffer{};
		stbi_uc* data = nullptr;
		{
			data = stbi_load(filePath.string().c_str(), &width, &height, &channels, 0);
		}

		buffer.Allocate(static_cast<unsigned long long>(width) * height * channels * sizeof(uint8_t));
		buffer.Data = data;

		// Save Binary Intermediate into File
		std::string intermediatePath = "Textures/Intermediates/" + (std::string)newAsset.Handle + ".kgtexture";
		std::filesystem::path intermediateFullPath = Project::GetAssetDirectory() / intermediatePath;
		FileSystem::WriteFileBinary(intermediateFullPath, buffer);

		// Check that save was successful
		if (!data)
		{
			KG_CORE_ERROR("Failed to load data from file in texture importer!");
			buffer.Release();
			return;
		}

		// Load data into In-Memory Metadata object
		newAsset.Data.Type = Assets::AssetType::Texture;
		newAsset.Data.IntermediateLocation = intermediatePath;
		Ref<TextureMetaData> metadata = CreateRef<TextureMetaData>();
		metadata->Width = width;
		metadata->Height = height;
		metadata->Channels = channels;
		metadata->InitialFileLocation = FileSystem::GetRelativePath(Project::GetAssetDirectory(), filePath);
		newAsset.Data.SpecificFileData = metadata;

		buffer.Release();

	}

	void AssetManager::CreateTextureIntermediateFromBuffer(Buffer buffer, int32_t width, int32_t height, int32_t channels, Asset& newAsset)
	{
		// Save Binary Intermediate into File
		std::string intermediatePath = "Textures/Intermediates/" + (std::string)newAsset.Handle + ".kgtexture";
		std::filesystem::path intermediateFullPath = Project::GetAssetDirectory() / intermediatePath;
		FileSystem::WriteFileBinary(intermediateFullPath, buffer);

		// Load data into In-Memory Metadata object
		newAsset.Data.Type = Assets::AssetType::Texture;
		newAsset.Data.IntermediateLocation = intermediatePath;
		Ref<TextureMetaData> metadata = CreateRef<TextureMetaData>();
		metadata->Width = width;
		metadata->Height = height;
		metadata->Channels = channels;
		metadata->InitialFileLocation = "None";
		newAsset.Data.SpecificFileData = metadata;
	}

	Ref<Texture2D> AssetManager::InstantiateTextureIntoMemory(Asset& asset)
	{
		TextureMetaData metadata = *static_cast<TextureMetaData*>(asset.Data.SpecificFileData.get());
		Buffer currentResource{};
		currentResource = FileSystem::ReadFileBinary(Project::GetAssetDirectory() / asset.Data.IntermediateLocation);
		Ref<Texture2D> newTexture = Texture2D::Create(currentResource, metadata);

		currentResource.Release();
		return newTexture;
	}

	Ref<Texture2D> AssetManager::GetTexture(const AssetHandle& handle)
	{
		KG_CORE_ASSERT(Project::GetActive(), "There is no active project when retreiving texture!");

		if (s_Textures.contains(handle)) { return s_Textures[handle]; }

		if (s_TextureRegistry.contains(handle))
		{
			auto asset = s_TextureRegistry[handle];

			Ref<Texture2D> newTexture = InstantiateTextureIntoMemory(asset);
			s_Textures.insert({ asset.Handle, newTexture });
			return newTexture;
		}

		KG_CORE_ERROR("No texture is associated with provided handle!");
		return nullptr;

	}

	void AssetManager::ClearTextureRegistry()
	{
		s_TextureRegistry.clear();
		s_Textures.clear();
	}


	std::unordered_map<AssetHandle, Asset> AssetManager::s_ShaderRegistry {};
	std::unordered_map<AssetHandle, Ref<Shader>> AssetManager::s_Shaders {};

	void AssetManager::DeserializeShaderRegistry()
	{
		s_ShaderRegistry.clear();
		KG_CORE_ASSERT(Project::GetActive(), "There is no currently loaded project to serialize from!");
		const auto& shaderRegistryLocation = Project::GetAssetDirectory() / "Shaders/Intermediates/ShaderRegistry.kgreg";

		if (!std::filesystem::exists(shaderRegistryLocation))
		{
			KG_CORE_ERROR("No .kgregistry file exists in project path!");
			return;
		}
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(shaderRegistryLocation.string());
		}
		catch (YAML::ParserException e)
		{
			KG_CORE_ERROR("Failed to load .kgreg file '{0}'\n     {1}", shaderRegistryLocation.string(), e.what());
			return;
		}

		if (!data["Registry"]) { return; }

		std::string registryName = data["Registry"].as<std::string>();
		KG_CORE_TRACE("Deserializing Shader Registry");

		auto assets = data["Assets"];
		if (assets)
		{
			for (auto asset : assets)
			{
				Asset newAsset{};
				newAsset.Handle = asset["AssetHandle"].as<uint64_t>();

				auto metadata = asset["MetaData"];

				newAsset.Data.CheckSum = metadata["CheckSum"].as<std::string>();
				newAsset.Data.IntermediateLocation = metadata["IntermediateLocation"].as<std::string>();
				newAsset.Data.Type = Assets::StringToAssetType(metadata["AssetType"].as<std::string>());
				if (newAsset.Data.Type == Assets::Shader)
				{
					Ref<ShaderMetaData> shaderMetaData = CreateRef<ShaderMetaData>();

					// ShaderSpecification Section
					shaderMetaData->ShaderSpec.ColorInput = Shader::StringToColorInputType(metadata["ColorInputType"].as<std::string>());
					shaderMetaData->ShaderSpec.AddProjectionMatrix = metadata["AddProjectionMatrix"].as<bool>();
					shaderMetaData->ShaderSpec.AddEntityID = metadata["AddEntityID"].as<bool>();
					shaderMetaData->ShaderSpec.AddCircleShape = metadata["AddCircleShape"].as<bool>();
					shaderMetaData->ShaderSpec.TextureInput = Shader::StringToTextureInputType(metadata["TextureInput"].as<std::string>());
					shaderMetaData->ShaderSpec.DrawOutline = metadata["DrawOutline"].as<bool>();
					shaderMetaData->ShaderSpec.RenderType = Shape::StringToRenderingType(metadata["RenderType"].as<std::string>());

					KG_CORE_ASSERT(sizeof(uint8_t) * 20 == sizeof(Shader::ShaderSpecification), "Please Update Deserialization and Serialization. Incorrect size of input data in Shader Deserializer!")
					{
						// InputBufferLayout Section
						auto inputBufferLayout = metadata["InputBufferLayout"];
						auto elementList = inputBufferLayout["Elements"];
						for (const auto& element : elementList)
						{
							shaderMetaData->InputLayout.AddBufferElement(InputBufferElement(
								StringToInputDataType(element["Type"].as<std::string>()),
								element["Name"].as<std::string>()
							));
						}
					}
					{
						// InputBufferLayout Section
						auto uniformBufferList = metadata["UniformBufferList"];
						auto elementList = uniformBufferList["Elements"];
						for (const auto& element : elementList)
						{
							shaderMetaData->UniformList.AddBufferElement(UniformElement(
								StringToUniformDataType(element["Type"].as<std::string>()),
								element["Name"].as<std::string>()
							));
						}
					}
					newAsset.Data.SpecificFileData = shaderMetaData;
				}
				s_ShaderRegistry.insert({ newAsset.Handle, newAsset });

			}
		}
	}

	void AssetManager::SerializeShaderRegistry()
	{
		KG_CORE_ASSERT(Project::GetActive(), "There is no currently loaded project to serialize to!");
		const auto& shaderRegistryLocation = Project::GetAssetDirectory() / "Shaders/Intermediates/ShaderRegistry.kgreg";
		YAML::Emitter out;

		out << YAML::BeginMap;
		out << YAML::Key << "Registry" << YAML::Value << "Untitled";
		out << YAML::Key << "Assets" << YAML::Value << YAML::BeginSeq;

		// Asset
		for (auto& [handle, asset] : s_ShaderRegistry)
		{
			out << YAML::BeginMap; // Asset Map
			out << YAML::Key << "AssetHandle" << YAML::Value << static_cast<uint64_t>(handle);
			out << YAML::Key << "MetaData" << YAML::Value;
			out << YAML::BeginMap; // MetaData Map
			out << YAML::Key << "CheckSum" << YAML::Value << asset.Data.CheckSum;
			out << YAML::Key << "IntermediateLocation" << YAML::Value << asset.Data.IntermediateLocation.string();
			out << YAML::Key << "AssetType" << YAML::Value << Assets::AssetTypeToString(asset.Data.Type);

			if (asset.Data.Type == Assets::AssetType::Shader)
			{
				// ShaderSpecification Section
				ShaderMetaData* metadata = static_cast<ShaderMetaData*>(asset.Data.SpecificFileData.get());
				out << YAML::Key << "ColorInputType" << YAML::Value << Shader::ColorInputTypeToString(metadata->ShaderSpec.ColorInput);
				out << YAML::Key << "AddProjectionMatrix" << YAML::Value << metadata->ShaderSpec.AddProjectionMatrix;
				out << YAML::Key << "AddEntityID" << YAML::Value << metadata->ShaderSpec.AddEntityID;
				out << YAML::Key << "AddCircleShape" << YAML::Value << metadata->ShaderSpec.AddCircleShape;
				out << YAML::Key << "TextureInput" << YAML::Value << Shader::TextureInputTypeToString(metadata->ShaderSpec.TextureInput);
				out << YAML::Key << "DrawOutline" << YAML::Value << metadata->ShaderSpec.DrawOutline;
				out << YAML::Key << "RenderType" << YAML::Value << Shape::RenderingTypeToString(metadata->ShaderSpec.RenderType);

				// InputBufferLayout Section
				out << YAML::Key << "InputBufferLayout" << YAML::Value << YAML::BeginMap; // Input Buffer Layout Map
				out << YAML::Key << "Elements" << YAML::Value << YAML::BeginSeq;
				for (const auto& element : metadata->InputLayout.GetElements())
				{
					out << YAML::BeginMap; // Input Element Map
					out << YAML::Key << "Name" << YAML::Value << element.Name;
					out << YAML::Key << "Type" << YAML::Value << InputDataTypeToString(element.Type);
					out << YAML::EndMap; // Input Element Map
				}
				out << YAML::EndSeq;
				out << YAML::EndMap; // Input Buffer Layout Map

				// UniformBufferList Section
				out << YAML::Key << "UniformBufferList" << YAML::Value << YAML::BeginMap; // Uniform Buffer Layout Map
				out << YAML::Key << "Elements" << YAML::Value << YAML::BeginSeq;
				for (const auto& element : metadata->UniformList.GetElements())
				{
					out << YAML::BeginMap; // Uniform Element Map
					out << YAML::Key << "Name" << YAML::Value << element.Name;
					out << YAML::Key << "Type" << YAML::Value << UniformDataTypeToString(element.Type);
					out << YAML::EndMap; // Uniform Element Map
				}
				out << YAML::EndSeq;
				out << YAML::EndMap; // Uniform Buffer Layout Map
			}

			out << YAML::EndMap; // MetaData Map

			out << YAML::EndMap; // Asset Map
		}

		out << YAML::EndSeq;
		out << YAML::EndMap;

		FileSystem::CreateNewDirectory(shaderRegistryLocation.parent_path());

		std::ofstream fout(shaderRegistryLocation);
		fout << out.c_str();
	}

	AssetHandle AssetManager::CreateNewShader(const Shader::ShaderSpecification& shaderSpec)
	{
		// TODO: IN PROGRESS, AHHHHHHHHHHHHHHHHHHHHHH
		// Create Checksum

		auto [shaderSource, bufferLayout, uniformList] = Shader::BuildShader(shaderSpec);
		std::string currentCheckSum = FileSystem::ChecksumFromString(shaderSource);

		if (currentCheckSum.empty())
		{
			KG_CORE_ERROR("Failed to generate checksum from shaderSpec!");
			return {};
		}

		// Compare currentChecksum to registered shaders
		bool isAssetDuplicate = false;
		AssetHandle currentHandle{};
		for (const auto& [handle, asset] : s_ShaderRegistry)
		{
			if (asset.Data.CheckSum == currentCheckSum)
			{
				isAssetDuplicate = true;
				currentHandle = handle;
				break;
			}
		}

		// TODO: If Asset is not a duplicate but has a similar spec to another asset, replace that asset! (Delete old asset and use it's UUID)

		if (isAssetDuplicate)
		{

			return currentHandle;
		}
		// Create New Asset and Asset Handle
		AssetHandle newHandle{};

		Asset newAsset{};
		newAsset.Handle = newHandle;

		// Create intermediate and save intermediate to disk
		CreateShaderIntermediate(shaderSource, newAsset, shaderSpec, bufferLayout, uniformList);
		newAsset.Data.CheckSum = currentCheckSum;

		s_ShaderRegistry.insert({ newHandle, newAsset }); // Update Registry Map in-memory
		SerializeShaderRegistry(); // Update Registry File on Disk
		Ref<Shader> newShader = InstantiateShaderIntoMemory(newAsset);
		s_Shaders.insert({ newHandle, newShader });

		return newHandle;
	}

	Ref<Shader> AssetManager::GetShader(const AssetHandle& handle)
	{
		KG_CORE_ASSERT(Project::GetActive(), "There is no active project when retrieving shader!");

		if (s_Shaders.contains(handle)) { return s_Shaders[handle]; }

		if (s_ShaderRegistry.contains(handle))
		{
			auto asset = s_ShaderRegistry[handle];

			Ref<Shader> newShader = InstantiateShaderIntoMemory(asset);
			s_Shaders.insert({ asset.Handle, newShader });
			return newShader;
		}

		//KG_CORE_INFO("No Shader is associated with provided asset handle!");
		return nullptr;
	}

	std::tuple<AssetHandle, Ref<Shader>> AssetManager::GetShader(const Shader::ShaderSpecification& shaderSpec)
	{
		KG_CORE_ASSERT(Project::GetActive(), "There is no active project when retrieving shader!");

		for (const auto& [assetHandle, shaderRef] : s_Shaders)
		{
			if (shaderRef->GetSpecification() == shaderSpec)
			{
				return std::make_tuple(assetHandle, shaderRef);
			}
		}

		for (auto& [assetHandle, asset] : s_ShaderRegistry)
		{
			ShaderMetaData metadata = *static_cast<ShaderMetaData*>(asset.Data.SpecificFileData.get());
			if (metadata.ShaderSpec == shaderSpec)
			{
				Ref<Shader> newShader = InstantiateShaderIntoMemory(asset);
				s_Shaders.insert({ asset.Handle, newShader });
				return std::make_tuple(assetHandle, newShader);
			}
		}

		// Create New Shader if none exists already
		AssetHandle newShaderHandle = CreateNewShader(shaderSpec);
		return std::make_tuple(newShaderHandle, GetShader(newShaderHandle));

	}

	void AssetManager::ClearShaderRegistry()
	{
		s_ShaderRegistry.clear();
		s_Shaders.clear();
	}

	namespace Utils {

		static GLenum ShaderTypeFromString(const std::string& type)
		{
			if (type == "vertex")
				return GL_VERTEX_SHADER;
			if (type == "fragment" || type == "pixel")
				return GL_FRAGMENT_SHADER;

			KG_CORE_ASSERT(false, "Unknown shader type!");
			return 0;
		}

		static std::string ShaderTypeToString(GLenum stage)
		{
			switch (stage)
			{
			case GL_VERTEX_SHADER:   return "vertex";
			case GL_FRAGMENT_SHADER: return "fragment";
			}
			KG_CORE_ASSERT(false, "Invalid Shader Type!");
			return "";
		}

		static shaderc_shader_kind GLShaderStageToShaderC(GLenum stage)
		{
			switch (stage)
			{
			case GL_VERTEX_SHADER:   return shaderc_glsl_vertex_shader;
			case GL_FRAGMENT_SHADER: return shaderc_glsl_fragment_shader;
			}
			KG_CORE_ASSERT(false, "Invalid Shader Type!");
			return (shaderc_shader_kind)0;
		}

		static const char* GLShaderStageToString(GLenum stage)
		{
			switch (stage)
			{
			case GL_VERTEX_SHADER:   return "GL_VERTEX_SHADER";
			case GL_FRAGMENT_SHADER: return "GL_FRAGMENT_SHADER";
			}
			KG_CORE_ASSERT(false, "Invalid Shader Type!");
			return nullptr;
		}

		static const char* ShaderBinaryFileExtension(uint32_t stage)
		{
			switch (stage)
			{
			case GL_VERTEX_SHADER:    return ".kgshadervert";
			case GL_FRAGMENT_SHADER:  return ".kgshaderfrag";
			}
			KG_CORE_ASSERT(false);
			return "";
		}
	}

	static std::unordered_map<GLenum, std::string> PreProcess(const std::string& source)
	{
		std::unordered_map<GLenum, std::string> shaderSources;

		const char* typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		size_t pos = source.find(typeToken, 0); //Start of shader type declaration line
		while (pos != std::string::npos)
		{
			size_t eol = source.find_first_of("\r\n", pos); //End of shader type declaration line
			KG_CORE_ASSERT(eol != std::string::npos, "Syntax error");
			size_t begin = pos + typeTokenLength + 1; //Start of shader type name (after "#type " keyword)
			std::string type = source.substr(begin, eol - begin);
			KG_CORE_ASSERT(Utils::ShaderTypeFromString(type), "Invalid shader type specified");

			size_t nextLinePos = source.find_first_not_of("\r\n", eol); //Start of shader code after shader type declaration line
			KG_CORE_ASSERT(nextLinePos != std::string::npos, "Syntax error");
			pos = source.find(typeToken, nextLinePos); //Start of next shader type declaration line

			shaderSources[Utils::ShaderTypeFromString(type)] = (pos == std::string::npos) ? source.substr(nextLinePos) : source.substr(nextLinePos, pos - nextLinePos);
		}

		return shaderSources;
	}

	static void CompileBinaries(const AssetHandle& assetHandle, const std::unordered_map<GLenum, std::string>& shaderSources, std::unordered_map<GLenum, std::vector<uint32_t>>& openGLSPIRV)
	{
		GLuint program = glCreateProgram();

		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		options.SetTargetEnvironment(shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);
		const bool optimize = true;
		if (optimize)
			options.SetOptimizationLevel(shaderc_optimization_level_performance);

		auto& shaderData = openGLSPIRV;
		shaderData.clear();
		for (auto&& [stage, source] : shaderSources)
		{
			std::string debugName = "Shader ID: " + static_cast<std::string>(assetHandle) + ", Shader Type: " + Utils::GLShaderStageToString(stage);
			// Compile SPIRV
			shaderc::SpvCompilationResult module;
			try
			{
				module = compiler.CompileGlslToSpv(source, Utils::GLShaderStageToShaderC(stage), debugName.c_str(), options);
			}
			catch (std::exception e)
			{
				KG_CORE_ERROR("Exception thrown inside shaderc!");
			}
			if (module.GetCompilationStatus() != shaderc_compilation_status_success)
			{
				KG_CORE_ERROR(module.GetErrorMessage());
				KG_CORE_ASSERT(false);
			}
			// Add Newly Compiled Spirv to m_OpenGLSPIRV
			shaderData[stage] = std::vector<uint32_t>(module.cbegin(), module.cend());
		}
	}


	Ref<Shader> AssetManager::InstantiateShaderIntoMemory(Asset& asset)
	{
		ShaderMetaData metadata = *static_cast<ShaderMetaData*>(asset.Data.SpecificFileData.get());
		std::unordered_map<GLenum, std::vector<uint32_t>> openGLSPIRV;
		std::filesystem::path intermediatePath = Project::GetAssetDirectory() / asset.Data.IntermediateLocation;
		std::vector<std::string> stageTypes = { "vertex", "fragment" };

		for (const auto& stage : stageTypes)
		{
			std::filesystem::path fullPath = intermediatePath.string() + Utils::ShaderBinaryFileExtension(Utils::ShaderTypeFromString(stage));

			std::ifstream in(fullPath, std::ios::in | std::ios::binary);

			if (in.is_open())
			{
				in.seekg(0, std::ios::end);
				auto size = in.tellg();
				in.seekg(0, std::ios::beg);

				auto& data = openGLSPIRV[Utils::ShaderTypeFromString(stage)];
				data.resize(size / sizeof(uint32_t));
				in.read((char*)data.data(), size);
			}
		}

		Ref<Shader> newShader = Shader::Create(static_cast<std::string>(asset.Handle), openGLSPIRV);
		newShader->SetSpecification(metadata.ShaderSpec);
		newShader->SetInputLayout(metadata.InputLayout);
		newShader->SetUniformList(metadata.UniformList);
		openGLSPIRV.clear();
		return newShader;
	}


	void AssetManager::CreateShaderIntermediate(const Shader::ShaderSource& shaderSource, Asset& newAsset, const Shader::ShaderSpecification& shaderSpec,
		const InputBufferLayout& inputLayout, const UniformBufferList& uniformLayout)
	{
		// Create Shader Binary
		auto shaderSources = PreProcess(shaderSource);

		std::unordered_map<GLenum, std::vector<uint32_t>> openGLSPIRV;
		CompileBinaries(newAsset.Handle, shaderSources, openGLSPIRV);

		// Save binary intermediates for all shader stages!
		std::string intermediatePath = "Shaders/Intermediates/" + (std::string)newAsset.Handle;
		for (const auto& [stage, source] : openGLSPIRV)
		{
			std::string intermediatePathWithExtension = intermediatePath + Utils::ShaderBinaryFileExtension(stage);
			std::filesystem::path intermediateFullPath = Project::GetAssetDirectory() / intermediatePathWithExtension;

			FileSystem::CreateNewDirectory(intermediateFullPath.parent_path());
			std::ofstream out(intermediateFullPath, std::ios::out | std::ios::binary);
			if (out.is_open())
			{
				// TODO: Add Checking to ensure output was successful!
				auto& data = source;
				out.write((char*)data.data(), data.size() * sizeof(uint32_t));
				out.flush();
				out.close();
			}
		}

		// Debug Only
#ifdef KG_DEBUG
		std::string debugString = shaderSource;
		std::filesystem::path debugPath = Project::GetAssetDirectory() / (intermediatePath + ".source");
		FileSystem::WriteFileString(debugPath, debugString);
#endif

		// Load In-Memory Metadata Object
		newAsset.Data.Type = Assets::AssetType::Shader;
		newAsset.Data.IntermediateLocation = intermediatePath;
		Ref<ShaderMetaData> metadata = CreateRef<ShaderMetaData>();
		metadata->ShaderSpec = shaderSpec;
		metadata->InputLayout = inputLayout;
		metadata->UniformList = uniformLayout;
		newAsset.Data.SpecificFileData = metadata;
	}


	std::unordered_map<AssetHandle, Asset> AssetManager::s_AudioRegistry {};
	std::unordered_map<AssetHandle, Ref<AudioBuffer>> AssetManager::s_Audio {};

	void AssetManager::DeserializeAudioRegistry()
	{
		s_AudioRegistry.clear();
		KG_CORE_ASSERT(Project::GetActive(), "There is no currently loaded project to serialize from!");
		const auto& audioRegistryLocation = Project::GetAssetDirectory() / "Audio/Intermediates/AudioRegistry.kgreg";

		if (!std::filesystem::exists(audioRegistryLocation))
		{
			KG_CORE_ERROR("No .kgregistry file exists in project path!");
			return;
		}
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(audioRegistryLocation.string());
		}
		catch (YAML::ParserException e)
		{
			KG_CORE_ERROR("Failed to load .kgscene file '{0}'\n     {1}", audioRegistryLocation.string(), e.what());
			return;
		}

		if (!data["Registry"]) { return; }

		std::string registryName = data["Registry"].as<std::string>();
		KG_CORE_TRACE("Deserializing Audio Registry");

		auto assets = data["Assets"];
		if (assets)
		{
			for (auto asset : assets)
			{
				Asset newAsset{};
				newAsset.Handle = asset["AssetHandle"].as<uint64_t>();

				auto metadata = asset["MetaData"];

				newAsset.Data.CheckSum = metadata["CheckSum"].as<std::string>();
				newAsset.Data.IntermediateLocation = metadata["IntermediateLocation"].as<std::string>();
				newAsset.Data.Type = Assets::StringToAssetType(metadata["AssetType"].as<std::string>());
				if (newAsset.Data.Type == Assets::Audio)
				{
					Ref<AudioMetaData> audioMetaData = CreateRef<AudioMetaData>();

					audioMetaData->Channels = metadata["Channels"].as<uint32_t>();
					audioMetaData->SampleRate = metadata["SampleRate"].as<uint32_t>();
					audioMetaData->TotalPcmFrameCount = metadata["TotalPcmFrameCount"].as<uint64_t>();
					audioMetaData->TotalSize = metadata["TotalSize"].as<uint64_t>();
					audioMetaData->InitialFileLocation = metadata["InitialFileLocation"].as<std::string>();

					newAsset.Data.SpecificFileData = audioMetaData;
				}
				s_AudioRegistry.insert({ newAsset.Handle, newAsset });

			}
		}
	}

	void AssetManager::SerializeAudioRegistry()
	{
		KG_CORE_ASSERT(Project::GetActive(), "There is no currently loaded project to serialize to!");
		const auto& audioRegistryLocation = Project::GetAssetDirectory() / "Audio/Intermediates/AudioRegistry.kgreg";
		YAML::Emitter out;

		out << YAML::BeginMap;
		out << YAML::Key << "Registry" << YAML::Value << "Audio";
		out << YAML::Key << "Assets" << YAML::Value << YAML::BeginSeq;

		// Asset
		for (auto& [handle, asset] : s_AudioRegistry)
		{
			out << YAML::BeginMap; // Asset Map
			out << YAML::Key << "AssetHandle" << YAML::Value << static_cast<uint64_t>(handle);

			out << YAML::Key << "MetaData" << YAML::Value;
			out << YAML::BeginMap; // MetaData Map
			out << YAML::Key << "CheckSum" << YAML::Value << asset.Data.CheckSum;
			out << YAML::Key << "IntermediateLocation" << YAML::Value << asset.Data.IntermediateLocation.string();
			out << YAML::Key << "AssetType" << YAML::Value << Assets::AssetTypeToString(asset.Data.Type);

			if (asset.Data.Type == Assets::AssetType::Audio)
			{
				AudioMetaData* metadata = static_cast<AudioMetaData*>(asset.Data.SpecificFileData.get());
				out << YAML::Key << "Channels" << YAML::Value << metadata->Channels;
				out << YAML::Key << "SampleRate" << YAML::Value << metadata->SampleRate;
				out << YAML::Key << "TotalPcmFrameCount" << YAML::Value << metadata->TotalPcmFrameCount;
				out << YAML::Key << "TotalSize" << YAML::Value << metadata->TotalSize;
				out << YAML::Key << "InitialFileLocation" << YAML::Value << metadata->InitialFileLocation.string();
			}

			out << YAML::EndMap; // MetaData Map
			out << YAML::EndMap; // Asset Map
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;

		FileSystem::CreateNewDirectory(audioRegistryLocation.parent_path());

		std::ofstream fout(audioRegistryLocation);
		fout << out.c_str();
	}

	AssetHandle AssetManager::ImportNewAudioFromFile(const std::filesystem::path& filePath)
	{
		// Create Checksum
		const std::string currentCheckSum = FileSystem::ChecksumFromFile(filePath);

		if (currentCheckSum.empty())
		{
			KG_CORE_ERROR("Failed to generate checksum from file!");
			return {};
		}

		for (const auto& [handle, asset] : s_AudioRegistry)
		{
			if (asset.Data.CheckSum == currentCheckSum)
			{
				KG_CORE_INFO("Attempt to instantiate duplicate audio asset");
				return handle;
			}
		}

		AssetHandle newHandle{};

		Asset newAsset{};
		newAsset.Handle = newHandle;

		CreateAudioIntermediateFromFile(filePath, newAsset);
		newAsset.Data.CheckSum = currentCheckSum;

		s_AudioRegistry.insert({ newHandle, newAsset }); // Update Registry Map in-memory
		SerializeAudioRegistry(); // Update Registry File on Disk

		s_Audio.insert({ newHandle, InstantiateAudioIntoMemory(newAsset) });

		return newHandle;
	}

	Ref<AudioBuffer> AssetManager::InstantiateAudioIntoMemory(Asset& asset)
	{
		AudioMetaData metadata = *static_cast<AudioMetaData*>(asset.Data.SpecificFileData.get());
		Buffer currentResource{};
		currentResource = FileSystem::ReadFileBinary(Project::GetAssetDirectory() / asset.Data.IntermediateLocation);
		Ref<AudioBuffer> newAudio = CreateRef<AudioBuffer>();
		alec(alGenBuffers(1, &(newAudio->m_BufferID)));
		alec(alBufferData(newAudio->m_BufferID, metadata.Channels > 1 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16, currentResource.Data, static_cast<ALsizei>(currentResource.Size), metadata.SampleRate));

		currentResource.Release();
		return newAudio;
	}

	Ref<AudioBuffer> AssetManager::GetAudio(const AssetHandle& handle)
	{
		KG_CORE_ASSERT(Project::GetActive(), "There is no active project when retreiving audio!");

		if (s_Audio.contains(handle)) { return s_Audio[handle]; }

		if (s_AudioRegistry.contains(handle))
		{
			auto asset = s_AudioRegistry[handle];

			Ref<AudioBuffer> newAudio = InstantiateAudioIntoMemory(asset);
			s_Audio.insert({ asset.Handle, newAudio });
			return newAudio;
		}

		KG_CORE_ERROR("No audio is associated with provided handle!");
		return nullptr;
	}

	void AssetManager::ClearAudioRegistry()
	{
		for (auto& [key, audioBuffer] : s_Audio)
		{
			KG_CORE_ASSERT(audioBuffer.use_count() == 1, "Not all Audio Buffer References have been cleared!");
		}
		s_AudioRegistry.clear();
		s_Audio.clear();
	}

	void AssetManager::CreateAudioIntermediateFromFile(const std::filesystem::path& filePath, Asset& newAsset)
	{
		// Create Buffers
		uint32_t channels = 0;
		uint32_t sampleRate = 0;
		uint64_t totalPcmFrameCount = 0;
		uint64_t totalSize = 0;
		Buffer pcmData{};
		drwav_int16* pSampleData = drwav_open_file_and_read_pcm_frames_s16(filePath.string().c_str(), &channels, &sampleRate, &totalPcmFrameCount, nullptr);
		if (!pSampleData)
		{
			KG_CORE_ERROR("Failed to load audio file");
			drwav_free(pSampleData, nullptr);
			return;
		}
		totalSize = totalPcmFrameCount * channels * 2;
		if ((totalSize) > drwav_uint64(std::numeric_limits<size_t>::max()))
		{
			KG_CORE_ERROR("Too much data in file for 32bit addressed vector");
			drwav_free(pSampleData, nullptr);
			return;
		}
		pcmData.Allocate(size_t(totalSize));
		std::memcpy(pcmData.Data, pSampleData, pcmData.Size /*two bytes in s16*/);
		drwav_free(pSampleData, nullptr);

		// Save Binary Intermediate into File
		std::string intermediatePath = "Audio/Intermediates/" + (std::string)newAsset.Handle + ".kgaudio";
		std::filesystem::path intermediateFullPath = Project::GetAssetDirectory() / intermediatePath;
		FileSystem::WriteFileBinary(intermediateFullPath, pcmData);

		// Check that save was successful
		if (!pcmData)
		{
			KG_CORE_ERROR("Failed to load data from file in audio importer!");
			return;
		}

		// Load data into In-Memory Metadata object
		newAsset.Data.Type = Assets::AssetType::Audio;
		newAsset.Data.IntermediateLocation = intermediatePath;
		Ref<AudioMetaData> metadata = CreateRef<AudioMetaData>();
		metadata->Channels = channels;
		metadata->SampleRate = sampleRate;;
		metadata->TotalPcmFrameCount = totalPcmFrameCount;
		metadata->TotalSize = totalSize;
		metadata->InitialFileLocation = FileSystem::GetRelativePath(Project::GetAssetDirectory(), filePath);
		newAsset.Data.SpecificFileData = metadata;
		pcmData.Release();
	}

}
