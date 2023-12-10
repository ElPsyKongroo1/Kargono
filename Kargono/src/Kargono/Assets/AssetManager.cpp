#include "kgpch.h"

#include "Kargono/Assets/AssetManager.h"

#include "Kargono/Core/FileSystem.h"
#include "Kargono/Core/Timer.h"
#include "Kargono/Project/Project.h"
#include "Kargono/Renderer/Shader.h"
#include "Kargono/Scene/Scene.h"
#include "API/Serialization/SerializationAPI.h"
#include "Kargono/Scripting/ScriptEngine.h"
#include "Kargono/Scene/Entity.h"
#include "Kargono/Scene/Components.h"


#include "stb_image.h"
#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include <dr_wav.h>
#include "ft2build.h"
#include FT_FREETYPE_H
#include "glad/glad.h"
#include <glm/gtc/matrix_transform.hpp>



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

	std::tuple<AssetHandle, Ref<AudioBuffer>> AssetManager::GetAudio(const std::filesystem::path& filepath)
	{
		KG_CORE_ASSERT(Project::GetActive(), "Attempt to use Project Field without active project!");

		for (auto& [assetHandle, asset] : s_AudioRegistry)
		{
			auto metadata = (AudioMetaData*)asset.Data.SpecificFileData.get();
			if (metadata->InitialFileLocation.compare(filepath) == 0)
			{
				return std::make_tuple(assetHandle, GetAudio(assetHandle));
			}
		}
		// Return empty audio if audio does not exist
		KG_CORE_TRACE("Invalid filepath provided to GetAudio {}", filepath.string());
		return std::make_tuple(0, nullptr);
	}

	void AssetManager::ClearAudioRegistry()
	{
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

	std::unordered_map<AssetHandle, Asset> AssetManager::s_FontRegistry {};
	std::unordered_map<AssetHandle, Ref<Font>> AssetManager::s_Fonts {};

	void AssetManager::DeserializeFontRegistry()
	{
		s_FontRegistry.clear();
		KG_CORE_ASSERT(Project::GetActive(), "There is no currently loaded project to serialize from!");
		const auto& fontRegistryLocation = Project::GetAssetDirectory() / "Fonts/Intermediates/FontRegistry.kgreg";

		if (!std::filesystem::exists(fontRegistryLocation))
		{
			KG_CORE_ERROR("No .kgregistry file exists in project path!");
			return;
		}
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(fontRegistryLocation.string());
		}
		catch (YAML::ParserException e)
		{
			KG_CORE_ERROR("Failed to load .kgscene file '{0}'\n     {1}", fontRegistryLocation.string(), e.what());
			return;
		}

		if (!data["Registry"]) { return; }

		std::string registryName = data["Registry"].as<std::string>();
		KG_CORE_TRACE("Deserializing Font Registry");

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
				if (newAsset.Data.Type == Assets::Font)
				{
					Ref<FontMetaData> fontMetaData = CreateRef<FontMetaData>();

					fontMetaData->AverageWidth = metadata["AverageWidth"].as<float>();
					fontMetaData->AverageHeight = metadata["AverageHeight"].as<float>();

					auto bufferLocations = metadata["BufferLocations"];
					fontMetaData->BufferLocations = std::vector<uint64_t>();
					auto& bufferLocationVector = fontMetaData->BufferLocations;
					for (YAML::const_iterator it = bufferLocations.begin(); it != bufferLocations.end(); ++it)
					{
						bufferLocationVector.push_back(it->as<uint64_t>());
					}

					auto characters = metadata["Characters"];
					auto& characterVector = fontMetaData->Characters;
					for (auto character : characters)
					{
						Font::Character newCharacter{};
						newCharacter.Texture = nullptr;
						newCharacter.Size = character["Size"].as<glm::ivec2>();
						newCharacter.Bearing = character["Bearing"].as<glm::ivec2>();
						newCharacter.Advance = character["Advance"].as<uint32_t>();
						characterVector.push_back( std::pair<unsigned char, Font::Character>(static_cast<uint8_t>(character["Character"].as<uint32_t>()), newCharacter));
					}

					fontMetaData->InitialFileLocation = metadata["InitialFileLocation"].as<std::string>();
					newAsset.Data.SpecificFileData = fontMetaData;

				}
				s_FontRegistry.insert({ newAsset.Handle, newAsset });

			}
		}
	}

	void AssetManager::SerializeFontRegistry()
	{
		KG_CORE_ASSERT(Project::GetActive(), "There is no currently loaded project to serialize to!");
		const auto& fontRegistryLocation = Project::GetAssetDirectory() / "Fonts/Intermediates/FontRegistry.kgreg";
		YAML::Emitter out;

		out << YAML::BeginMap;
		out << YAML::Key << "Registry" << YAML::Value << "Font";
		out << YAML::Key << "Assets" << YAML::Value << YAML::BeginSeq;

		// Asset
		for (auto& [handle, asset] : s_FontRegistry)
		{
			out << YAML::BeginMap; // Asset Map
			out << YAML::Key << "AssetHandle" << YAML::Value << static_cast<uint64_t>(handle);

			out << YAML::Key << "MetaData" << YAML::Value;
			out << YAML::BeginMap; // MetaData Map
			out << YAML::Key << "CheckSum" << YAML::Value << asset.Data.CheckSum;
			out << YAML::Key << "IntermediateLocation" << YAML::Value << asset.Data.IntermediateLocation.string();
			out << YAML::Key << "AssetType" << YAML::Value << Assets::AssetTypeToString(asset.Data.Type);

			if (asset.Data.Type == Assets::AssetType::Font)
			{
				FontMetaData* metadata = static_cast<FontMetaData*>(asset.Data.SpecificFileData.get());

				out << YAML::Key << "InitialFileLocation" << YAML::Value << metadata->InitialFileLocation.string();
				out << YAML::Key << "AverageWidth" << YAML::Value << metadata->AverageWidth;
				out << YAML::Key << "AverageHeight" << YAML::Value << metadata->AverageHeight;

				out << YAML::Key << "BufferLocations" << YAML::BeginSeq;
				for (auto& location : metadata->BufferLocations){out << location;}
				out << YAML::EndSeq;

				out << YAML::Key << "Characters" << YAML::Value << YAML::BeginSeq;
				for (auto& [character, characterStruct] : metadata->Characters)
				{
					out << YAML::BeginMap;
					out << YAML::Key << "Character" << YAML::Value << static_cast<uint32_t>(character);
					out << YAML::Key << "Size" << YAML::Value << characterStruct.Size;
					out << YAML::Key << "Bearing" << YAML::Value << characterStruct.Bearing;
					out << YAML::Key << "Advance" << YAML::Value << characterStruct.Advance;
					out << YAML::EndMap;
				}
				out << YAML::EndSeq;
			}

			out << YAML::EndMap; // MetaData Map
			out << YAML::EndMap; // Asset Map
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;

		FileSystem::CreateNewDirectory(fontRegistryLocation.parent_path());

		std::ofstream fout(fontRegistryLocation);
		fout << out.c_str();
	}

	AssetHandle AssetManager::ImportNewFontFromFile(const std::filesystem::path& filePath)
	{
		// Create Checksum
		const std::string currentCheckSum = FileSystem::ChecksumFromFile(filePath);

		if (currentCheckSum.empty())
		{
			KG_CORE_ERROR("Failed to generate checksum from file!");
			return {};
		}

		for (const auto& [handle, asset] : s_FontRegistry)
		{
			if (asset.Data.CheckSum == currentCheckSum)
			{
				KG_CORE_INFO("Attempt to instantiate duplicate font asset");
				return handle;
			}
		}

		AssetHandle newHandle{};

		Asset newAsset{};
		newAsset.Handle = newHandle;

		CreateFontIntermediateFromFile(filePath, newAsset);
		newAsset.Data.CheckSum = currentCheckSum;

		s_FontRegistry.insert({ newHandle, newAsset }); // Update Registry Map in-memory
		SerializeFontRegistry(); // Update Registry File on Disk

		s_Fonts.insert({ newHandle, InstantiateFontIntoMemory(newAsset) });

		return newHandle;
	}

	Ref<Font> AssetManager::InstantiateFontIntoMemory(Asset& asset)
	{
		FontMetaData metadata = *static_cast<FontMetaData*>(asset.Data.SpecificFileData.get());
		Buffer currentResource{};
		currentResource = FileSystem::ReadFileBinary(Project::GetAssetDirectory() / asset.Data.IntermediateLocation);
		Ref<Font> newFont = CreateRef<Font>();
		newFont->m_AverageWidth = metadata.AverageWidth;
		newFont->m_AverageHeight = metadata.AverageHeight;
		auto& fontCharacters = newFont->GetCharacters();

		auto& bufferLocations = metadata.BufferLocations;

		// disable byte-alignment restriction
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		uint64_t iterator{ 0 };
		for (auto& [character, characterStruct] : metadata.Characters)
		{
			// Generate Texture
			unsigned int texture;
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_RED,
				characterStruct.Size.x,
				characterStruct.Size.y,
				0,
				GL_RED,
				GL_UNSIGNED_BYTE,
				(currentResource.Data + bufferLocations.at(iterator))
			);
			// set texture options
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			iterator++;

			Font::Character mapCharacter = 
			{
				Texture2D::Create(texture, characterStruct.Size.x, characterStruct.Size.y),
				characterStruct.Size,
				characterStruct.Bearing,
				characterStruct.Advance
			};

			fontCharacters.insert(std::pair<unsigned char, Font::Character>(character, mapCharacter));
		}

		currentResource.Release();
		return newFont;
		
	}

	Ref<Font> AssetManager::GetFont(const AssetHandle& handle)
	{
		KG_CORE_ASSERT(Project::GetActive(), "There is no active project when retreiving font!");

		if (s_Fonts.contains(handle)) { return s_Fonts[handle]; }

		if (s_FontRegistry.contains(handle))
		{
			auto asset = s_FontRegistry[handle];

			Ref<Font> newFont = InstantiateFontIntoMemory(asset);
			s_Fonts.insert({ asset.Handle, newFont });
			return newFont;
		}

		KG_CORE_ERROR("No font is associated with provided handle!");
		return nullptr;
	}

	void AssetManager::ClearFontRegistry()
	{
		s_FontRegistry.clear();
		s_Fonts.clear();
	}

	void AssetManager::CreateFontIntermediateFromFile(const std::filesystem::path& filePath, Asset& newAsset)
	{
		// Create Buffers
		FT_Library ft;
		if (FT_Init_FreeType(&ft))
		{
			KG_CORE_ASSERT(false, "FreeType Library Not Initialized Properly");
			return;
		}

		FT_Face face;
		if (FT_New_Face(ft, filePath.string().c_str(), 0, &face))
		{
			KG_CORE_ERROR("Font not Loaded Correctly!");
			return;
		}

		FT_Set_Pixel_Sizes(face, 0, 48);

		// disable byte-alignment restriction
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		double widthSum{ 0 };
		double heightSum{ 0 };
		uint32_t count{ 0 };
		float AverageWidth{0};
		float AverageHeight{0};

		std::vector<std::pair<unsigned char, Font::Character>> characters {};
		std::vector<uint64_t> bufferLocations {};
		std::vector<Buffer> characterBuffers{};
		uint64_t totalBuffer{ 0 };

		for (unsigned char character{ 0 }; character < 128; character++)
		{
			Buffer fontBuffer{};
			// Load Character Glyph
			if (FT_Load_Char(face, character, FT_LOAD_RENDER))
			{
				KG_CORE_ERROR("Font Character not Loaded Correctly!");
				return;
			}

			Font::Character mapCharacter = {
				nullptr,
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
		  glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
				static_cast<uint32_t>(face->glyph->advance.x)
			};
			// Create Character Buffer!
			characters.push_back({ character, mapCharacter });
			widthSum += mapCharacter.Advance >> 6;
			heightSum += face->glyph->bitmap.rows;
			count++;

			// Store character image files in series of buffers and holding a vector of locations
			//		inside the buffer where each image is located.
			//		Ex: Image 2's start location is located at bufferLocations.at(1);
			uint64_t bufferSize = mapCharacter.Size.x * mapCharacter.Size.y * sizeof(uint8_t);
			fontBuffer.Allocate(bufferSize);
			bufferLocations.push_back(totalBuffer);
			totalBuffer += bufferSize;
			std::memcpy(fontBuffer.Data, face->glyph->bitmap.buffer, bufferSize);
			characterBuffers.push_back(fontBuffer);
		}

		AverageWidth = static_cast<float>(widthSum / count);
		AverageHeight = static_cast<float>(heightSum / count);

		FT_Done_Face(face);
		FT_Done_FreeType(ft);

		// Save Binary Intermediate into File
		std::string intermediatePath = "Fonts/Intermediates/" + (std::string)newAsset.Handle + ".kgfont";
		std::filesystem::path intermediateFullPath = Project::GetAssetDirectory() / intermediatePath;
		FileSystem::WriteFileBinary(intermediateFullPath, characterBuffers);

		// Load data into In-Memory Metadata object
		newAsset.Data.Type = Assets::AssetType::Font;
		newAsset.Data.IntermediateLocation = intermediatePath;
		Ref<FontMetaData> metadata = CreateRef<FontMetaData>();
		metadata->BufferLocations = bufferLocations;
		metadata->Characters = characters;
		metadata->AverageWidth = AverageWidth;
		metadata->AverageHeight= AverageHeight;
		metadata->InitialFileLocation = FileSystem::GetRelativePath(Project::GetAssetDirectory(), filePath);
		newAsset.Data.SpecificFileData = metadata;
		// Release all buffer data
		for (auto& buffer : characterBuffers)
		{
			buffer.Release();
		}
	}

	std::unordered_map<AssetHandle, Asset> AssetManager::s_SceneRegistry {};

	void AssetManager::DeserializeSceneRegistry()
	{
		s_SceneRegistry.clear();
		KG_CORE_ASSERT(Project::GetActive(), "There is no currently loaded project to serialize from!");
		const auto& sceneRegistryLocation = Project::GetAssetDirectory() / "Scenes/SceneRegistry.kgreg";

		if (!std::filesystem::exists(sceneRegistryLocation))
		{
			KG_CORE_ERROR("No .kgregistry file exists in project path!");
			return;
		}
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(sceneRegistryLocation.string());
		}
		catch (YAML::ParserException e)
		{
			KG_CORE_ERROR("Failed to load .kgscene file '{0}'\n     {1}", sceneRegistryLocation.string(), e.what());
			return;
		}

		if (!data["Registry"]) { return; }

		std::string registryName = data["Registry"].as<std::string>();
		KG_CORE_TRACE("Deserializing Scene Registry");

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
				if (newAsset.Data.Type == Assets::Scene)
				{
					Ref<SceneMetaData> sceneMetaData = CreateRef<SceneMetaData>();
					newAsset.Data.SpecificFileData = sceneMetaData;
				}
				s_SceneRegistry.insert({ newAsset.Handle, newAsset });
			}
		}
	}

	void AssetManager::SerializeSceneRegistry()
	{
		KG_CORE_ASSERT(Project::GetActive(), "There is no currently loaded project to serialize to!");
		const auto& sceneRegistryLocation = Project::GetAssetDirectory() / "Scenes/SceneRegistry.kgreg";
		YAML::Emitter out;

		out << YAML::BeginMap;
		out << YAML::Key << "Registry" << YAML::Value << "Scene";
		out << YAML::Key << "Assets" << YAML::Value << YAML::BeginSeq;

		// Asset
		for (auto& [handle, asset] : s_SceneRegistry)
		{
			out << YAML::BeginMap; // Asset Map
			out << YAML::Key << "AssetHandle" << YAML::Value << static_cast<uint64_t>(handle);
			out << YAML::Key << "MetaData" << YAML::Value;
			out << YAML::BeginMap; // MetaData Map
			out << YAML::Key << "CheckSum" << YAML::Value << asset.Data.CheckSum;
			out << YAML::Key << "IntermediateLocation" << YAML::Value << asset.Data.IntermediateLocation.string();
			out << YAML::Key << "AssetType" << YAML::Value << Assets::AssetTypeToString(asset.Data.Type);

			out << YAML::EndMap; // MetaData Map
			out << YAML::EndMap; // Asset Map
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;

		FileSystem::CreateNewDirectory(sceneRegistryLocation.parent_path());

		std::ofstream fout(sceneRegistryLocation);
		fout << out.c_str();
	}

	static void SerializeEntity(YAML::Emitter& out, Entity entity)
	{
		KG_CORE_ASSERT(entity.HasComponent<IDComponent>(), "Entity does not have a component");

		out << YAML::BeginMap; // Entity Map
		out << YAML::Key << "Entity" << YAML::Value << static_cast<uint64_t>(entity.GetUUID());


		if (entity.HasComponent<TagComponent>())
		{
			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap; // Component Map
			auto& tag = entity.GetComponent<TagComponent>().Tag;
			out << YAML::Key << "Tag" << YAML::Value << tag;
			out << YAML::EndMap; // Component Map
		}

		if (entity.HasComponent<TransformComponent>())
		{
			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap; // Component Map
			auto& tc = entity.GetComponent<TransformComponent>();
			out << YAML::Key << "Translation" << YAML::Value << tc.Translation;
			out << YAML::Key << "Rotation" << YAML::Value << tc.Rotation;
			out << YAML::Key << "Scale" << YAML::Value << tc.Scale;

			out << YAML::EndMap; // Component Map
		}
		if (entity.HasComponent<CameraComponent>())
		{
			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap; // Component Map

			auto& cameraComponent = entity.GetComponent<CameraComponent>();
			auto& camera = cameraComponent.Camera;

			out << YAML::Key << "Camera" << YAML::Value;
			out << YAML::BeginMap;
			out << YAML::Key << "ProjectionType" << YAML::Value << (int)camera.GetProjectionType();
			out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera.GetPerspectiveVerticalFOV();
			out << YAML::Key << "PerspectiveNear" << YAML::Value << camera.GetPerspectiveNearClip();
			out << YAML::Key << "PerspectiveFar" << YAML::Value << camera.GetPerspectiveFarClip();

			out << YAML::Key << "OrthographicSize" << YAML::Value << camera.GetOrthographicSize();
			out << YAML::Key << "OrthographicNear" << YAML::Value << camera.GetOrthographicNearClip();
			out << YAML::Key << "OrthographicFar" << YAML::Value << camera.GetOrthographicFarClip();
			out << YAML::EndMap;

			out << YAML::Key << "Primary" << YAML::Value << cameraComponent.Primary;
			out << YAML::Key << "FixedAspectRatio" << YAML::Value << cameraComponent.FixedAspectRatio;


			out << YAML::EndMap; // Component Map
		}

		if (entity.HasComponent<AudioComponent>())
		{
			out << YAML::Key << "AudioComponent";
			out << YAML::BeginMap; // Component Map
			auto& audioComponent = entity.GetComponent<AudioComponent>();
			out << YAML::Key << "Name" << YAML::Value << audioComponent.Name;
			out << YAML::Key << "AudioHandle" << YAML::Value << static_cast<uint64_t>(audioComponent.AudioHandle);
			out << YAML::EndMap; // Component Map
		}

		if (entity.HasComponent<MultiAudioComponent>())
		{
			out << YAML::Key << "MultiAudioComponent";
			out << YAML::BeginSeq; // Component Sequence

			for (auto& [key, audioComp] : entity.GetComponent<MultiAudioComponent>().AudioComponents)
			{
				out << YAML::BeginMap; // Audio Component Map
				out << YAML::Key << "Name" << YAML::Value << audioComp.Name;
				out << YAML::Key << "AudioHandle" << YAML::Value << static_cast<uint64_t>(audioComp.AudioHandle);
				out << YAML::EndMap; // Audio Component Map
			}

			out << YAML::EndSeq; // Component Sequence
		}

		if (entity.HasComponent<ShapeComponent>())
		{
			out << YAML::Key << "ShapeComponent";
			out << YAML::BeginMap; // Component Map
			auto& shapeComponent = entity.GetComponent<ShapeComponent>();
			out << YAML::Key << "CurrentShape" << YAML::Value << Shape::ShapeTypeToString(shapeComponent.CurrentShape);
			if (shapeComponent.VertexColors)
			{
				out << YAML::Key << "VertexColors" << YAML::Value << YAML::BeginSeq;
				for (const auto& color : *shapeComponent.VertexColors)
				{
					out << YAML::BeginMap;
					out << YAML::Key << "Color" << YAML::Value << color;
					out << YAML::EndMap;
				}
				out << YAML::EndSeq;
			}
			if (shapeComponent.Texture)
			{
				out << YAML::Key << "TextureHandle" << YAML::Value << static_cast<uint64_t>(shapeComponent.TextureHandle);
			}
			KG_CORE_ASSERT(sizeof(uint8_t) * 20 == sizeof(Shader::ShaderSpecification), "Please Update Deserialization and Serialization. Incorrect size of input data in Scene Serializer!");
			if (shapeComponent.Shader)
			{
				// Add Shader Handle
				out << YAML::Key << "ShaderHandle" << YAML::Value << static_cast<uint64_t>(shapeComponent.ShaderHandle);
				// Add Shader Specification
				const Shader::ShaderSpecification& shaderSpec = shapeComponent.Shader->GetSpecification();
				out << YAML::Key << "ShaderSpecification" << YAML::Value;
				out << YAML::BeginMap;
				out << YAML::Key << "ColorInputType" << YAML::Value << Shader::ColorInputTypeToString(shaderSpec.ColorInput);
				out << YAML::Key << "AddProjectionMatrix" << YAML::Value << shaderSpec.AddProjectionMatrix;
				out << YAML::Key << "AddEntityID" << YAML::Value << shaderSpec.AddEntityID;
				out << YAML::Key << "AddCircleShape" << YAML::Value << shaderSpec.AddCircleShape;
				out << YAML::Key << "TextureInput" << YAML::Value << Shader::TextureInputTypeToString(shaderSpec.TextureInput);
				out << YAML::Key << "DrawOutline" << YAML::Value << shaderSpec.DrawOutline;
				out << YAML::Key << "RenderType" << YAML::Value << Shape::RenderingTypeToString(shaderSpec.RenderType);

				out << YAML::EndMap;
				// Add Buffer
				out << YAML::Key << "Buffer" << YAML::Value << YAML::Binary(shapeComponent.ShaderData.Data, shapeComponent.ShaderData.Size);
			}
			out << YAML::EndMap; // Component Map
		}

		if (entity.HasComponent<Rigidbody2DComponent>())
		{
			out << YAML::Key << "Rigidbody2DComponent";
			out << YAML::BeginMap; // Component Map
			auto& rb2dComponent = entity.GetComponent<Rigidbody2DComponent>();
			out << YAML::Key << "BodyType" << YAML::Value << RigidBody2DBodyTypeToString(rb2dComponent.Type);
			out << YAML::Key << "FixedRotation" << YAML::Value << rb2dComponent.FixedRotation;
			out << YAML::EndMap; // Component Map
		}

		if (entity.HasComponent<BoxCollider2DComponent>())
		{
			out << YAML::Key << "BoxCollider2DComponent";
			out << YAML::BeginMap; // Component Map
			auto& bc2dComponent = entity.GetComponent<BoxCollider2DComponent>();
			out << YAML::Key << "Offset" << YAML::Value << bc2dComponent.Offset;
			out << YAML::Key << "Size" << YAML::Value << bc2dComponent.Size;
			out << YAML::Key << "Density" << YAML::Value << bc2dComponent.Density;
			out << YAML::Key << "Friction" << YAML::Value << bc2dComponent.Friction;
			out << YAML::Key << "Restitution" << YAML::Value << bc2dComponent.Restitution;
			out << YAML::Key << "RestitutionThreshold" << YAML::Value << bc2dComponent.RestitutionThreshold;
			out << YAML::EndMap; // Component Map
		}

		if (entity.HasComponent<CircleCollider2DComponent>())
		{
			out << YAML::Key << "CircleCollider2DComponent";
			out << YAML::BeginMap; // Component Map
			auto& cc2dComponent = entity.GetComponent<CircleCollider2DComponent>();
			out << YAML::Key << "Offset" << YAML::Value << cc2dComponent.Offset;
			out << YAML::Key << "Radius" << YAML::Value << cc2dComponent.Radius;
			out << YAML::Key << "Density" << YAML::Value << cc2dComponent.Density;
			out << YAML::Key << "Friction" << YAML::Value << cc2dComponent.Friction;
			out << YAML::Key << "Restitution" << YAML::Value << cc2dComponent.Restitution;
			out << YAML::Key << "RestitutionThreshold" << YAML::Value << cc2dComponent.RestitutionThreshold;
			out << YAML::EndMap; // Component Map
		}

		if (entity.HasComponent<ScriptComponent>())
		{
			auto& scriptComponent = entity.GetComponent<ScriptComponent>();

			out << YAML::Key << "ScriptComponent";
			out << YAML::BeginMap; // Component Map
			out << YAML::Key << "ClassName" << YAML::Value << scriptComponent.ClassName;


			// Fields
			Ref<ScriptClass> entityClass = ScriptEngine::GetEntityClass(scriptComponent.ClassName);
			const auto& fields = entityClass->GetFields();
			if (fields.size() > 0)
			{
				out << YAML::Key << "ScriptFields" << YAML::Value;
				auto& entityFields = ScriptEngine::GetScriptFieldMap(entity);
				out << YAML::BeginSeq;
				for (const auto& [name, field] : fields)
				{

					if (!entityFields.contains(name)) { continue; }

					out << YAML::BeginMap; // Script Fields
					out << YAML::Key << "Name" << YAML::Value << name;
					out << YAML::Key << "Type" << YAML::Value << Utils::ScriptFieldTypeToString(field.Type);
					out << YAML::Key << "Data" << YAML::Value;

					ScriptFieldInstance& scriptField = entityFields.at(name);

					switch (field.Type)
					{
						WRITE_SCRIPT_FIELD(ScriptFieldType::Float, float);
						WRITE_SCRIPT_FIELD(ScriptFieldType::Double, double);
						WRITE_SCRIPT_FIELD(ScriptFieldType::Bool, bool);
						WRITE_SCRIPT_FIELD(ScriptFieldType::Char, char);
						WRITE_SCRIPT_FIELD(ScriptFieldType::Byte, int8_t);
						WRITE_SCRIPT_FIELD(ScriptFieldType::Short, int16_t);
						WRITE_SCRIPT_FIELD(ScriptFieldType::Int, int32_t);
						WRITE_SCRIPT_FIELD(ScriptFieldType::Long, int64_t);
						WRITE_SCRIPT_FIELD(ScriptFieldType::UByte, uint8_t);
						WRITE_SCRIPT_FIELD(ScriptFieldType::UShort, uint16_t);
						WRITE_SCRIPT_FIELD(ScriptFieldType::UInt, uint32_t);
						WRITE_SCRIPT_FIELD(ScriptFieldType::ULong, uint64_t);
						WRITE_SCRIPT_FIELD(ScriptFieldType::Vector2, glm::vec2);
						WRITE_SCRIPT_FIELD(ScriptFieldType::Vector3, glm::vec3);
						WRITE_SCRIPT_FIELD(ScriptFieldType::Vector4, glm::vec4);
						WRITE_SCRIPT_FIELD(ScriptFieldType::Entity, uint64_t);

					}

					out << YAML::EndMap; // Script Fields
				}
				out << YAML::EndSeq;
			}


			out << YAML::EndMap; // Component Map
		}


		out << YAML::EndMap; // Entity
	}


	void AssetManager::SerializeScene(Ref<Scene> scene, const std::filesystem::path& filepath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap; // Start of File Map
		{ // Physics
			out << YAML::Key << "Physics" << YAML::BeginMap; // Physics Map
			out << YAML::Key << "Gravity" << YAML::Value << scene->m_PhysicsSpecification.Gravity;
			out << YAML::EndMap; // Physics Maps
		}

		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
		scene->m_Registry.each([&](auto entityID)
			{
				Entity entity = { entityID, scene.get() };
				if (!entity) { return; }

				SerializeEntity(out, entity);
			});
		out << YAML::EndSeq;
		out << YAML::EndMap; // Start of File Map

		std::ofstream fout(filepath);
		fout << out.c_str();
		KG_CORE_INFO("Successfully Serialized Scene at {}", filepath);
	}

	bool AssetManager::CheckSceneExists(const std::string& sceneName)
	{
		// Create Checksum
		const std::string currentCheckSum = FileSystem::ChecksumFromString(sceneName);

		if (currentCheckSum.empty())
		{
			KG_CORE_ERROR("Failed to generate checksum from file!");
			return {};
		}

		for (const auto& [handle, asset] : s_SceneRegistry)
		{
			if (asset.Data.CheckSum == currentCheckSum)
			{
				KG_CORE_INFO("Attempt to instantiate duplicate font asset");
				return true;
			}
		}

		return false;
	}

	bool AssetManager::DeserializeScene(Ref<Scene> scene, const std::filesystem::path& filepath)
	{
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(filepath.string());
		}
		catch (YAML::ParserException e)
		{
			KG_CORE_ERROR("Failed to load .kgscene file '{0}'\n     {1}", filepath, e.what());
			return false;
		}

		KG_CORE_TRACE("Deserializing scene");

		auto physics = data["Physics"];
		scene->GetPhysicsSpecification().Gravity = physics["Gravity"].as<glm::vec2>();

		auto entities = data["Entities"];
		if (entities)
		{
			for (auto entity : entities)
			{
				uint64_t uuid = entity["Entity"].as<uint64_t>();

				std::string name;
				auto tagComponent = entity["TagComponent"];
				if (tagComponent) { name = tagComponent["Tag"].as<std::string>(); }
				//KG_CORE_TRACE("Deserialize entity with ID = {0}, name = {1}", uuid, name);

				Entity deserializedEntity = scene->CreateEntityWithUUID(uuid, name);

				auto transformComponent = entity["TransformComponent"];
				if (transformComponent)
				{
					auto& tc = deserializedEntity.GetComponent<TransformComponent>();
					tc.Translation = transformComponent["Translation"].as<glm::vec3>();
					tc.Rotation = transformComponent["Rotation"].as<glm::vec3>();
					tc.Scale = transformComponent["Scale"].as<glm::vec3>();
				}

				auto cameraComponent = entity["CameraComponent"];
				if (cameraComponent)
				{
					auto& cc = deserializedEntity.AddComponent<CameraComponent>();

					const auto& cameraProps = cameraComponent["Camera"];

					cc.Camera.SetProjectionType((SceneCamera::ProjectionType)cameraProps["ProjectionType"].as<int>());
					cc.Camera.SetPerspectiveVerticalFOV(cameraProps["PerspectiveFOV"].as<float>());
					cc.Camera.SetPerspectiveNearClip(cameraProps["PerspectiveNear"].as<float>());
					cc.Camera.SetPerspectiveFarClip(cameraProps["PerspectiveFar"].as<float>());

					cc.Camera.SetOrthographicSize(cameraProps["OrthographicSize"].as<float>());
					cc.Camera.SetOrthographicNearClip(cameraProps["OrthographicNear"].as<float>());
					cc.Camera.SetOrthographicFarClip(cameraProps["OrthographicFar"].as<float>());

					cc.Primary = cameraComponent["Primary"].as<bool>();
					cc.FixedAspectRatio = cameraComponent["FixedAspectRatio"].as<bool>();
				}

				auto audioComponent = entity["AudioComponent"];
				if (audioComponent)
				{
					auto& audioComp = deserializedEntity.AddComponent<AudioComponent>();
					audioComp.Name = audioComponent["Name"].as<std::string>();
					audioComp.AudioHandle = audioComponent["AudioHandle"].as<uint64_t>();
					audioComp.Audio = AssetManager::GetAudio(audioComp.AudioHandle);
				}

				auto multiAudioComponent = entity["MultiAudioComponent"];
				if (multiAudioComponent)
				{
					auto& multiAudioComp = deserializedEntity.AddComponent<MultiAudioComponent>();

					for (auto audioComp : multiAudioComponent)
					{
						AudioComponent newComponent{};
						newComponent.Name = audioComp["Name"].as<std::string>();
						newComponent.AudioHandle = audioComp["AudioHandle"].as<uint64_t>();
						newComponent.Audio = AssetManager::GetAudio(newComponent.AudioHandle);
						multiAudioComp.AudioComponents.insert({ newComponent.Name, newComponent });
					}
				}

				auto scriptComponent = entity["ScriptComponent"];
				if (scriptComponent)
				{
					auto& sc = deserializedEntity.AddComponent<ScriptComponent>();
					sc.ClassName = scriptComponent["ClassName"].as<std::string>();

					auto scriptFields = scriptComponent["ScriptFields"];
					if (scriptFields)
					{

						Ref<ScriptClass> entityClass = ScriptEngine::GetEntityClass(sc.ClassName);
						if (entityClass)
						{
							KG_CORE_ASSERT(entityClass);
							const auto& fields = entityClass->GetFields();

							auto& entityFields = ScriptEngine::GetScriptFieldMap(deserializedEntity);

							for (auto scriptField : scriptFields)
							{
								std::string name = scriptField["Name"].as<std::string>();
								std::string typeString = scriptField["Type"].as<std::string>();
								ScriptFieldType type = Utils::ScriptFieldTypeFromString(typeString);

								ScriptFieldInstance& fieldInstance = entityFields[name];
								// TODO(): Turn into Log Message
								KG_CORE_ASSERT(fields.contains(name))
									if (!fields.contains(name)) { continue; }
								fieldInstance.Field = fields.at(name);

								switch (type)
								{
									READ_SCRIPT_FIELD(ScriptFieldType::Float, float);
									READ_SCRIPT_FIELD(ScriptFieldType::Double, double);
									READ_SCRIPT_FIELD(ScriptFieldType::Bool, bool);
									READ_SCRIPT_FIELD(ScriptFieldType::Char, char);
									READ_SCRIPT_FIELD(ScriptFieldType::Byte, int8_t);
									READ_SCRIPT_FIELD(ScriptFieldType::Short, int16_t);
									READ_SCRIPT_FIELD(ScriptFieldType::Int, int32_t);
									READ_SCRIPT_FIELD(ScriptFieldType::Long, int64_t);
									READ_SCRIPT_FIELD(ScriptFieldType::UByte, uint8_t);
									READ_SCRIPT_FIELD(ScriptFieldType::UShort, uint16_t);
									READ_SCRIPT_FIELD(ScriptFieldType::UInt, uint32_t);
									READ_SCRIPT_FIELD(ScriptFieldType::ULong, uint64_t);
									READ_SCRIPT_FIELD(ScriptFieldType::Vector2, glm::vec2);
									READ_SCRIPT_FIELD(ScriptFieldType::Vector3, glm::vec3);
									READ_SCRIPT_FIELD(ScriptFieldType::Vector4, glm::vec4);
									READ_SCRIPT_FIELD(ScriptFieldType::Entity, UUID);
								}
							}
						}
					}
				}

				auto shapeComponent = entity["ShapeComponent"];
				if (shapeComponent)
				{
					auto& sc = deserializedEntity.AddComponent<ShapeComponent>();
					sc.CurrentShape = Shape::StringToShapeType(shapeComponent["CurrentShape"].as<std::string>());

					if (shapeComponent["VertexColors"])
					{
						auto vertexColors = shapeComponent["VertexColors"];
						sc.VertexColors = CreateRef<std::vector<glm::vec4>>();
						for (auto color : vertexColors)
						{
							sc.VertexColors->push_back(color["Color"].as<glm::vec4>());
						}
					}

					if (shapeComponent["TextureHandle"])
					{
						AssetHandle textureHandle = shapeComponent["TextureHandle"].as<uint64_t>();
						sc.Texture = AssetManager::GetTexture(textureHandle);
						sc.TextureHandle = textureHandle;
					}

					if (shapeComponent["ShaderHandle"])
					{
						AssetHandle shaderHandle = shapeComponent["ShaderHandle"].as<uint64_t>();
						sc.Shader = AssetManager::GetShader(shaderHandle);
						if (!sc.Shader)
						{
							auto shaderSpecificationNode = shapeComponent["ShaderSpecification"];
							Shader::ShaderSpecification shaderSpec{};
							// ShaderSpecification Section
							shaderSpec.ColorInput = Shader::StringToColorInputType(shaderSpecificationNode["ColorInputType"].as<std::string>());
							shaderSpec.AddProjectionMatrix = shaderSpecificationNode["AddProjectionMatrix"].as<bool>();
							shaderSpec.AddEntityID = shaderSpecificationNode["AddEntityID"].as<bool>();
							shaderSpec.AddCircleShape = shaderSpecificationNode["AddCircleShape"].as<bool>();
							shaderSpec.TextureInput = Shader::StringToTextureInputType(shaderSpecificationNode["TextureInput"].as<std::string>());
							shaderSpec.DrawOutline = shaderSpecificationNode["DrawOutline"].as<bool>();
							shaderSpec.RenderType = Shape::StringToRenderingType(shaderSpecificationNode["RenderType"].as<std::string>());
							auto [newHandle, newShader] = AssetManager::GetShader(shaderSpec);
							shaderHandle = newHandle;
							sc.Shader = newShader;
						}
						sc.ShaderHandle = shaderHandle;
						sc.ShaderSpecification = sc.Shader->GetSpecification();
						YAML::Binary binary = shapeComponent["Buffer"].as<YAML::Binary>();
						Buffer buffer{ binary.size() };
						memcpy_s(buffer.Data, buffer.Size, binary.data(), buffer.Size);
						sc.ShaderData = buffer;
						if (sc.CurrentShape != Shape::ShapeTypes::None)
						{
							if (sc.ShaderSpecification.RenderType == Shape::RenderingType::DrawIndex)
							{
								sc.Vertices = CreateRef<std::vector<glm::vec3>>(Shape::ShapeTypeToShape(sc.CurrentShape).GetIndexVertices());
								sc.Indices = CreateRef<std::vector<uint32_t>>(Shape::ShapeTypeToShape(sc.CurrentShape).GetIndices());
								sc.TextureCoordinates = CreateRef<std::vector<glm::vec2>>(Shape::ShapeTypeToShape(sc.CurrentShape).GetIndexTextureCoordinates());
							}

							if (sc.ShaderSpecification.RenderType == Shape::RenderingType::DrawTriangle)
							{
								sc.Vertices = CreateRef<std::vector<glm::vec3>>(Shape::ShapeTypeToShape(sc.CurrentShape).GetTriangleVertices());
								sc.TextureCoordinates = CreateRef<std::vector<glm::vec2>>(Shape::ShapeTypeToShape(sc.CurrentShape).GetTriangleTextureCoordinates());
							}
						}
					}
				}

				auto rigidbody2DComponent = entity["Rigidbody2DComponent"];
				if (rigidbody2DComponent)
				{
					auto& rb2d = deserializedEntity.AddComponent<Rigidbody2DComponent>();
					rb2d.Type = StringToRigidBody2DBodyType(rigidbody2DComponent["BodyType"].as<std::string>());
					rb2d.FixedRotation = rigidbody2DComponent["FixedRotation"].as<bool>();
				}

				auto boxCollider2DComponent = entity["BoxCollider2DComponent"];
				if (boxCollider2DComponent)
				{
					auto& bc2d = deserializedEntity.AddComponent<BoxCollider2DComponent>();
					bc2d.Offset = boxCollider2DComponent["Offset"].as<glm::vec2>();
					bc2d.Size = boxCollider2DComponent["Size"].as<glm::vec2>();
					bc2d.Density = boxCollider2DComponent["Density"].as<float>();
					bc2d.Friction = boxCollider2DComponent["Friction"].as<float>();
					bc2d.Restitution = boxCollider2DComponent["Restitution"].as<float>();
					bc2d.RestitutionThreshold = boxCollider2DComponent["RestitutionThreshold"].as<float>();
				}

				auto circleCollider2DComponent = entity["CircleCollider2DComponent"];
				if (circleCollider2DComponent)
				{
					auto& cc2d = deserializedEntity.AddComponent<CircleCollider2DComponent>();
					cc2d.Offset = circleCollider2DComponent["Offset"].as<glm::vec2>();
					cc2d.Radius = circleCollider2DComponent["Radius"].as<float>();
					cc2d.Density = circleCollider2DComponent["Density"].as<float>();
					cc2d.Friction = circleCollider2DComponent["Friction"].as<float>();
					cc2d.Restitution = circleCollider2DComponent["Restitution"].as<float>();
					cc2d.RestitutionThreshold = circleCollider2DComponent["RestitutionThreshold"].as<float>();
				}
			}
		}
		return true;

	}

	AssetHandle AssetManager::CreateNewScene(const std::string& sceneName)
	{
		// Create Checksum
		const std::string currentCheckSum = FileSystem::ChecksumFromString(sceneName);

		if (currentCheckSum.empty())
		{
			KG_CORE_ERROR("Failed to generate checksum from file!");
			return {};
		}

		for (const auto& [handle, asset] : s_SceneRegistry)
		{
			if (asset.Data.CheckSum == currentCheckSum)
			{
				KG_CORE_INFO("Attempt to instantiate duplicate font asset");
				return handle;
			}
		}

		AssetHandle newHandle{};

		Asset newAsset{};
		newAsset.Handle = newHandle;

		CreateSceneFile(sceneName, newAsset);
		newAsset.Data.CheckSum = currentCheckSum;

		s_SceneRegistry.insert({ newHandle, newAsset }); // Update Registry Map in-memory
		SerializeSceneRegistry(); // Update Registry File on Disk

		return newHandle;
	}

	void AssetManager::SaveScene(AssetHandle sceneHandle, Ref<Scene> scene)
	{
		if (!s_SceneRegistry.contains(sceneHandle))
		{
			KG_ERROR("Attempt to save scene that does not exist in registry");
			return;
		}
		Asset sceneAsset = s_SceneRegistry[sceneHandle];
		SerializeScene(scene, (Project::GetAssetDirectory() / sceneAsset.Data.IntermediateLocation).string());
	}

	Ref<Scene> AssetManager::GetScene(const AssetHandle& handle)
	{
		KG_CORE_ASSERT(Project::GetActive(), "There is no active project when retreiving scene!");

		if (s_SceneRegistry.contains(handle))
		{
			auto asset = s_SceneRegistry[handle];
			return InstantiateScene(asset);
		}

		KG_CORE_ERROR("No scene is associated with provided handle!");
		return nullptr;
	}
	std::tuple<AssetHandle, Ref<Scene>> AssetManager::GetScene(const std::filesystem::path& filepath)
	{
		KG_CORE_ASSERT(Project::GetActive(), "Attempt to use Project Field without active project!");
		
		std::filesystem::path scenePath = FileSystem::GetRelativePath(Project::GetAssetDirectory(), filepath);

		for (auto& [assetHandle, asset] : s_SceneRegistry)
		{
			if (asset.Data.IntermediateLocation.compare(scenePath) == 0)
			{
				return std::make_tuple(assetHandle, InstantiateScene(asset));
			}
		}
		// Return empty scene if scene does not exist
		KG_CORE_WARN("No Scene Associated with provided handle. Returned new empty scene");
		AssetHandle newHandle = CreateNewScene(filepath.stem().string());
		return std::make_tuple(newHandle, GetScene(newHandle));
	}

	Ref<Scene> AssetManager::InstantiateScene(const Asset& sceneAsset)
	{
		Ref<Scene> newScene = CreateRef<Scene>();
		DeserializeScene(newScene, (Project::GetAssetDirectory() / sceneAsset.Data.IntermediateLocation).string());
		return newScene;
	}


	void AssetManager::ClearSceneRegistry()
	{
		s_SceneRegistry.clear();
	}

	void AssetManager::CreateSceneFile(const std::string& sceneName, Asset& newAsset)
	{
		// Create Temporary Scene
		Ref<Scene> temporaryScene = CreateRef<Scene>();

		// Save Binary Intermediate into File
		std::string scenePath = "Scenes/" + sceneName + ".kgscene";
		std::filesystem::path intermediateFullPath = Project::GetAssetDirectory() / scenePath;
		SerializeScene(temporaryScene, intermediateFullPath.string());

		// Load data into In-Memory Metadata object
		newAsset.Data.Type = Assets::AssetType::Scene;
		newAsset.Data.IntermediateLocation = scenePath;
		Ref<SceneMetaData> metadata = CreateRef<SceneMetaData>();
		newAsset.Data.SpecificFileData = metadata;
	}

	std::unordered_map<AssetHandle, Asset> AssetManager::s_UIObjectRegistry {};

	void AssetManager::DeserializeUIObjectRegistry()
	{
		s_UIObjectRegistry.clear();
		KG_CORE_ASSERT(Project::GetActive(), "There is no currently loaded project to serialize from!");
		const auto& uiObjectRegistryLocation = Project::GetAssetDirectory() / "UserInterface/UIObjectRegistry.kgreg";

		if (!std::filesystem::exists(uiObjectRegistryLocation))
		{
			KG_CORE_ERROR("No .kgregistry file exists in project path!");
			return;
		}
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(uiObjectRegistryLocation.string());
		}
		catch (YAML::ParserException e)
		{
			KG_CORE_ERROR("Failed to load .kguiObject file '{0}'\n     {1}", uiObjectRegistryLocation.string(), e.what());
			return;
		}

		if (!data["Registry"]) { return; }

		std::string registryName = data["Registry"].as<std::string>();
		KG_CORE_TRACE("Deserializing UIObject Registry");

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
				if (newAsset.Data.Type == Assets::UIObject)
				{
					Ref<UIObjectMetaData> uiObjectMetaData = CreateRef<UIObjectMetaData>();
					newAsset.Data.SpecificFileData = uiObjectMetaData;
				}
				s_UIObjectRegistry.insert({ newAsset.Handle, newAsset });
			}
		}
	}

	void AssetManager::SerializeUIObjectRegistry()
	{
		KG_CORE_ASSERT(Project::GetActive(), "There is no currently loaded project to serialize to!");
		const auto& uiObjectRegistryLocation = Project::GetAssetDirectory() / "UserInterface/UIObjectRegistry.kgreg";
		YAML::Emitter out;

		out << YAML::BeginMap;
		out << YAML::Key << "Registry" << YAML::Value << "UserInterface";
		out << YAML::Key << "Assets" << YAML::Value << YAML::BeginSeq;

		// Asset
		for (auto& [handle, asset] : s_UIObjectRegistry)
		{
			out << YAML::BeginMap; // Asset Map
			out << YAML::Key << "AssetHandle" << YAML::Value << static_cast<uint64_t>(handle);
			out << YAML::Key << "MetaData" << YAML::Value;
			out << YAML::BeginMap; // MetaData Map
			out << YAML::Key << "CheckSum" << YAML::Value << asset.Data.CheckSum;
			out << YAML::Key << "IntermediateLocation" << YAML::Value << asset.Data.IntermediateLocation.string();
			out << YAML::Key << "AssetType" << YAML::Value << Assets::AssetTypeToString(asset.Data.Type);

			out << YAML::EndMap; // MetaData Map
			out << YAML::EndMap; // Asset Map
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;

		FileSystem::CreateNewDirectory(uiObjectRegistryLocation.parent_path());

		std::ofstream fout(uiObjectRegistryLocation);
		fout << out.c_str();
	}

	void AssetManager::SerializeUIObject(Ref<UIEngine::UIObject> uiObject, const std::filesystem::path& filepath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap; // Start of File Map
		// Font
		out << YAML::Key << "Font" << YAML::Value << static_cast<uint64_t>(uiObject->m_FontHandle);
		// Windows
		out << YAML::Key << "Windows" << YAML::Value;
		out << YAML::BeginSeq; // Start of Windows Seq

		for (auto& window : uiObject->Windows)
		{
			out << YAML::BeginMap; // Start Window Map

			out << YAML::Key << "ScreenPosition" << YAML::Value << window.ScreenPosition;
			out << YAML::Key << "Size" << YAML::Value << window.Size;
			out << YAML::Key << "BackgroundColor" << YAML::Value << window.BackgroundColor;
			out << YAML::Key << "ParentIndex" << YAML::Value << window.ParentIndex;
			out << YAML::Key << "ChildBufferIndex" << YAML::Value << window.ChildBufferIndex;
			out << YAML::Key << "ChildBufferSize" << YAML::Value << window.ChildBufferSize;

			out << YAML::Key << "WidgetCounts" << YAML::Value;
			out << YAML::BeginMap; // Begin WidgetCounts Map
			out << YAML::Key << "TextWidgetCount" << YAML::Value << window.WidgetCounts.TextWidgetCount;
			out << YAML::Key << "TextWidgetLocation" << YAML::Value << window.WidgetCounts.TextWidgetLocation;
			out << YAML::Key << "ButtonWidgetCount" << YAML::Value << window.WidgetCounts.ButtonWidgetCount;
			out << YAML::Key << "ButtonWidgetLocation" << YAML::Value << window.WidgetCounts.ButtonWidgetLocation;
			out << YAML::Key << "CheckboxWidgetCount" << YAML::Value << window.WidgetCounts.CheckboxWidgetCount;
			out << YAML::Key << "CheckboxWidgetLocation" << YAML::Value << window.WidgetCounts.CheckboxWidgetLocation;
			out << YAML::Key << "ComboWidgetCount" << YAML::Value << window.WidgetCounts.ComboWidgetCount;
			out << YAML::Key << "ComboWidgetLocation" << YAML::Value << window.WidgetCounts.ComboWidgetLocation;
			out << YAML::Key << "PopupWidgetCount" << YAML::Value << window.WidgetCounts.PopupWidgetCount;
			out << YAML::Key << "PopupWidgetLocation" << YAML::Value << window.WidgetCounts.PopupWidgetLocation;
			out << YAML::EndMap; // End WidgetCounts Map

			out << YAML::Key << "Widgets" << YAML::Value;
			out << YAML::BeginSeq; // Begin Widget Sequence

			for (auto& widget : window.Widgets)
			{
				out << YAML::BeginMap; // Begin Widget Map

				out << YAML::Key << "WindowPosition" << YAML::Value << widget->WindowPosition;
				out << YAML::Key << "Size" << YAML::Value << widget->Size;
				out << YAML::Key << "BackgroundColor" << YAML::Value << widget->BackgroundColor;
				out << YAML::Key << "WidgetType" << YAML::Value << UIEngine::WidgetTypeToString(widget->WidgetType);
				switch (widget->WidgetType)
				{
				case UIEngine::WidgetTypes::TextWidget:
					{
						UIEngine::TextWidget* textWidget = static_cast<UIEngine::TextWidget*>(widget.get());
						out << YAML::Key << "TextWidget" << YAML::Value;
						out << YAML::BeginMap; // Begin TextWidget Map
						out << YAML::Key << "Text" << YAML::Value << textWidget->Text;
						out << YAML::Key << "TextSize" << YAML::Value << textWidget->TextSize;
						out << YAML::Key << "TextAbsoluteDimensions" << YAML::Value << textWidget->TextAbsoluteDimensions;
						out << YAML::Key << "TextCentered" << YAML::Value << textWidget->TextCentered;
						out << YAML::EndMap; // End TextWidget Map
						break;
					}
				}

				out << YAML::EndMap; // End Widget Map
			}

			out << YAML::EndSeq; // End Widget Sequence

			out << YAML::EndMap; // End Window Map
			std::vector<Ref<UIEngine::Widget>> Widgets {};
		}

		out << YAML::EndSeq; // End of Windows Seq
		out << YAML::EndMap; // Start of File Map

		std::ofstream fout(filepath);
		fout << out.c_str();
		KG_CORE_INFO("Successfully Serialized UIObject at {}", filepath);
	}

	bool AssetManager::CheckUIObjectExists(const std::string& uiObjectName)
	{
		// Create Checksum
		const std::string currentCheckSum = FileSystem::ChecksumFromString(uiObjectName);

		if (currentCheckSum.empty())
		{
			KG_CORE_ERROR("Failed to generate checksum from file!");
			return {};
		}

		for (const auto& [handle, asset] : s_UIObjectRegistry)
		{
			if (asset.Data.CheckSum == currentCheckSum)
			{
				KG_CORE_INFO("Attempt to instantiate duplicate font asset");
				return true;
			}
		}

		return false;
	}

	bool AssetManager::DeserializeUIObject(Ref<UIEngine::UIObject> uiObject, const std::filesystem::path& filepath)
	{
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(filepath.string());
		}
		catch (YAML::ParserException e)
		{
			KG_CORE_ERROR("Failed to load .kgui file '{0}'\n     {1}", filepath, e.what());
			return false;
		}

		KG_CORE_TRACE("Deserializing user interface object");
		// Get Font!
		uiObject->m_FontHandle = data["Font"].as<uint64_t>();
		uiObject->m_Font = AssetManager::GetFont(uiObject->m_FontHandle);
		// Get Windows!
		auto windows = data["Windows"];
		if (windows)
		{
			auto& newWindowsList = uiObject->Windows;
			for (auto window : windows)
			{
				UIEngine::Window newWindow{};
				newWindow.ScreenPosition = window["ScreenPosition"].as<glm::vec3>();
				newWindow.Size = window["Size"].as<glm::vec2>();
				newWindow.BackgroundColor = window["BackgroundColor"].as<glm::vec4>();
				newWindow.ParentIndex = window["ParentIndex"].as<int32_t>();
				newWindow.ChildBufferIndex = window["ChildBufferIndex"].as<int32_t>();
				newWindow.ChildBufferSize = window["ChildBufferSize"].as<uint32_t>();

				auto widgetCounts = window["WidgetCounts"];
				newWindow.WidgetCounts.TextWidgetCount = widgetCounts["TextWidgetCount"].as<uint16_t>();
				newWindow.WidgetCounts.TextWidgetLocation = widgetCounts["TextWidgetLocation"].as<uint16_t>();
				newWindow.WidgetCounts.ButtonWidgetCount = widgetCounts["ButtonWidgetCount"].as<uint16_t>();
				newWindow.WidgetCounts.ButtonWidgetLocation = widgetCounts["ButtonWidgetLocation"].as<uint16_t>();
				newWindow.WidgetCounts.CheckboxWidgetCount = widgetCounts["CheckboxWidgetCount"].as<uint16_t>();
				newWindow.WidgetCounts.CheckboxWidgetLocation = widgetCounts["CheckboxWidgetLocation"].as<uint16_t>();
				newWindow.WidgetCounts.ComboWidgetCount = widgetCounts["ComboWidgetCount"].as<uint16_t>();
				newWindow.WidgetCounts.ComboWidgetLocation = widgetCounts["ComboWidgetLocation"].as<uint16_t>();
				newWindow.WidgetCounts.PopupWidgetCount = widgetCounts["PopupWidgetCount"].as<uint16_t>();
				newWindow.WidgetCounts.PopupWidgetLocation = widgetCounts["PopupWidgetLocation"].as<uint16_t>();

				auto widgets = window["Widgets"];

				if (widgets)
				{
					auto& newWidgetsList = newWindow.Widgets;
					for (auto widget : widgets)
					{
						UIEngine::WidgetTypes widgetType = UIEngine::StringToWidgetType(widget["WidgetType"].as<std::string>());
						Ref<UIEngine::Widget> newWidget = nullptr;
						YAML::Node specificWidget;
						switch (widgetType)
						{
						case UIEngine::WidgetTypes::TextWidget:
						{
							specificWidget = widget["TextWidget"];
							newWidget = CreateRef<UIEngine::TextWidget>();
							newWidget->WidgetType = widgetType;
							UIEngine::TextWidget* textWidget = static_cast<UIEngine::TextWidget*>(newWidget.get());
							textWidget->Text = specificWidget["Text"].as<std::string>();
							textWidget->TextSize = specificWidget["TextSize"].as<float>();
							textWidget->TextAbsoluteDimensions = specificWidget["TextAbsoluteDimensions"].as<glm::vec2>();
							textWidget->TextCentered = specificWidget["TextCentered"].as<bool>();
							break;
						}
						default:
							{
							KG_CORE_ASSERT("Invalid Widget Type in RuntimeUI Deserialization");
							return false;
							}
						}

						newWidget->WindowPosition = widget["WindowPosition"].as<glm::vec2>();
						newWidget->Size = widget["Size"].as<glm::vec2>();
						newWidget->BackgroundColor = widget["BackgroundColor"].as<glm::vec4>();

						newWidgetsList.push_back(newWidget);


					}
				}

				newWindowsList.push_back(newWindow);
				
			}
		}
		return true;

	}

	AssetHandle AssetManager::CreateNewUIObject(const std::string& uiObjectName)
	{
		// Create Checksum
		const std::string currentCheckSum = FileSystem::ChecksumFromString(uiObjectName);

		if (currentCheckSum.empty())
		{
			KG_CORE_ERROR("Failed to generate checksum from file!");
			return {};
		}

		for (const auto& [handle, asset] : s_UIObjectRegistry)
		{
			if (asset.Data.CheckSum == currentCheckSum)
			{
				KG_CORE_INFO("Attempt to instantiate duplicate font asset");
				return handle;
			}
		}

		AssetHandle newHandle{};

		Asset newAsset{};
		newAsset.Handle = newHandle;

		CreateUIObjectFile(uiObjectName, newAsset);
		newAsset.Data.CheckSum = currentCheckSum;

		s_UIObjectRegistry.insert({ newHandle, newAsset }); // Update Registry Map in-memory
		SerializeUIObjectRegistry(); // Update Registry File on Disk

		return newHandle;
	}

	void AssetManager::SaveUIObject(AssetHandle uiObjectHandle, Ref<UIEngine::UIObject> uiObject)
	{
		if (!s_UIObjectRegistry.contains(uiObjectHandle))
		{
			KG_ERROR("Attempt to save uiObject that does not exist in registry");
			return;
		}
		Asset uiObjectAsset = s_UIObjectRegistry[uiObjectHandle];
		SerializeUIObject(uiObject, (Project::GetAssetDirectory() / uiObjectAsset.Data.IntermediateLocation).string());
	}

	std::filesystem::path AssetManager::GetUIObjectLocation(const AssetHandle& handle)
	{
		if (!s_UIObjectRegistry.contains(handle))
		{
			KG_ERROR("Attempt to save uiObject that does not exist in registry");
			return "";
		}
		return s_UIObjectRegistry[handle].Data.IntermediateLocation;
	}

	Ref<UIEngine::UIObject> AssetManager::GetUIObject(const AssetHandle& handle)
	{
		KG_CORE_ASSERT(Project::GetActive(), "There is no active project when retreiving uiObject!");

		if (s_UIObjectRegistry.contains(handle))
		{
			auto asset = s_UIObjectRegistry[handle];
			return InstantiateUIObject(asset);
		}

		KG_CORE_ERROR("No uiObject is associated with provided handle!");
		return nullptr;
	}
	std::tuple<AssetHandle, Ref<UIEngine::UIObject>> AssetManager::GetUIObject(const std::filesystem::path& filepath)
	{
		KG_CORE_ASSERT(Project::GetActive(), "Attempt to use Project Field without active project!");

		std::filesystem::path uiObjectPath = FileSystem::GetRelativePath(Project::GetAssetDirectory(), filepath);

		for (auto& [assetHandle, asset] : s_UIObjectRegistry)
		{
			if (asset.Data.IntermediateLocation.compare(uiObjectPath) == 0)
			{
				return std::make_tuple(assetHandle, InstantiateUIObject(asset));
			}
		}
		// Return empty uiObject if uiObject does not exist
		KG_CORE_WARN("No UIObject Associated with provided handle. Returned new empty uiObject");
		AssetHandle newHandle = CreateNewUIObject(filepath.stem().string());
		return std::make_tuple(newHandle, GetUIObject(newHandle));
	}

	Ref<UIEngine::UIObject> AssetManager::InstantiateUIObject(const Asset& uiObjectAsset)
	{
		Ref<UIEngine::UIObject> newUIObject = CreateRef<UIEngine::UIObject>();
		DeserializeUIObject(newUIObject, (Project::GetAssetDirectory() / uiObjectAsset.Data.IntermediateLocation).string());
		return newUIObject;
	}


	void AssetManager::ClearUIObjectRegistry()
	{
		s_UIObjectRegistry.clear();
	}

	void AssetManager::CreateUIObjectFile(const std::string& uiObjectName, Asset& newAsset)
	{
		// Create Temporary UIObject
		Ref<UIEngine::UIObject> temporaryUIObject = CreateRef<UIEngine::UIObject>();

		// Save Binary Intermediate into File
		std::string uiObjectPath = "UserInterface/" + uiObjectName + ".kgui";
		std::filesystem::path intermediateFullPath = Project::GetAssetDirectory() / uiObjectPath;
		SerializeUIObject(temporaryUIObject, intermediateFullPath.string());

		// Load data into In-Memory Metadata object
		newAsset.Data.Type = Assets::AssetType::UIObject;
		newAsset.Data.IntermediateLocation = uiObjectPath;
		Ref<UIObjectMetaData> metadata = CreateRef<UIObjectMetaData>();
		newAsset.Data.SpecificFileData = metadata;
	}


	bool AssetManager::SerializeProject(Ref<Project> project, const std::filesystem::path& filepath)
	{
		const auto& config = project->GetConfig();
		YAML::Emitter out;

		{
			out << YAML::BeginMap; // Root
			out << YAML::Key << "Project" << YAML::Value;
			{
				out << YAML::BeginMap; // Project
				out << YAML::Key << "Name" << YAML::Value << config.Name;
				out << YAML::Key << "StartScene" << YAML::Value << config.StartScenePath.string();
				out << YAML::Key << "StartSceneHandle" << YAML::Value << static_cast<uint64_t>(config.StartSceneHandle);
				out << YAML::Key << "AssetDirectory" << YAML::Value << config.AssetDirectory.string();
				out << YAML::Key << "ScriptModulePath" << YAML::Value << config.ScriptModulePath.string();
				out << YAML::Key << "DefaultFullscreen" << YAML::Value << config.DefaultFullscreen;
				out << YAML::Key << "TargetResolution" << YAML::Value << Utility::ScreenResolutionToString(config.TargetResolution);
				out << YAML::EndMap; // Project
			}

			out << YAML::EndMap; // Root
		}

		std::ofstream fout(filepath);
		fout << out.c_str();

		return true;
	}
	
	bool AssetManager::DeserializeProject(Ref<Project> project, const std::filesystem::path& filepath)
	{
		auto& config = project->GetConfig();

		YAML::Node data;
		try
		{
			data = YAML::LoadFile(filepath.string());
		}
		catch (YAML::ParserException e)
		{
			KG_CORE_ERROR("Failed to load project file '{0}'\n     {1}", filepath, e.what());
			return false;
		}
		auto projectNode = data["Project"];
		if (!projectNode) { return false; }

		config.Name = projectNode["Name"].as<std::string>();
		config.StartScenePath = projectNode["StartScene"].as<std::string>();
		config.StartSceneHandle = static_cast<AssetHandle>(projectNode["StartSceneHandle"].as<uint64_t>());
		config.AssetDirectory = projectNode["AssetDirectory"].as<std::string>();
		config.ScriptModulePath = projectNode["ScriptModulePath"].as<std::string>();
		config.DefaultFullscreen = projectNode["DefaultFullscreen"].as<bool>();
		config.TargetResolution = Utility::StringToScreenResolution(projectNode["TargetResolution"].as<std::string>());
		return true;
	}

}
