#include "kgpch.h"

#include "Kargono/Assets/AssetManager.h"

#include "Kargono/Core/FileSystem.h"
#include "Kargono/Core/Timers.h"
#include "Kargono/Projects/Project.h"
#include "Kargono/Renderer/Shader.h"
#include "Kargono/Scene/Scene.h"
#include "API/Serialization/SerializationAPI.h"
#include "Kargono/Script/ScriptEngine.h"
#include "Kargono/Scene/Entity.h"
#include "Kargono/Scene/Components.h"
#include "Kargono/Math/Math.h"

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>
#include <glad/glad.h>
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include <dr_wav.h>
#include "msdf-atlas-gen/msdf-atlas-gen.h"
#include "msdf-atlas-gen/FontGeometry.h"
#include "Kargono/Renderer/ShaderBuilder.h"

namespace Kargono::Utility
{
	static GLenum ShaderTypeFromString(const std::string& type)
	{
		if (type == "vertex")
			return GL_VERTEX_SHADER;
		if (type == "fragment" || type == "pixel")
			return GL_FRAGMENT_SHADER;

		KG_ASSERT(false, "Unknown shader type!");
		return 0;
	}

	static std::string ShaderTypeToString(GLenum stage)
	{
		switch (stage)
		{
		case GL_VERTEX_SHADER:   return "vertex";
		case GL_FRAGMENT_SHADER: return "fragment";
		}
		KG_ASSERT(false, "Invalid Shader Type!");
		return "";
	}

	static shaderc_shader_kind GLShaderStageToShaderC(GLenum stage)
	{
		switch (stage)
		{
		case GL_VERTEX_SHADER:   return shaderc_glsl_vertex_shader;
		case GL_FRAGMENT_SHADER: return shaderc_glsl_fragment_shader;
		}
		KG_ASSERT(false, "Invalid Shader Type!");
		return (shaderc_shader_kind)0;
	}

	static const char* GLShaderStageToString(GLenum stage)
	{
		switch (stage)
		{
		case GL_VERTEX_SHADER:   return "GL_VERTEX_SHADER";
		case GL_FRAGMENT_SHADER: return "GL_FRAGMENT_SHADER";
		}
		KG_ASSERT(false, "Invalid Shader Type!");
		return nullptr;
	}

	static const char* ShaderBinaryFileExtension(uint32_t stage)
	{
		switch (stage)
		{
		case GL_VERTEX_SHADER:    return ".kgshadervert";
		case GL_FRAGMENT_SHADER:  return ".kgshaderfrag";
		}
		KG_ASSERT(false);
		return "";
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
			KG_ASSERT(eol != std::string::npos, "Syntax error");
			size_t begin = pos + typeTokenLength + 1; //Start of shader type name (after "#type " keyword)
			std::string type = source.substr(begin, eol - begin);
			KG_ASSERT(Utility::ShaderTypeFromString(type), "Invalid shader type specified");

			size_t nextLinePos = source.find_first_not_of("\r\n", eol); //Start of shader code after shader type declaration line
			KG_ASSERT(nextLinePos != std::string::npos, "Syntax error");
			pos = source.find(typeToken, nextLinePos); //Start of next shader type declaration line

			shaderSources[Utility::ShaderTypeFromString(type)] = (pos == std::string::npos) ? source.substr(nextLinePos) : source.substr(nextLinePos, pos - nextLinePos);
		}

		return shaderSources;
	}

	static void CompileBinaries(const Assets::AssetHandle& assetHandle, const std::unordered_map<GLenum, std::string>& shaderSources, std::unordered_map<GLenum, std::vector<uint32_t>>& openGLSPIRV)
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
			std::string debugName = "Shader ID: " + static_cast<std::string>(assetHandle) + ", Shader Type: " + Utility::GLShaderStageToString(stage);
			// Compile SPIRV
			shaderc::SpvCompilationResult module;
			try
			{
				module = compiler.CompileGlslToSpv(source, Utility::GLShaderStageToShaderC(stage), debugName.c_str(), options);
			}
			catch (std::exception e)
			{
				KG_ERROR("Exception thrown inside shaderc!");
			}
			if (module.GetCompilationStatus() != shaderc_compilation_status_success)
			{

				KG_ERROR(module.GetErrorMessage());
				KG_ERROR("Here are the shaders: ");
				for (auto& [enumName, text] : shaderSources)
				{
					KG_ERROR(text);
				}
				KG_ASSERT(false);
			}
			// Add Newly Compiled Spirv to m_OpenGLSPIRV
			shaderData[stage] = std::vector<uint32_t>(module.cbegin(), module.cend());
		}
	}
}

namespace Kargono::Assets
{

	std::unordered_map<AssetHandle, Assets::Asset> AssetManager::s_ShaderRegistry {};
	std::unordered_map<AssetHandle, Ref<Kargono::Shader>> AssetManager::s_Shaders {};

	void AssetManager::DeserializeShaderRegistry()
	{
		// Clear current registry and open registry in current project 
		s_ShaderRegistry.clear();
		KG_ASSERT(Projects::Project::GetActive(), "There is no currently loaded project to serialize from!");
		const auto& shaderRegistryLocation = Projects::Project::GetAssetDirectory() / "Shaders/Intermediates/ShaderRegistry.kgreg";

		if (!std::filesystem::exists(shaderRegistryLocation))
		{
			KG_ERROR("No .kgregistry file exists in project path!");
			return;
		}
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(shaderRegistryLocation.string());
		}
		catch (YAML::ParserException e)
		{
			KG_ERROR("Failed to load .kgreg file '{0}'\n     {1}", shaderRegistryLocation.string(), e.what());
			return;
		}
		// Opening registry node 
		if (!data["Registry"]) { return; }

		std::string registryName = data["Registry"].as<std::string>();
		KG_INFO("Deserializing Shader Registry");
		// Opening all assets 
		auto assets = data["Assets"];
		if (assets)
		{
			for (auto asset : assets)
			{
				Assets::Asset newAsset{};
				newAsset.Handle = asset["AssetHandle"].as<uint64_t>();
				// Retrieving metadata for asset 
				auto metadata = asset["MetaData"];
				newAsset.Data.CheckSum = metadata["CheckSum"].as<std::string>();
				newAsset.Data.IntermediateLocation = metadata["IntermediateLocation"].as<std::string>();
				newAsset.Data.Type = Utility::StringToAssetType(metadata["AssetType"].as<std::string>());
				// Retrieving shader specific metadata 
				if (newAsset.Data.Type == Assets::Shader)
				{
					Ref<Assets::ShaderMetaData> shaderMetaData = CreateRef<Assets::ShaderMetaData>();

					// ShaderSpecification Section
					shaderMetaData->ShaderSpec.ColorInput = Utility::StringToColorInputType(metadata["ColorInputType"].as<std::string>());
					shaderMetaData->ShaderSpec.AddProjectionMatrix = metadata["AddProjectionMatrix"].as<bool>();
					shaderMetaData->ShaderSpec.AddEntityID = metadata["AddEntityID"].as<bool>();
					shaderMetaData->ShaderSpec.AddCircleShape = metadata["AddCircleShape"].as<bool>();
					shaderMetaData->ShaderSpec.TextureInput = Utility::StringToTextureInputType(metadata["TextureInput"].as<std::string>());
					shaderMetaData->ShaderSpec.DrawOutline = metadata["DrawOutline"].as<bool>();
					shaderMetaData->ShaderSpec.RenderType = Utility::StringToRenderingType(metadata["RenderType"].as<std::string>());

					KG_ASSERT(sizeof(uint8_t) * 20 == sizeof(ShaderSpecification), "Please Update Deserialization and Serialization. Incorrect size of input data in Shader Deserializer!")
					{
						// InputBufferLayout Section
						auto inputBufferLayout = metadata["InputBufferLayout"];
						auto elementList = inputBufferLayout["Elements"];
						for (const auto& element : elementList)
						{
							shaderMetaData->InputLayout.AddBufferElement(InputBufferElement(
								Utility::StringToInputDataType(element["Type"].as<std::string>()),
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
								Utility::StringToUniformDataType(element["Type"].as<std::string>()),
								element["Name"].as<std::string>()
							));
						}
					}
					newAsset.Data.SpecificFileData = shaderMetaData;
				}
				// Add asset to in memory registry 
				s_ShaderRegistry.insert({ newAsset.Handle, newAsset });

			}
		}
	}

	void AssetManager::SerializeShaderRegistry()
	{
		KG_ASSERT(Projects::Project::GetActive(), "There is no currently loaded project to serialize to!");
		const auto& shaderRegistryLocation = Projects::Project::GetAssetDirectory() / "Shaders/Intermediates/ShaderRegistry.kgreg";
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
			out << YAML::Key << "AssetType" << YAML::Value << Utility::AssetTypeToString(asset.Data.Type);

			if (asset.Data.Type == Assets::AssetType::Shader)
			{
				// ShaderSpecification Section
				Assets::ShaderMetaData* metadata = static_cast<Assets::ShaderMetaData*>(asset.Data.SpecificFileData.get());
				out << YAML::Key << "ColorInputType" << YAML::Value << Utility::ColorInputTypeToString(metadata->ShaderSpec.ColorInput);
				out << YAML::Key << "AddProjectionMatrix" << YAML::Value << metadata->ShaderSpec.AddProjectionMatrix;
				out << YAML::Key << "AddEntityID" << YAML::Value << metadata->ShaderSpec.AddEntityID;
				out << YAML::Key << "AddCircleShape" << YAML::Value << metadata->ShaderSpec.AddCircleShape;
				out << YAML::Key << "TextureInput" << YAML::Value << Utility::TextureInputTypeToString(metadata->ShaderSpec.TextureInput);
				out << YAML::Key << "DrawOutline" << YAML::Value << metadata->ShaderSpec.DrawOutline;
				out << YAML::Key << "RenderType" << YAML::Value << Utility::RenderingTypeToString(metadata->ShaderSpec.RenderType);

				// InputBufferLayout Section
				out << YAML::Key << "InputBufferLayout" << YAML::Value << YAML::BeginMap; // Input Buffer Layout Map
				out << YAML::Key << "Elements" << YAML::Value << YAML::BeginSeq;
				for (const auto& element : metadata->InputLayout.GetElements())
				{
					out << YAML::BeginMap; // Input Element Map
					out << YAML::Key << "Name" << YAML::Value << element.Name;
					out << YAML::Key << "Type" << YAML::Value << Utility::InputDataTypeToString(element.Type);
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
					out << YAML::Key << "Type" << YAML::Value << Utility::UniformDataTypeToString(element.Type);
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

	AssetHandle AssetManager::CreateNewShader(const ShaderSpecification& shaderSpec)
	{
		// Create Checksum
		auto [shaderSource, bufferLayout, uniformList] = ShaderBuilder::BuildShader(shaderSpec);
		std::string currentCheckSum = FileSystem::ChecksumFromString(shaderSource);

		if (currentCheckSum.empty())
		{
			KG_ERROR("Failed to generate checksum from shaderSpec!");
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

		// If Asset is not a duplicate but has a similar spec to another asset, replace that asset! (Delete old asset and use it's UUID)
		if (isAssetDuplicate)
		{
			return currentHandle;
		}
		// Create New Asset and Asset Handle
		AssetHandle newHandle{};

		Assets::Asset newAsset{};
		newAsset.Handle = newHandle;

		// Create intermediate and save intermediate to disk
		CreateShaderIntermediate(shaderSource, newAsset, shaderSpec, bufferLayout, uniformList);
		newAsset.Data.CheckSum = currentCheckSum;

		// Register New Asset and Create Shader
		s_ShaderRegistry.insert({ newHandle, newAsset }); // Update Registry Map in-memory
		SerializeShaderRegistry(); // Update Registry File on Disk
		Ref<Kargono::Shader> newShader = InstantiateShaderIntoMemory(newAsset);
		s_Shaders.insert({ newHandle, newShader });

		return newHandle;
	}

	Ref<Kargono::Shader> AssetManager::GetShader(const AssetHandle& handle)
	{
		KG_ASSERT(Projects::Project::GetActive(), "There is no active project when retrieving shader!");

		if (s_Shaders.contains(handle)) { return s_Shaders[handle]; }

		if (s_ShaderRegistry.contains(handle))
		{
			auto asset = s_ShaderRegistry[handle];

			Ref<Kargono::Shader> newShader = InstantiateShaderIntoMemory(asset);
			s_Shaders.insert({ asset.Handle, newShader });
			return newShader;
		}

		//KG_CORE_INFO("No Shader is associated with provided asset handle!");
		return nullptr;
	}

	std::tuple<AssetHandle, Ref<Kargono::Shader>> AssetManager::GetShader(const ShaderSpecification& shaderSpec)
	{
		KG_ASSERT(Projects::Project::GetActive(), "There is no active project when retrieving shader!");

		for (const auto& [assetHandle, shaderRef] : s_Shaders)
		{
			if (shaderRef->GetSpecification() == shaderSpec)
			{
				return std::make_tuple(assetHandle, shaderRef);
			}
		}

		for (auto& [assetHandle, asset] : s_ShaderRegistry)
		{
			Assets::ShaderMetaData metadata = *static_cast<Assets::ShaderMetaData*>(asset.Data.SpecificFileData.get());
			if (metadata.ShaderSpec == shaderSpec)
			{
				Ref<Kargono::Shader> newShader = InstantiateShaderIntoMemory(asset);
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

	Ref<Kargono::Shader> AssetManager::InstantiateShaderIntoMemory(Assets::Asset& asset)
	{
		Assets::ShaderMetaData metadata = *static_cast<Assets::ShaderMetaData*>(asset.Data.SpecificFileData.get());
		std::unordered_map<GLenum, std::vector<uint32_t>> openGLSPIRV;
		std::filesystem::path intermediatePath = Projects::Project::GetAssetDirectory() / asset.Data.IntermediateLocation;
		std::vector<std::string> stageTypes = { "vertex", "fragment" };

		for (const auto& stage : stageTypes)
		{
			std::filesystem::path fullPath = intermediatePath.string() + Utility::ShaderBinaryFileExtension(Utility::ShaderTypeFromString(stage));

			std::ifstream in(fullPath, std::ios::in | std::ios::binary);

			if (in.is_open())
			{
				in.seekg(0, std::ios::end);
				auto size = in.tellg();
				in.seekg(0, std::ios::beg);

				auto& data = openGLSPIRV[Utility::ShaderTypeFromString(stage)];
				data.resize(size / sizeof(uint32_t));
				in.read((char*)data.data(), size);
			}
		}

		Ref<Kargono::Shader> newShader = Shader::Create(static_cast<std::string>(asset.Handle), openGLSPIRV);
		newShader->SetSpecification(metadata.ShaderSpec);
		newShader->SetInputLayout(metadata.InputLayout);
		newShader->SetUniformList(metadata.UniformList);
		openGLSPIRV.clear();
		return newShader;
	}


	void AssetManager::CreateShaderIntermediate(const ShaderSource& shaderSource, Assets::Asset& newAsset, const ShaderSpecification& shaderSpec,
		const InputBufferLayout& inputLayout, const UniformBufferList& uniformLayout)
	{
		// Create Shader Binary
		auto shaderSources = Utility::PreProcess(shaderSource);

		std::unordered_map<GLenum, std::vector<uint32_t>> openGLSPIRV;
		Utility::CompileBinaries(newAsset.Handle, shaderSources, openGLSPIRV);

		// Save binary intermediates for all shader stages!
		std::string intermediatePath = "Shaders/Intermediates/" + (std::string)newAsset.Handle;
		for (const auto& [stage, source] : openGLSPIRV)
		{
			std::string intermediatePathWithExtension = intermediatePath + Utility::ShaderBinaryFileExtension(stage);
			std::filesystem::path intermediateFullPath = Projects::Project::GetAssetDirectory() / intermediatePathWithExtension;

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
		std::filesystem::path debugPath = Projects::Project::GetAssetDirectory() / (intermediatePath + ".source");
		FileSystem::WriteFileString(debugPath, debugString);
#endif

		// Load In-Memory Metadata Object
		newAsset.Data.Type = Assets::AssetType::Shader;
		newAsset.Data.IntermediateLocation = intermediatePath;
		Ref<Assets::ShaderMetaData> metadata = CreateRef<Assets::ShaderMetaData>();
		metadata->ShaderSpec = shaderSpec;
		metadata->InputLayout = inputLayout;
		metadata->UniformList = uniformLayout;
		newAsset.Data.SpecificFileData = metadata;
	}

}

namespace Kargono::Utility
{
#pragma once
#define OpenAL_ErrorCheck(message)\
		{\
			ALenum error = alGetError();\
			if( error != AL_NO_ERROR)\
			{\
				KG_ERROR("OpenAL Error: {} with call for {}", error, #message);\
			}\
		}

#define alec(FUNCTION_CALL)\
		FUNCTION_CALL;\
		OpenAL_ErrorCheck(FUNCTION_CALL)
}

namespace Kargono::Assets
{

	std::unordered_map<AssetHandle, Assets::Asset> AssetManager::s_AudioRegistry {};
	std::unordered_map<AssetHandle, Ref<Audio::AudioBuffer>> AssetManager::s_Audio {};

	void AssetManager::DeserializeAudioRegistry()
	{
		// Clear current registry and open registry in current project 
		s_AudioRegistry.clear();
		KG_ASSERT(Projects::Project::GetActive(), "There is no currently loaded project to serialize from!");
		const auto& audioRegistryLocation = Projects::Project::GetAssetDirectory() / "Audio/Intermediates/AudioRegistry.kgreg";

		if (!std::filesystem::exists(audioRegistryLocation))
		{
			KG_ERROR("No .kgregistry file exists in project path!");
			return;
		}
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(audioRegistryLocation.string());
		}
		catch (YAML::ParserException e)
		{
			KG_ERROR("Failed to load .kgscene file '{0}'\n     {1}", audioRegistryLocation.string(), e.what());
			return;
		}
		// Opening registry node 
		if (!data["Registry"]) { return; }

		std::string registryName = data["Registry"].as<std::string>();
		KG_INFO("Deserializing Audio Registry");
		// Opening all assets 
		auto assets = data["Assets"];
		if (assets)
		{
			for (auto asset : assets)
			{
				Assets::Asset newAsset{};
				newAsset.Handle = asset["AssetHandle"].as<uint64_t>();

				// Retrieving metadata for asset 
				auto metadata = asset["MetaData"];
				newAsset.Data.CheckSum = metadata["CheckSum"].as<std::string>();
				newAsset.Data.IntermediateLocation = metadata["IntermediateLocation"].as<std::string>();
				newAsset.Data.Type = Utility::StringToAssetType(metadata["AssetType"].as<std::string>());

				// Retrieving audio specific metadata 
				if (newAsset.Data.Type == Assets::Audio)
				{
					Ref<Assets::AudioMetaData> audioMetaData = CreateRef<Assets::AudioMetaData>();

					audioMetaData->Channels = metadata["Channels"].as<uint32_t>();
					audioMetaData->SampleRate = metadata["SampleRate"].as<uint32_t>();
					audioMetaData->TotalPcmFrameCount = metadata["TotalPcmFrameCount"].as<uint64_t>();
					audioMetaData->TotalSize = metadata["TotalSize"].as<uint64_t>();
					audioMetaData->InitialFileLocation = metadata["InitialFileLocation"].as<std::string>();

					newAsset.Data.SpecificFileData = audioMetaData;
				}

				// Add asset to in memory registry 
				s_AudioRegistry.insert({ newAsset.Handle, newAsset });

			}
		}
	}

	void AssetManager::SerializeAudioRegistry()
	{
		KG_ASSERT(Projects::Project::GetActive(), "There is no currently loaded project to serialize to!");
		const auto& audioRegistryLocation = Projects::Project::GetAssetDirectory() / "Audio/Intermediates/AudioRegistry.kgreg";
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
			out << YAML::Key << "AssetType" << YAML::Value << Utility::AssetTypeToString(asset.Data.Type);

			if (asset.Data.Type == Assets::AssetType::Audio)
			{
				Assets::AudioMetaData* metadata = static_cast<Assets::AudioMetaData*>(asset.Data.SpecificFileData.get());
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
			KG_ERROR("Failed to generate checksum from file!");
			return {};
		}

		for (const auto& [handle, asset] : s_AudioRegistry)
		{
			if (asset.Data.CheckSum == currentCheckSum)
			{
				KG_INFO("Attempt to instantiate duplicate audio asset");
				return handle;
			}
		}

		// Create New Asset/Handle
		AssetHandle newHandle{};

		Assets::Asset newAsset{};
		newAsset.Handle = newHandle;

		// Create Intermediate
		CreateAudioIntermediateFromFile(filePath, newAsset);
		newAsset.Data.CheckSum = currentCheckSum;

		// Register New Asset and Create Texture
		s_AudioRegistry.insert({ newHandle, newAsset }); // Update Registry Map in-memory
		SerializeAudioRegistry(); // Update Registry File on Disk

		s_Audio.insert({ newHandle, InstantiateAudioIntoMemory(newAsset) });

		return newHandle;
	}

	Ref<Audio::AudioBuffer> AssetManager::InstantiateAudioIntoMemory(Assets::Asset& asset)
	{
		Assets::AudioMetaData metadata = *static_cast<Assets::AudioMetaData*>(asset.Data.SpecificFileData.get());
		Buffer currentResource{};
		currentResource = FileSystem::ReadFileBinary(Projects::Project::GetAssetDirectory() / asset.Data.IntermediateLocation);
		Ref<Audio::AudioBuffer> newAudio = CreateRef<Audio::AudioBuffer>();
		alec(alBufferData(newAudio->m_BufferID, metadata.Channels > 1 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16, currentResource.Data, static_cast<ALsizei>(currentResource.Size), metadata.SampleRate));

		currentResource.Release();
		return newAudio;
	}

	Ref<Audio::AudioBuffer> AssetManager::GetAudio(const AssetHandle& handle)
	{
		KG_ASSERT(Projects::Project::GetActive(), "There is no active project when retreiving audio!");

		if (s_Audio.contains(handle)) { return s_Audio[handle]; }

		if (s_AudioRegistry.contains(handle))
		{
			auto asset = s_AudioRegistry[handle];

			Ref<Audio::AudioBuffer> newAudio = InstantiateAudioIntoMemory(asset);
			s_Audio.insert({ asset.Handle, newAudio });
			return newAudio;
		}

		KG_ERROR("No audio is associated with provided handle!");
		return nullptr;
	}

	std::tuple<AssetHandle, Ref<Audio::AudioBuffer>> AssetManager::GetAudio(const std::filesystem::path& filepath)
	{
		KG_ASSERT(Projects::Project::GetActive(), "Attempt to use Project Field without active project!");

		for (auto& [assetHandle, asset] : s_AudioRegistry)
		{
			auto metadata = (Assets::AudioMetaData*)asset.Data.SpecificFileData.get();
			if (metadata->InitialFileLocation.compare(filepath) == 0)
			{
				return std::make_tuple(assetHandle, GetAudio(assetHandle));
			}
		}
		// Return empty audio if audio does not exist
		KG_INFO("Invalid filepath provided to GetAudio {}", filepath.string());
		return std::make_tuple(0, nullptr);
	}

	void AssetManager::ClearAudioRegistry()
	{
		s_AudioRegistry.clear();
		s_Audio.clear();
	}

	void AssetManager::CreateAudioIntermediateFromFile(const std::filesystem::path& filePath, Assets::Asset& newAsset)
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
			KG_ERROR("Failed to load audio file");
			drwav_free(pSampleData, nullptr);
			return;
		}
		totalSize = totalPcmFrameCount * channels * 2;
		if ((totalSize) > drwav_uint64(std::numeric_limits<size_t>::max()))
		{
			KG_ERROR("Too much data in file for 32bit addressed vector");
			drwav_free(pSampleData, nullptr);
			return;
		}
		pcmData.Allocate(size_t(totalSize));
		std::memcpy(pcmData.Data, pSampleData, pcmData.Size /*two bytes in s16*/);
		drwav_free(pSampleData, nullptr);

		// Save Binary Intermediate into File
		std::string intermediatePath = "Audio/Intermediates/" + (std::string)newAsset.Handle + ".kgaudio";
		std::filesystem::path intermediateFullPath = Projects::Project::GetAssetDirectory() / intermediatePath;
		FileSystem::WriteFileBinary(intermediateFullPath, pcmData);

		// Check that save was successful
		if (!pcmData)
		{
			KG_ERROR("Failed to load data from file in audio importer!");
			return;
		}

		// Load data into In-Memory Metadata object
		newAsset.Data.Type = Assets::AssetType::Audio;
		newAsset.Data.IntermediateLocation = intermediatePath;
		Ref<Assets::AudioMetaData> metadata = CreateRef<Assets::AudioMetaData>();
		metadata->Channels = channels;
		metadata->SampleRate = sampleRate;;
		metadata->TotalPcmFrameCount = totalPcmFrameCount;
		metadata->TotalSize = totalSize;
		metadata->InitialFileLocation = FileSystem::GetRelativePath(Projects::Project::GetAssetDirectory(), filePath);
		newAsset.Data.SpecificFileData = metadata;
		pcmData.Release();
	}
}

namespace Kargono::Utility
{
	template<typename T, typename S, int32_t N, msdf_atlas::GeneratorFunction<S, N> GenFunc>
	static void CreateAtlas(const std::string& fontName, float fontSize, const std::vector<msdf_atlas::GlyphGeometry>& glyphs,
		const msdf_atlas::FontGeometry& fontGeometry, uint32_t width, uint32_t height, TextureSpecification& textureSpec, Buffer& buffer)
	{
		uint32_t numAvailableThread = std::thread::hardware_concurrency() / 2;
		msdf_atlas::GeneratorAttributes attributes;
		attributes.config.overlapSupport = true;
		attributes.scanlinePass = true;
		msdf_atlas::ImmediateAtlasGenerator<S, N, GenFunc, msdf_atlas::BitmapAtlasStorage<T, N>> generator(width, height);
		generator.setAttributes(attributes);
		generator.setThreadCount(numAvailableThread);
		generator.generate(glyphs.data(), (int32_t)glyphs.size());

		msdfgen::BitmapConstRef<T, N> bitmap = (msdfgen::BitmapConstRef<T, N>)generator.atlasStorage();

		textureSpec.Width = bitmap.width;
		textureSpec.Height = bitmap.height;
		textureSpec.Format = ImageFormat::RGB8;
		textureSpec.GenerateMipMaps = false;

		buffer.Allocate(bitmap.width * bitmap.height * Utility::ImageFormatToBytes(textureSpec.Format));
		memcpy_s(buffer.Data, buffer.Size, bitmap.pixels, bitmap.width * bitmap.height * Utility::ImageFormatToBytes(textureSpec.Format));
	}


	/*template<typename T, typename S, int32_t N, msdf_atlas::GeneratorFunction<S, N> GenFunc>
	static Ref<Texture2D> CreateAndCacheAtlas(const std::string& fontName, float fontSize, const std::vector<msdf_atlas::GlyphGeometry>& glyphs,
		const msdf_atlas::FontGeometry& fontGeometry, uint32_t width, uint32_t height)
	{
		uint32_t numAvailableThread = std::thread::hardware_concurrency() / 2;
		msdf_atlas::GeneratorAttributes attributes;
		attributes.config.overlapSupport = true;
		attributes.scanlinePass = true;
		msdf_atlas::ImmediateAtlasGenerator<S, N, GenFunc, msdf_atlas::BitmapAtlasStorage<T, N>> generator(width, height);
		generator.setAttributes(attributes);
		generator.setThreadCount(numAvailableThread);
		generator.generate(glyphs.data(), (int32_t)glyphs.size());

		msdfgen::BitmapConstRef<T, N> bitmap = (msdfgen::BitmapConstRef<T, N>)generator.atlasStorage();

		TextureSpecification spec;
		spec.Width = bitmap.width;
		spec.Height = bitmap.height;
		spec.Format = ImageFormat::RGB8;
		spec.GenerateMipMaps = false;
		Ref<Texture2D> texture = Texture2D::Create(spec);
		texture->SetData((void*)bitmap.pixels, bitmap.width * bitmap.height * Utility::ImageFormatToBytes(spec.Format));
		return texture;
	}*/

}

namespace Kargono::Assets
{
	std::unordered_map<AssetHandle, Assets::Asset> AssetManager::s_FontRegistry {};
	std::unordered_map<AssetHandle, Ref<RuntimeUI::Font>> AssetManager::s_Fonts {};

	void AssetManager::DeserializeFontRegistry()
	{
		// Clear current registry and open registry in current project 
		s_FontRegistry.clear();
		KG_ASSERT(Projects::Project::GetActive(), "There is no currently loaded project to serialize from!");
		const auto& fontRegistryLocation = Projects::Project::GetAssetDirectory() / "Fonts/Intermediates/FontRegistry.kgreg";

		if (!std::filesystem::exists(fontRegistryLocation))
		{
			KG_ERROR("No .kgregistry file exists in project path!");
			return;
		}
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(fontRegistryLocation.string());
		}
		catch (YAML::ParserException e)
		{
			KG_ERROR("Failed to load .kgscene file '{0}'\n     {1}", fontRegistryLocation.string(), e.what());
			return;
		}

		// Opening registry node 
		if (!data["Registry"]) { return; }

		std::string registryName = data["Registry"].as<std::string>();
		KG_INFO("Deserializing Font Registry");

		// Opening all assets 
		auto assets = data["Assets"];
		if (assets)
		{
			for (auto asset : assets)
			{
				Assets::Asset newAsset{};
				newAsset.Handle = asset["AssetHandle"].as<uint64_t>();

				// Retrieving metadata for asset 
				auto metadata = asset["MetaData"];
				newAsset.Data.CheckSum = metadata["CheckSum"].as<std::string>();
				newAsset.Data.IntermediateLocation = metadata["IntermediateLocation"].as<std::string>();
				newAsset.Data.Type = Utility::StringToAssetType(metadata["AssetType"].as<std::string>());

				// Retrieving font specific metadata 
				if (newAsset.Data.Type == Assets::Font)
				{
					Ref<Assets::FontMetaData> fontMetaData = CreateRef<Assets::FontMetaData>();

					fontMetaData->AtlasWidth = metadata["AtlasWidth"].as<float>();
					fontMetaData->AtlasHeight = metadata["AtlasHeight"].as<float>();
					fontMetaData->LineHeight = metadata["LineHeight"].as<float>();

					auto characters = metadata["Characters"];
					auto& characterVector = fontMetaData->Characters;
					for (auto character : characters)
					{
						RuntimeUI::Character newCharacter{};
						newCharacter.Size = character["Size"].as<Math::vec2>();
						newCharacter.Advance = character["Advance"].as<float>();
						newCharacter.TexCoordinateMin = character["TexCoordinateMin"].as<Math::vec2>();
						newCharacter.TexCoordinateMax = character["TexCoordinateMax"].as<Math::vec2>();
						newCharacter.QuadMin = character["QuadMin"].as<Math::vec2>();
						newCharacter.QuadMax = character["QuadMax"].as<Math::vec2>();
						characterVector.push_back(std::pair<unsigned char, RuntimeUI::Character>(static_cast<uint8_t>(character["Character"].as<uint32_t>()), newCharacter));
					}

					fontMetaData->InitialFileLocation = metadata["InitialFileLocation"].as<std::string>();
					newAsset.Data.SpecificFileData = fontMetaData;

				}

				// Add asset to in memory registry 
				s_FontRegistry.insert({ newAsset.Handle, newAsset });

			}
		}
	}

	void AssetManager::SerializeFontRegistry()
	{
		KG_ASSERT(Projects::Project::GetActive(), "There is no currently loaded project to serialize to!");
		const auto& fontRegistryLocation = Projects::Project::GetAssetDirectory() / "Fonts/Intermediates/FontRegistry.kgreg";
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
			out << YAML::Key << "AssetType" << YAML::Value << Utility::AssetTypeToString(asset.Data.Type);

			if (asset.Data.Type == Assets::AssetType::Font)
			{
				Assets::FontMetaData* metadata = static_cast<Assets::FontMetaData*>(asset.Data.SpecificFileData.get());

				out << YAML::Key << "InitialFileLocation" << YAML::Value << metadata->InitialFileLocation.string();

				out << YAML::Key << "AtlasWidth" << YAML::Value << metadata->AtlasWidth;
				out << YAML::Key << "AtlasHeight" << YAML::Value << metadata->AtlasHeight;
				out << YAML::Key << "LineHeight" << YAML::Value << metadata->LineHeight;

				out << YAML::Key << "Characters" << YAML::Value << YAML::BeginSeq;
				for (auto& [character, characterStruct] : metadata->Characters)
				{
					out << YAML::BeginMap;
					out << YAML::Key << "Character" << YAML::Value << static_cast<uint32_t>(character);
					out << YAML::Key << "Size" << YAML::Value << characterStruct.Size;
					out << YAML::Key << "TexCoordinateMin" << YAML::Value << characterStruct.TexCoordinateMin;
					out << YAML::Key << "TexCoordinateMax" << YAML::Value << characterStruct.TexCoordinateMax;
					out << YAML::Key << "QuadMin" << YAML::Value << characterStruct.QuadMin;
					out << YAML::Key << "QuadMax" << YAML::Value << characterStruct.QuadMax;
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
			KG_ERROR("Failed to generate checksum from file!");
			return {};
		}

		// Compare currentChecksum to registered assets
		for (const auto& [handle, asset] : s_FontRegistry)
		{
			if (asset.Data.CheckSum == currentCheckSum)
			{
				KG_INFO("Attempt to instantiate duplicate font asset");
				return handle;
			}
		}

		// Create New Asset/Handle
		AssetHandle newHandle{};
		Assets::Asset newAsset{};
		newAsset.Handle = newHandle;

		// Create Intermediate
		CreateFontIntermediateFromFile(filePath, newAsset);
		newAsset.Data.CheckSum = currentCheckSum;

		// Register New Asset and Create Font
		s_FontRegistry.insert({ newHandle, newAsset }); // Update Registry Map in-memory
		SerializeFontRegistry(); // Update Registry File on Disk

		s_Fonts.insert({ newHandle, InstantiateFontIntoMemory(newAsset) });

		return newHandle;
	}

	Ref<RuntimeUI::Font> AssetManager::InstantiateFontIntoMemory(Assets::Asset& asset)
	{
		Assets::FontMetaData metadata = *static_cast<Assets::FontMetaData*>(asset.Data.SpecificFileData.get());
		Buffer currentResource{};
		currentResource = FileSystem::ReadFileBinary(Projects::Project::GetAssetDirectory() / asset.Data.IntermediateLocation);
		Ref<RuntimeUI::Font> newFont = CreateRef<RuntimeUI::Font>();
		auto& fontCharacters = newFont->GetCharacters();

		// Create Texture
		TextureSpecification spec;
		spec.Width = static_cast<uint32_t>(metadata.AtlasWidth);
		spec.Height = static_cast<uint32_t>(metadata.AtlasHeight);
		spec.Format = ImageFormat::RGB8;
		spec.GenerateMipMaps = false;
		Ref<Texture2D> texture = Texture2D::Create(spec);
		texture->SetData((void*)currentResource.Data, spec.Width * spec.Height * Utility::ImageFormatToBytes(spec.Format));
		newFont->m_AtlasTexture = texture;

		newFont->SetLineHeight(metadata.LineHeight);

		for (auto& [character, characterStruct] : metadata.Characters)
		{
			fontCharacters.insert(std::pair<unsigned char, RuntimeUI::Character>(character, characterStruct));
		}

		currentResource.Release();
		return newFont;

	}

	Ref<RuntimeUI::Font> AssetManager::GetFont(const AssetHandle& handle)
	{
		KG_ASSERT(Projects::Project::GetActive(), "There is no active project when retreiving font!");

		if (s_Fonts.contains(handle)) { return s_Fonts[handle]; }

		if (s_FontRegistry.contains(handle))
		{
			auto asset = s_FontRegistry[handle];

			Ref<RuntimeUI::Font> newFont = InstantiateFontIntoMemory(asset);
			s_Fonts.insert({ asset.Handle, newFont });
			return newFont;
		}

		KG_ERROR("No font is associated with provided handle!");
		return nullptr;
	}

	void AssetManager::ClearFontRegistry()
	{
		s_FontRegistry.clear();
		s_Fonts.clear();
	}

	void AssetManager::CreateFontIntermediateFromFile(const std::filesystem::path& filePath, Assets::Asset& newAsset)
	{
		// Create Buffers
		std::vector<msdf_atlas::GlyphGeometry> glyphs;
		msdf_atlas::FontGeometry fontGeometry;
		float lineHeight{ 0 };
		std::vector<std::pair<unsigned char, RuntimeUI::Character>> characters {};

		msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype();
		KG_ASSERT(ft, "MSDFGEN failed to initialize!");

		std::string fileString = filePath.string();
		msdfgen::FontHandle* font = msdfgen::loadFont(ft, fileString.c_str());
		if (!font)
		{
			KG_ERROR("Font not loaded correctly from filepath: " + filePath.string());
			return;
		}

		struct CharsetRange
		{
			uint32_t Begin, End;
		};

		// From imgui_draw.cpp
		static const CharsetRange charsetRanges[] =
		{
			{0x0020, 0x00FF}
		};

		msdf_atlas::Charset charset;
		for (CharsetRange range : charsetRanges)
		{
			for (uint32_t character = range.Begin; character <= range.End; character++)
			{
				charset.add(character);
			}
		}

		double fontScale = 1.0;
		fontGeometry = msdf_atlas::FontGeometry(&glyphs);
		int glyphsLoaded = fontGeometry.loadCharset(font, fontScale, charset);
		KG_INFO("Loaded {} glyphs from font (out of {})", glyphsLoaded, charset.size());

		double emSize = 40.0;

		msdf_atlas::TightAtlasPacker atlasPacker;
		// atlasPacker.setDimensionsConstraint();
		atlasPacker.setPixelRange(2.0);
		atlasPacker.setMiterLimit(1.0);
		atlasPacker.setPadding(0);
		atlasPacker.setScale(emSize);
		int32_t remaining = atlasPacker.pack(glyphs.data(), (int32_t)glyphs.size());
		KG_ASSERT(remaining == 0);

		int32_t width, height;
		atlasPacker.getDimensions(width, height);
		emSize = atlasPacker.getScale();
		uint32_t numAvailableThread = std::thread::hardware_concurrency() / 2;
#define DEFAULT_ANGLE_THRESHOLD 3.0
#define LCG_MULTIPLIER 6364136223846793005ull
#define LCG_INCREMENT 1442695040888963407ull

		// if MSDF || MTSDF
		uint64_t coloringSeed = 0;
		bool expensiveColoring = false;
		if (expensiveColoring)
		{
			msdf_atlas::Workload([&glyphs = glyphs, &coloringSeed](int i, int threadNo) -> bool {
				unsigned long long glyphSeed = (LCG_MULTIPLIER * (coloringSeed ^ i) + LCG_INCREMENT) * !!coloringSeed;
				glyphs[i].edgeColoring(msdfgen::edgeColoringInkTrap, DEFAULT_ANGLE_THRESHOLD, glyphSeed);
				return true;
				}, static_cast<int32_t>(glyphs.size())).finish(numAvailableThread);
		}
		else {
			unsigned long long glyphSeed = coloringSeed;
			for (msdf_atlas::GlyphGeometry& glyph : glyphs)
			{
				glyphSeed *= LCG_MULTIPLIER;
				glyph.edgeColoring(msdfgen::edgeColoringInkTrap, DEFAULT_ANGLE_THRESHOLD, glyphSeed);
			}
		}
		Buffer buffer{};
		TextureSpecification textureSpec{};
		Utility::CreateAtlas<uint8_t, float, 3, msdf_atlas::msdfGenerator>("Test", (float)emSize, glyphs, fontGeometry, width, height, textureSpec, buffer);

		msdfgen::destroyFont(font);
		msdfgen::deinitializeFreetype(ft);

		const auto& metrics = fontGeometry.getMetrics();
		lineHeight = static_cast<float>(metrics.lineHeight);

		const auto& glyphMetrics = fontGeometry.getGlyphs();
		for (auto& glyphGeometry : glyphMetrics)
		{
			unsigned char character = static_cast<uint8_t>(glyphGeometry.getCodepoint());
			RuntimeUI::Character characterStruct{};

			// Fill the texture location inside Atlas
			double al, ab, ar, at;
			glyphGeometry.getQuadAtlasBounds(al, ab, ar, at);
			characterStruct.TexCoordinateMin = { (float)al, (float)ab };
			characterStruct.TexCoordinateMax = { (float)ar, (float)at };
			// Fill the Bounding Box Size when Rendering
			double pl, pb, pr, pt;
			glyphGeometry.getQuadPlaneBounds(pl, pb, pr, pt);
			characterStruct.QuadMin = { (float)pl, (float)pb };
			characterStruct.QuadMax = { (float)pr, (float)pt };
			// Fill the Advance
			characterStruct.Advance = (float)glyphGeometry.getAdvance();
			// Fill Glyph Size
			int32_t glyphWidth, glyphHeight;
			glyphGeometry.getBoxSize(glyphWidth, glyphHeight);
			characterStruct.Size = { glyphWidth, glyphHeight };
			characters.push_back({ character, characterStruct });
		}

		// Save Binary Intermediate into File
		std::string intermediatePath = "Fonts/Intermediates/" + (std::string)newAsset.Handle + ".kgfont";
		std::filesystem::path intermediateFullPath = Projects::Project::GetAssetDirectory() / intermediatePath;
		FileSystem::WriteFileBinary(intermediateFullPath, buffer);

		// Load data into In-Memory Metadata object
		newAsset.Data.Type = Assets::AssetType::Font;
		newAsset.Data.IntermediateLocation = intermediatePath;
		Ref<Assets::FontMetaData> metadata = CreateRef<Assets::FontMetaData>();
		metadata->AtlasWidth = static_cast<float>(textureSpec.Width);
		metadata->AtlasHeight = static_cast<float>(textureSpec.Height);
		metadata->LineHeight = lineHeight;
		metadata->Characters = characters;
		metadata->InitialFileLocation = FileSystem::GetRelativePath(Projects::Project::GetAssetDirectory(), filePath);
		newAsset.Data.SpecificFileData = metadata;

		buffer.Release();
	}


}
namespace Kargono::Utility
{
	static void SerializeEntity(YAML::Emitter& out, Entity entity)
	{
		KG_ASSERT(entity.HasComponent<IDComponent>(), "Entity does not have a component");

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
			out << YAML::Key << "CurrentShape" << YAML::Value << Utility::ShapeTypeToString(shapeComponent.CurrentShape);
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
			KG_ASSERT(sizeof(uint8_t) * 20 == sizeof(ShaderSpecification), "Please Update Deserialization and Serialization. Incorrect size of input data in Scene Serializer!");
			if (shapeComponent.Shader)
			{
				// Add Shader Handle
				out << YAML::Key << "ShaderHandle" << YAML::Value << static_cast<uint64_t>(shapeComponent.ShaderHandle);
				// Add Shader Specification
				const ShaderSpecification& shaderSpec = shapeComponent.Shader->GetSpecification();
				out << YAML::Key << "ShaderSpecification" << YAML::Value;
				out << YAML::BeginMap;
				out << YAML::Key << "ColorInputType" << YAML::Value << Utility::ColorInputTypeToString(shaderSpec.ColorInput);
				out << YAML::Key << "AddProjectionMatrix" << YAML::Value << shaderSpec.AddProjectionMatrix;
				out << YAML::Key << "AddEntityID" << YAML::Value << shaderSpec.AddEntityID;
				out << YAML::Key << "AddCircleShape" << YAML::Value << shaderSpec.AddCircleShape;
				out << YAML::Key << "TextureInput" << YAML::Value << Utility::TextureInputTypeToString(shaderSpec.TextureInput);
				out << YAML::Key << "DrawOutline" << YAML::Value << shaderSpec.DrawOutline;
				out << YAML::Key << "RenderType" << YAML::Value << Utility::RenderingTypeToString(shaderSpec.RenderType);

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
			out << YAML::Key << "BodyType" << YAML::Value << Utility::RigidBody2DBodyTypeToString(rb2dComponent.Type);
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
			Ref<Script::ScriptClass> entityClass = Script::ScriptEngine::GetEntityClass(scriptComponent.ClassName);
			const auto& fields = entityClass->GetFields();
			if (fields.size() > 0)
			{
				out << YAML::Key << "ScriptFields" << YAML::Value;
				auto& entityFields = Script::ScriptEngine::GetScriptFieldMap(entity);
				out << YAML::BeginSeq;
				for (const auto& [name, field] : fields)
				{

					if (!entityFields.contains(name)) { continue; }

					out << YAML::BeginMap; // Script Fields
					out << YAML::Key << "Name" << YAML::Value << name;
					out << YAML::Key << "Type" << YAML::Value << Utility::ScriptFieldTypeToString(field.Type);
					out << YAML::Key << "Data" << YAML::Value;

					Script::ScriptFieldInstance& scriptField = entityFields.at(name);

					switch (field.Type)
					{
						WRITE_SCRIPT_FIELD(Script::ScriptFieldType::Float, float);
						WRITE_SCRIPT_FIELD(Script::ScriptFieldType::Double, double);
						WRITE_SCRIPT_FIELD(Script::ScriptFieldType::Bool, bool);
						WRITE_SCRIPT_FIELD(Script::ScriptFieldType::Char, char);
						WRITE_SCRIPT_FIELD(Script::ScriptFieldType::Byte, int8_t);
						WRITE_SCRIPT_FIELD(Script::ScriptFieldType::Short, int16_t);
						WRITE_SCRIPT_FIELD(Script::ScriptFieldType::Int, int32_t);
						WRITE_SCRIPT_FIELD(Script::ScriptFieldType::Long, int64_t);
						WRITE_SCRIPT_FIELD(Script::ScriptFieldType::UByte, uint8_t);
						WRITE_SCRIPT_FIELD(Script::ScriptFieldType::UShort, uint16_t);
						WRITE_SCRIPT_FIELD(Script::ScriptFieldType::UInt, uint32_t);
						WRITE_SCRIPT_FIELD(Script::ScriptFieldType::ULong, uint64_t);
						WRITE_SCRIPT_FIELD(Script::ScriptFieldType::Vector2, Math::vec2);
						WRITE_SCRIPT_FIELD(Script::ScriptFieldType::Vector3, Math::vec3);
						WRITE_SCRIPT_FIELD(Script::ScriptFieldType::Vector4, Math::vec4);
						WRITE_SCRIPT_FIELD(Script::ScriptFieldType::Entity, uint64_t);

					}

					out << YAML::EndMap; // Script Fields
				}
				out << YAML::EndSeq;
			}


			out << YAML::EndMap; // Component Map
		}


		out << YAML::EndMap; // Entity
	}
}

namespace Kargono::Assets
{
	std::unordered_map<AssetHandle, Assets::Asset> AssetManager::s_SceneRegistry {};

	void AssetManager::DeserializeSceneRegistry()
	{
		// Clear current registry and open registry in current project 
		s_SceneRegistry.clear();
		KG_ASSERT(Projects::Project::GetActive(), "There is no currently loaded project to serialize from!");
		const auto& sceneRegistryLocation = Projects::Project::GetAssetDirectory() / "Scenes/SceneRegistry.kgreg";

		if (!std::filesystem::exists(sceneRegistryLocation))
		{
			KG_ERROR("No .kgregistry file exists in project path!");
			return;
		}
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(sceneRegistryLocation.string());
		}
		catch (YAML::ParserException e)
		{
			KG_ERROR("Failed to load .kgscene file '{0}'\n     {1}", sceneRegistryLocation.string(), e.what());
			return;
		}

		// Opening registry node 
		if (!data["Registry"]) { return; }

		std::string registryName = data["Registry"].as<std::string>();
		KG_INFO("Deserializing Scene Registry");

		// Opening all assets 
		auto assets = data["Assets"];
		if (assets)
		{
			for (auto asset : assets)
			{
				Assets::Asset newAsset{};
				newAsset.Handle = asset["AssetHandle"].as<uint64_t>();

				// Retrieving metadata for asset 
				auto metadata = asset["MetaData"];
				newAsset.Data.CheckSum = metadata["CheckSum"].as<std::string>();
				newAsset.Data.IntermediateLocation = metadata["IntermediateLocation"].as<std::string>();
				newAsset.Data.Type = Utility::StringToAssetType(metadata["AssetType"].as<std::string>());

				// Retrieving shader specific metadata 
				if (newAsset.Data.Type == Assets::Scene)
				{
					Ref<Assets::SceneMetaData> sceneMetaData = CreateRef<Assets::SceneMetaData>();
					newAsset.Data.SpecificFileData = sceneMetaData;
				}

				// Add asset to in memory registry 
				s_SceneRegistry.insert({ newAsset.Handle, newAsset });
			}
		}
	}

	void AssetManager::SerializeSceneRegistry()
	{
		KG_ASSERT(Projects::Project::GetActive(), "There is no currently loaded project to serialize to!");
		const auto& sceneRegistryLocation = Projects::Project::GetAssetDirectory() / "Scenes/SceneRegistry.kgreg";
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
			out << YAML::Key << "AssetType" << YAML::Value << Utility::AssetTypeToString(asset.Data.Type);

			out << YAML::EndMap; // MetaData Map
			out << YAML::EndMap; // Asset Map
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;

		FileSystem::CreateNewDirectory(sceneRegistryLocation.parent_path());

		std::ofstream fout(sceneRegistryLocation);
		fout << out.c_str();
	}

	void AssetManager::SerializeScene(Ref<Kargono::Scene> scene, const std::filesystem::path& filepath)
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

				Utility::SerializeEntity(out, entity);
			});
		out << YAML::EndSeq;
		out << YAML::EndMap; // Start of File Map

		std::ofstream fout(filepath);
		fout << out.c_str();
		KG_INFO("Successfully Serialized Scene at {}", filepath);
	}

	bool AssetManager::CheckSceneExists(const std::string& sceneName)
	{
		// Create Checksum
		const std::string currentCheckSum = FileSystem::ChecksumFromString(sceneName);

		if (currentCheckSum.empty())
		{
			KG_ERROR("Failed to generate checksum from file!");
			return {};
		}

		for (const auto& [handle, asset] : s_SceneRegistry)
		{
			if (asset.Data.CheckSum == currentCheckSum)
			{
				KG_INFO("Attempt to instantiate duplicate font asset");
				return true;
			}
		}

		return false;
	}

	bool AssetManager::DeserializeScene(Ref<Kargono::Scene> scene, const std::filesystem::path& filepath)
	{
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(filepath.string());
		}
		catch (YAML::ParserException e)
		{
			KG_ERROR("Failed to load .kgscene file '{0}'\n     {1}", filepath, e.what());
			return false;
		}

		KG_INFO("Deserializing scene");

		auto physics = data["Physics"];
		scene->GetPhysicsSpecification().Gravity = physics["Gravity"].as<Math::vec2>();

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
					tc.Translation = transformComponent["Translation"].as<Math::vec3>();
					tc.Rotation = transformComponent["Rotation"].as<Math::vec3>();
					tc.Scale = transformComponent["Scale"].as<Math::vec3>();
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

						Ref<Script::ScriptClass> entityClass = Script::ScriptEngine::GetEntityClass(sc.ClassName);
						if (entityClass)
						{
							KG_ASSERT(entityClass);
							const auto& fields = entityClass->GetFields();

							auto& entityFields = Script::ScriptEngine::GetScriptFieldMap(deserializedEntity);

							for (auto scriptField : scriptFields)
							{
								std::string name = scriptField["Name"].as<std::string>();
								std::string typeString = scriptField["Type"].as<std::string>();
								Script::ScriptFieldType type = Utility::ScriptFieldTypeFromString(typeString);

								Script::ScriptFieldInstance& fieldInstance = entityFields[name];
								// TODO(): Turn into Log Message
								KG_ASSERT(fields.contains(name))
									if (!fields.contains(name)) { continue; }
								fieldInstance.Field = fields.at(name);

								switch (type)
								{
									READ_SCRIPT_FIELD(Script::ScriptFieldType::Float, float);
									READ_SCRIPT_FIELD(Script::ScriptFieldType::Double, double);
									READ_SCRIPT_FIELD(Script::ScriptFieldType::Bool, bool);
									READ_SCRIPT_FIELD(Script::ScriptFieldType::Char, char);
									READ_SCRIPT_FIELD(Script::ScriptFieldType::Byte, int8_t);
									READ_SCRIPT_FIELD(Script::ScriptFieldType::Short, int16_t);
									READ_SCRIPT_FIELD(Script::ScriptFieldType::Int, int32_t);
									READ_SCRIPT_FIELD(Script::ScriptFieldType::Long, int64_t);
									READ_SCRIPT_FIELD(Script::ScriptFieldType::UByte, uint8_t);
									READ_SCRIPT_FIELD(Script::ScriptFieldType::UShort, uint16_t);
									READ_SCRIPT_FIELD(Script::ScriptFieldType::UInt, uint32_t);
									READ_SCRIPT_FIELD(Script::ScriptFieldType::ULong, uint64_t);
									READ_SCRIPT_FIELD(Script::ScriptFieldType::Vector2, Math::vec2);
									READ_SCRIPT_FIELD(Script::ScriptFieldType::Vector3, Math::vec3);
									READ_SCRIPT_FIELD(Script::ScriptFieldType::Vector4, Math::vec4);
									READ_SCRIPT_FIELD(Script::ScriptFieldType::Entity, UUID);
								}
							}
						}
					}
				}

				auto shapeComponent = entity["ShapeComponent"];
				if (shapeComponent)
				{
					auto& sc = deserializedEntity.AddComponent<ShapeComponent>();
					sc.CurrentShape = Utility::StringToShapeType(shapeComponent["CurrentShape"].as<std::string>());

					if (shapeComponent["VertexColors"])
					{
						auto vertexColors = shapeComponent["VertexColors"];
						sc.VertexColors = CreateRef<std::vector<Math::vec4>>();
						for (auto color : vertexColors)
						{
							sc.VertexColors->push_back(color["Color"].as<Math::vec4>());
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
							ShaderSpecification shaderSpec{};
							// ShaderSpecification Section
							shaderSpec.ColorInput = Utility::StringToColorInputType(shaderSpecificationNode["ColorInputType"].as<std::string>());
							shaderSpec.AddProjectionMatrix = shaderSpecificationNode["AddProjectionMatrix"].as<bool>();
							shaderSpec.AddEntityID = shaderSpecificationNode["AddEntityID"].as<bool>();
							shaderSpec.AddCircleShape = shaderSpecificationNode["AddCircleShape"].as<bool>();
							shaderSpec.TextureInput = Utility::StringToTextureInputType(shaderSpecificationNode["TextureInput"].as<std::string>());
							shaderSpec.DrawOutline = shaderSpecificationNode["DrawOutline"].as<bool>();
							shaderSpec.RenderType = Utility::StringToRenderingType(shaderSpecificationNode["RenderType"].as<std::string>());
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
						if (sc.CurrentShape != ShapeTypes::None)
						{
							if (sc.ShaderSpecification.RenderType == RenderingType::DrawIndex)
							{
								sc.Vertices = CreateRef<std::vector<Math::vec3>>(Utility::ShapeTypeToShape(sc.CurrentShape).GetIndexVertices());
								sc.Indices = CreateRef<std::vector<uint32_t>>(Utility::ShapeTypeToShape(sc.CurrentShape).GetIndices());
								sc.TextureCoordinates = CreateRef<std::vector<Math::vec2>>(Utility::ShapeTypeToShape(sc.CurrentShape).GetIndexTextureCoordinates());
							}

							if (sc.ShaderSpecification.RenderType == RenderingType::DrawTriangle)
							{
								sc.Vertices = CreateRef<std::vector<Math::vec3>>(Utility::ShapeTypeToShape(sc.CurrentShape).GetTriangleVertices());
								sc.TextureCoordinates = CreateRef<std::vector<Math::vec2>>(Utility::ShapeTypeToShape(sc.CurrentShape).GetTriangleTextureCoordinates());
							}
						}
					}
				}

				auto rigidbody2DComponent = entity["Rigidbody2DComponent"];
				if (rigidbody2DComponent)
				{
					auto& rb2d = deserializedEntity.AddComponent<Rigidbody2DComponent>();
					rb2d.Type = Utility::StringToRigidBody2DBodyType(rigidbody2DComponent["BodyType"].as<std::string>());
					rb2d.FixedRotation = rigidbody2DComponent["FixedRotation"].as<bool>();
				}

				auto boxCollider2DComponent = entity["BoxCollider2DComponent"];
				if (boxCollider2DComponent)
				{
					auto& bc2d = deserializedEntity.AddComponent<BoxCollider2DComponent>();
					bc2d.Offset = boxCollider2DComponent["Offset"].as<Math::vec2>();
					bc2d.Size = boxCollider2DComponent["Size"].as<Math::vec2>();
					bc2d.Density = boxCollider2DComponent["Density"].as<float>();
					bc2d.Friction = boxCollider2DComponent["Friction"].as<float>();
					bc2d.Restitution = boxCollider2DComponent["Restitution"].as<float>();
					bc2d.RestitutionThreshold = boxCollider2DComponent["RestitutionThreshold"].as<float>();
				}

				auto circleCollider2DComponent = entity["CircleCollider2DComponent"];
				if (circleCollider2DComponent)
				{
					auto& cc2d = deserializedEntity.AddComponent<CircleCollider2DComponent>();
					cc2d.Offset = circleCollider2DComponent["Offset"].as<Math::vec2>();
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
			KG_ERROR("Failed to generate checksum from file!");
			return {};
		}

		// Compare currentChecksum to registered assets
		for (const auto& [handle, asset] : s_SceneRegistry)
		{
			if (asset.Data.CheckSum == currentCheckSum)
			{
				KG_INFO("Attempt to instantiate duplicate font asset");
				return handle;
			}
		}

		// Create New Asset/Handle
		AssetHandle newHandle{};
		Assets::Asset newAsset{};
		newAsset.Handle = newHandle;

		// Create File
		CreateSceneFile(sceneName, newAsset);
		newAsset.Data.CheckSum = currentCheckSum;

		// Register New Asset and Create Scene
		s_SceneRegistry.insert({ newHandle, newAsset }); // Update Registry Map in-memory
		SerializeSceneRegistry(); // Update Registry File on Disk

		return newHandle;
	}

	void AssetManager::SaveScene(AssetHandle sceneHandle, Ref<Kargono::Scene> scene)
	{
		if (!s_SceneRegistry.contains(sceneHandle))
		{
			KG_ERROR("Attempt to save scene that does not exist in registry");
			return;
		}
		Assets::Asset sceneAsset = s_SceneRegistry[sceneHandle];
		SerializeScene(scene, (Projects::Project::GetAssetDirectory() / sceneAsset.Data.IntermediateLocation).string());
	}

	Ref<Kargono::Scene> AssetManager::GetScene(const AssetHandle& handle)
	{
		KG_ASSERT(Projects::Project::GetActive(), "There is no active project when retreiving scene!");

		if (s_SceneRegistry.contains(handle))
		{
			auto asset = s_SceneRegistry[handle];
			return InstantiateScene(asset);
		}

		KG_ERROR("No scene is associated with provided handle!");
		return nullptr;
	}
	std::tuple<AssetHandle, Ref<Kargono::Scene>> AssetManager::GetScene(const std::filesystem::path& filepath)
	{
		KG_ASSERT(Projects::Project::GetActive(), "Attempt to use Project Field without active project!");

		std::filesystem::path scenePath {};

		if (FileSystem::DoesPathContainSubPath(Projects::Project::GetAssetDirectory(), filepath))
		{
			scenePath = FileSystem::GetRelativePath(Projects::Project::GetAssetDirectory(), filepath);
		}
		else
		{
			scenePath = filepath;
		}

		for (auto& [assetHandle, asset] : s_SceneRegistry)
		{
			if (asset.Data.IntermediateLocation.compare(scenePath) == 0)
			{
				return std::make_tuple(assetHandle, InstantiateScene(asset));
			}
		}
		// Return empty scene if scene does not exist
		KG_WARN("No Scene Associated with provided handle. Returned new empty scene");
		AssetHandle newHandle = CreateNewScene(filepath.stem().string());
		return std::make_tuple(newHandle, GetScene(newHandle));
	}

	Ref<Kargono::Scene> AssetManager::InstantiateScene(const Assets::Asset& sceneAsset)
	{
		Ref<Kargono::Scene> newScene = CreateRef<Kargono::Scene>();
		DeserializeScene(newScene, (Projects::Project::GetAssetDirectory() / sceneAsset.Data.IntermediateLocation).string());
		return newScene;
	}


	void AssetManager::ClearSceneRegistry()
	{
		s_SceneRegistry.clear();
	}

	void AssetManager::CreateSceneFile(const std::string& sceneName, Assets::Asset& newAsset)
	{
		// Create Temporary Scene
		Ref<Kargono::Scene> temporaryScene = CreateRef<Kargono::Scene>();

		// Save Binary Intermediate into File
		std::string scenePath = "Scenes/" + sceneName + ".kgscene";
		std::filesystem::path intermediateFullPath = Projects::Project::GetAssetDirectory() / scenePath;
		SerializeScene(temporaryScene, intermediateFullPath.string());

		// Load data into In-Memory Metadata object
		newAsset.Data.Type = Assets::AssetType::Scene;
		newAsset.Data.IntermediateLocation = scenePath;
		Ref<Assets::SceneMetaData> metadata = CreateRef<Assets::SceneMetaData>();
		newAsset.Data.SpecificFileData = metadata;
	}

	std::unordered_map<AssetHandle, Assets::Asset> AssetManager::s_UIObjectRegistry {};

	void AssetManager::DeserializeUIObjectRegistry()
	{
		// Clear current registry and open registry in current project 
		s_UIObjectRegistry.clear();
		KG_ASSERT(Projects::Project::GetActive(), "There is no currently loaded project to serialize from!");
		const auto& uiObjectRegistryLocation = Projects::Project::GetAssetDirectory() / "UserInterface/UIObjectRegistry.kgreg";

		if (!std::filesystem::exists(uiObjectRegistryLocation))
		{
			KG_ERROR("No .kgregistry file exists in project path!");
			return;
		}
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(uiObjectRegistryLocation.string());
		}
		catch (YAML::ParserException e)
		{
			KG_ERROR("Failed to load .kguiObject file '{0}'\n     {1}", uiObjectRegistryLocation.string(), e.what());
			return;
		}

		// Opening registry node 
		if (!data["Registry"]) { return; }

		std::string registryName = data["Registry"].as<std::string>();
		KG_INFO("Deserializing UIObject Registry");

		// Opening all assets 
		auto assets = data["Assets"];
		if (assets)
		{
			for (auto asset : assets)
			{
				Assets::Asset newAsset{};
				newAsset.Handle = asset["AssetHandle"].as<uint64_t>();

				// Retrieving metadata for asset 
				auto metadata = asset["MetaData"];
				newAsset.Data.CheckSum = metadata["CheckSum"].as<std::string>();
				newAsset.Data.IntermediateLocation = metadata["IntermediateLocation"].as<std::string>();
				newAsset.Data.Type = Utility::StringToAssetType(metadata["AssetType"].as<std::string>());

				// Retrieving uiobject specific metadata 
				if (newAsset.Data.Type == Assets::UIObject)
				{
					Ref<Assets::UIObjectMetaData> uiObjectMetaData = CreateRef<Assets::UIObjectMetaData>();
					newAsset.Data.SpecificFileData = uiObjectMetaData;
				}

				// Add asset to in memory registry 
				s_UIObjectRegistry.insert({ newAsset.Handle, newAsset });
			}
		}
	}

	void AssetManager::SerializeUIObjectRegistry()
	{
		KG_ASSERT(Projects::Project::GetActive(), "There is no currently loaded project to serialize to!");
		const auto& uiObjectRegistryLocation = Projects::Project::GetAssetDirectory() / "UserInterface/UIObjectRegistry.kgreg";
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
			out << YAML::Key << "AssetType" << YAML::Value << Utility::AssetTypeToString(asset.Data.Type);

			out << YAML::EndMap; // MetaData Map
			out << YAML::EndMap; // Asset Map
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;

		FileSystem::CreateNewDirectory(uiObjectRegistryLocation.parent_path());

		std::ofstream fout(uiObjectRegistryLocation);
		fout << out.c_str();
	}

	void AssetManager::SerializeUIObject(Ref<RuntimeUI::UIObject> uiObject, const std::filesystem::path& filepath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap; // Start of File Map
		// Select Color
		out << YAML::Key << "SelectColor" << YAML::Value << uiObject->m_SelectColor;

		// Function Pointers
		out << YAML::Key << "FunctionPointerOnMove" << YAML::Value << uiObject->m_FunctionPointers.OnMove;
		// Font
		out << YAML::Key << "Font" << YAML::Value << static_cast<uint64_t>(uiObject->m_FontHandle);
		// Windows
		out << YAML::Key << "Windows" << YAML::Value;
		out << YAML::BeginSeq; // Start of Windows Seq

		for (auto& window : uiObject->Windows)
		{
			out << YAML::BeginMap; // Start Window Map

			out << YAML::Key << "Tag" << YAML::Value << window.Tag;
			out << YAML::Key << "ScreenPosition" << YAML::Value << window.ScreenPosition;
			out << YAML::Key << "Size" << YAML::Value << window.Size;
			out << YAML::Key << "BackgroundColor" << YAML::Value << window.BackgroundColor;
			out << YAML::Key << "ParentIndex" << YAML::Value << window.ParentIndex;
			out << YAML::Key << "ChildBufferIndex" << YAML::Value << window.ChildBufferIndex;
			out << YAML::Key << "ChildBufferSize" << YAML::Value << window.ChildBufferSize;
			out << YAML::Key << "DefaultActiveWidget" << YAML::Value << window.DefaultActiveWidget;

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

				out << YAML::Key << "Tag" << YAML::Value << widget->Tag;
				out << YAML::Key << "WindowPosition" << YAML::Value << widget->WindowPosition;
				out << YAML::Key << "Size" << YAML::Value << widget->Size;
				out << YAML::Key << "DefaultBackgroundColor" << YAML::Value << widget->DefaultBackgroundColor;
				out << YAML::Key << "WidgetType" << YAML::Value << Utility::WidgetTypeToString(widget->WidgetType);
				out << YAML::Key << "Selectable" << YAML::Value << widget->Selectable;
				out << YAML::Key << "DirectionPointerUp" << YAML::Value << widget->DirectionPointer.Up;
				out << YAML::Key << "DirectionPointerDown" << YAML::Value << widget->DirectionPointer.Down;
				out << YAML::Key << "DirectionPointerLeft" << YAML::Value << widget->DirectionPointer.Left;
				out << YAML::Key << "DirectionPointerRight" << YAML::Value << widget->DirectionPointer.Right;

				out << YAML::Key << "FunctionPointerOnPress" << YAML::Value << widget->FunctionPointers.OnPress;
				switch (widget->WidgetType)
				{
				case RuntimeUI::WidgetTypes::TextWidget:
					{
						RuntimeUI::TextWidget* textWidget = static_cast<RuntimeUI::TextWidget*>(widget.get());
						out << YAML::Key << "TextWidget" << YAML::Value;
						out << YAML::BeginMap; // Begin TextWidget Map
						out << YAML::Key << "Text" << YAML::Value << textWidget->Text;
						out << YAML::Key << "TextSize" << YAML::Value << textWidget->TextSize;
						out << YAML::Key << "TextColor" << YAML::Value << textWidget->TextColor;
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
			std::vector<Ref<RuntimeUI::Widget>> Widgets {};
		}

		out << YAML::EndSeq; // End of Windows Seq
		out << YAML::EndMap; // Start of File Map

		std::ofstream fout(filepath);
		fout << out.c_str();
		KG_INFO("Successfully Serialized UIObject at {}", filepath);
	}

	bool AssetManager::CheckUIObjectExists(const std::string& uiObjectName)
	{
		// Create Checksum
		const std::string currentCheckSum = FileSystem::ChecksumFromString(uiObjectName);

		if (currentCheckSum.empty())
		{
			KG_ERROR("Failed to generate checksum from file!");
			return {};
		}

		for (const auto& [handle, asset] : s_UIObjectRegistry)
		{
			if (asset.Data.CheckSum == currentCheckSum)
			{
				KG_INFO("Attempt to instantiate duplicate font asset");
				return true;
			}
		}

		return false;
	}

	bool AssetManager::DeserializeUIObject(Ref<RuntimeUI::UIObject> uiObject, const std::filesystem::path& filepath)
	{
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(filepath.string());
		}
		catch (YAML::ParserException e)
		{
			KG_ERROR("Failed to load .kgui file '{0}'\n     {1}", filepath, e.what());
			return false;
		}

		KG_INFO("Deserializing user interface object");

		// Get SelectColor
		uiObject->m_SelectColor = data["SelectColor"].as<Math::vec4>();
		// Function Pointers
		uiObject->m_FunctionPointers.OnMove = data["FunctionPointerOnMove"].as<std::string>();
		// Get Font
		uiObject->m_FontHandle = data["Font"].as<uint64_t>();
		uiObject->m_Font = AssetManager::GetFont(uiObject->m_FontHandle);
		// Get Windows
		auto windows = data["Windows"];
		if (windows)
		{
			auto& newWindowsList = uiObject->Windows;
			for (auto window : windows)
			{
				RuntimeUI::Window newWindow{};
				newWindow.Tag = window["Tag"].as<std::string>();
				newWindow.ScreenPosition = window["ScreenPosition"].as<Math::vec3>();
				newWindow.Size = window["Size"].as<Math::vec2>();
				newWindow.BackgroundColor = window["BackgroundColor"].as<Math::vec4>();
				newWindow.ParentIndex = window["ParentIndex"].as<int32_t>();
				newWindow.ChildBufferIndex = window["ChildBufferIndex"].as<int32_t>();
				newWindow.ChildBufferSize = window["ChildBufferSize"].as<uint32_t>();
				newWindow.DefaultActiveWidget = window["DefaultActiveWidget"].as<int32_t>();

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
						RuntimeUI::WidgetTypes widgetType = Utility::StringToWidgetType(widget["WidgetType"].as<std::string>());
						Ref<RuntimeUI::Widget> newWidget = nullptr;
						YAML::Node specificWidget;
						switch (widgetType)
						{
						case RuntimeUI::WidgetTypes::TextWidget:
						{
							specificWidget = widget["TextWidget"];
							newWidget = CreateRef<RuntimeUI::TextWidget>();
							newWidget->WidgetType = widgetType;
							RuntimeUI::TextWidget* textWidget = static_cast<RuntimeUI::TextWidget*>(newWidget.get());
							textWidget->Text = specificWidget["Text"].as<std::string>();
							textWidget->TextSize = specificWidget["TextSize"].as<float>();
							textWidget->TextColor = specificWidget["TextColor"].as<glm::vec4>();
							textWidget->TextAbsoluteDimensions = specificWidget["TextAbsoluteDimensions"].as<Math::vec2>();
							textWidget->TextCentered = specificWidget["TextCentered"].as<bool>();
							break;
						}
						default:
							{
							KG_ASSERT("Invalid Widget Type in RuntimeUI Deserialization");
							return false;
							}
						}

						newWidget->Tag = widget["Tag"].as<std::string>();
						newWidget->WindowPosition = widget["WindowPosition"].as<Math::vec2>();
						newWidget->Size = widget["Size"].as<Math::vec2>();
						newWidget->DefaultBackgroundColor = widget["DefaultBackgroundColor"].as<Math::vec4>();
						newWidget->ActiveBackgroundColor = newWidget->DefaultBackgroundColor;
						newWidget->Selectable = widget["Selectable"].as<bool>();
						newWidget->DirectionPointer.Up = widget["DirectionPointerUp"].as<int32_t>();
						newWidget->DirectionPointer.Down = widget["DirectionPointerDown"].as<int32_t>();
						newWidget->DirectionPointer.Left = widget["DirectionPointerLeft"].as<int32_t>();
						newWidget->DirectionPointer.Right = widget["DirectionPointerRight"].as<int32_t>();

						newWidget->FunctionPointers.OnPress = widget["FunctionPointerOnPress"].as<std::string>();

						newWidgetsList.push_back(newWidget);


					}
					if (newWindow.DefaultActiveWidget != -1) { newWindow.DefaultActiveWidgetRef = newWidgetsList.at(newWindow.DefaultActiveWidget); }
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
			KG_ERROR("Failed to generate checksum from file!");
			return {};
		}

		// Compare currentChecksum to registered assets
		for (const auto& [handle, asset] : s_UIObjectRegistry)
		{
			if (asset.Data.CheckSum == currentCheckSum)
			{
				KG_INFO("Attempt to instantiate duplicate font asset");
				return handle;
			}
		}

		// Create New Asset/Handle
		AssetHandle newHandle{};
		Assets::Asset newAsset{};
		newAsset.Handle = newHandle;

		// Create File
		CreateUIObjectFile(uiObjectName, newAsset);
		newAsset.Data.CheckSum = currentCheckSum;

		// Register New Asset and return handle.
		s_UIObjectRegistry.insert({ newHandle, newAsset }); // Update Registry Map in-memory
		SerializeUIObjectRegistry(); // Update Registry File on Disk

		return newHandle;
	}

	void AssetManager::SaveUIObject(AssetHandle uiObjectHandle, Ref<RuntimeUI::UIObject> uiObject)
	{
		if (!s_UIObjectRegistry.contains(uiObjectHandle))
		{
			KG_ERROR("Attempt to save uiObject that does not exist in registry");
			return;
		}
		Assets::Asset uiObjectAsset = s_UIObjectRegistry[uiObjectHandle];
		SerializeUIObject(uiObject, (Projects::Project::GetAssetDirectory() / uiObjectAsset.Data.IntermediateLocation).string());
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

	Ref<RuntimeUI::UIObject> AssetManager::GetUIObject(const AssetHandle& handle)
	{
		KG_ASSERT(Projects::Project::GetActive(), "There is no active project when retreiving uiObject!");

		if (s_UIObjectRegistry.contains(handle))
		{
			auto asset = s_UIObjectRegistry[handle];
			return InstantiateUIObject(asset);
		}

		KG_ERROR("No uiObject is associated with provided handle!");
		return nullptr;
	}
	std::tuple<AssetHandle, Ref<RuntimeUI::UIObject>> AssetManager::GetUIObject(const std::filesystem::path& filepath)
	{
		KG_ASSERT(Projects::Project::GetActive(), "Attempt to use Project Field without active project!");
		std::filesystem::path uiObjectPath = filepath;

		if (filepath.is_absolute())
		{
			uiObjectPath = FileSystem::GetRelativePath(Projects::Project::GetAssetDirectory(), filepath);
		}

		for (auto& [assetHandle, asset] : s_UIObjectRegistry)
		{
			if (asset.Data.IntermediateLocation.compare(uiObjectPath) == 0)
			{
				return std::make_tuple(assetHandle, InstantiateUIObject(asset));
			}
		}
		// Return empty uiObject if uiObject does not exist
		KG_WARN("No UIObject Associated with provided handle. Returned new empty uiObject");
		AssetHandle newHandle = CreateNewUIObject(filepath.stem().string());
		return std::make_tuple(newHandle, GetUIObject(newHandle));
	}

	Ref<RuntimeUI::UIObject> AssetManager::InstantiateUIObject(const Assets::Asset& uiObjectAsset)
	{
		Ref<RuntimeUI::UIObject> newUIObject = CreateRef<RuntimeUI::UIObject>();
		DeserializeUIObject(newUIObject, (Projects::Project::GetAssetDirectory() / uiObjectAsset.Data.IntermediateLocation).string());
		return newUIObject;
	}


	void AssetManager::ClearUIObjectRegistry()
	{
		s_UIObjectRegistry.clear();
	}

	void AssetManager::CreateUIObjectFile(const std::string& uiObjectName, Assets::Asset& newAsset)
	{
		// Create Temporary UIObject
		Ref<RuntimeUI::UIObject> temporaryUIObject = CreateRef<RuntimeUI::UIObject>();

		// Save Binary Intermediate into File
		std::string uiObjectPath = "UserInterface/" + uiObjectName + ".kgui";
		std::filesystem::path intermediateFullPath = Projects::Project::GetAssetDirectory() / uiObjectPath;
		SerializeUIObject(temporaryUIObject, intermediateFullPath.string());

		// Load data into In-Memory Metadata object
		newAsset.Data.Type = Assets::AssetType::UIObject;
		newAsset.Data.IntermediateLocation = uiObjectPath;
		Ref<Assets::UIObjectMetaData> metadata = CreateRef<Assets::UIObjectMetaData>();
		newAsset.Data.SpecificFileData = metadata;
	}


	std::unordered_map<AssetHandle, Assets::Asset> AssetManager::s_InputModeRegistry {};

	void AssetManager::DeserializeInputModeRegistry()
	{
		// Clear current registry and open registry in current project 
		s_InputModeRegistry.clear();
		KG_ASSERT(Projects::Project::GetActive(), "There is no currently loaded project to serialize from!");
		const auto& inputModeRegistryLocation = Projects::Project::GetAssetDirectory() / "Input/InputRegistry.kgreg";

		if (!std::filesystem::exists(inputModeRegistryLocation))
		{
			KG_ERROR("No .kgregistry file exists in project path!");
			return;
		}
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(inputModeRegistryLocation.string());
		}
		catch (YAML::ParserException e)
		{
			KG_ERROR("Failed to load .kginputMode file '{0}'\n     {1}", inputModeRegistryLocation.string(), e.what());
			return;
		}

		// Opening registry node 
		if (!data["Registry"]) { return; }

		std::string registryName = data["Registry"].as<std::string>();
		KG_INFO("Deserializing InputMode Registry");

		// Opening all assets 
		auto assets = data["Assets"];
		if (assets)
		{
			for (auto asset : assets)
			{
				Assets::Asset newAsset{};
				newAsset.Handle = asset["AssetHandle"].as<uint64_t>();

				// Retrieving metadata for asset 
				auto metadata = asset["MetaData"];
				newAsset.Data.CheckSum = metadata["CheckSum"].as<std::string>();
				newAsset.Data.IntermediateLocation = metadata["IntermediateLocation"].as<std::string>();
				newAsset.Data.Type = Utility::StringToAssetType(metadata["AssetType"].as<std::string>());

				// Retrieving inputMode specific metadata 
				if (newAsset.Data.Type == Assets::InputMode)
				{
					Ref<Assets::InputModeMetaData> inputModeMetaData = CreateRef<Assets::InputModeMetaData>();
					newAsset.Data.SpecificFileData = inputModeMetaData;
				}

				// Add asset to in memory registry 
				s_InputModeRegistry.insert({ newAsset.Handle, newAsset });
			}
		}
	}

	void AssetManager::SerializeInputModeRegistry()
	{
		KG_ASSERT(Projects::Project::GetActive(), "There is no currently loaded project to serialize to!");
		const auto& inputModeRegistryLocation = Projects::Project::GetAssetDirectory() / "Input/InputRegistry.kgreg";
		YAML::Emitter out;

		out << YAML::BeginMap;
		out << YAML::Key << "Registry" << YAML::Value << "UserInterface";
		out << YAML::Key << "Assets" << YAML::Value << YAML::BeginSeq;

		// Asset
		for (auto& [handle, asset] : s_InputModeRegistry)
		{
			out << YAML::BeginMap; // Asset Map
			out << YAML::Key << "AssetHandle" << YAML::Value << static_cast<uint64_t>(handle);
			out << YAML::Key << "MetaData" << YAML::Value;
			out << YAML::BeginMap; // MetaData Map
			out << YAML::Key << "CheckSum" << YAML::Value << asset.Data.CheckSum;
			out << YAML::Key << "IntermediateLocation" << YAML::Value << asset.Data.IntermediateLocation.string();
			out << YAML::Key << "AssetType" << YAML::Value << Utility::AssetTypeToString(asset.Data.Type);

			out << YAML::EndMap; // MetaData Map
			out << YAML::EndMap; // Asset Map
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;

		FileSystem::CreateNewDirectory(inputModeRegistryLocation.parent_path());

		std::ofstream fout(inputModeRegistryLocation);
		fout << out.c_str();
	}

	void AssetManager::SerializeInputMode(Ref<Kargono::InputMode> inputMode, const std::filesystem::path& filepath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap; // Start of File Map

		{
			// Keyboard Polling
			out << YAML::Key << "KeyboardPolling" << YAML::Value;
			out << YAML::BeginSeq; // Start of KeyboardPolling Seq

			for (auto& [slot, keyCode] : inputMode->m_KeyboardPolling)
			{
				out << YAML::BeginMap; // Start Polling Combo

				out << YAML::Key << "Slot" << YAML::Value << slot;
				out << YAML::Key << "KeyCode" << YAML::Value << keyCode;

				out << YAML::EndMap; // End Polling Combo
			}
			out << YAML::EndSeq; // End of KeyboardPolling Seq
		}

		{
			// CustomCalls OnUpdate
			out << YAML::Key << "CustomCallsOnUpdate" << YAML::Value;
			out << YAML::BeginSeq; // Start of CustomCalls OnUpdate Seq

			for (auto& inputBinding : inputMode->m_CustomCallsOnUpdateBindings)
			{
				out << YAML::BeginMap; // InputActionBinding Start

				out << YAML::Key << "BindingType" << YAML::Value << Utility::InputActionTypeToString(inputBinding->GetActionType());
				out << YAML::Key << "FunctionBinding" << YAML::Value << inputBinding->GetFunctionBinding();

				switch (inputBinding->GetActionType())
				{
				case InputMode::KeyboardAction:
					{
					InputMode::KeyboardActionBinding* keyboardBinding = (InputMode::KeyboardActionBinding*)inputBinding.get();
					out << YAML::Key << "KeyBinding" << YAML::Value << keyboardBinding->GetKeyBinding();
					break;
					}
				case InputMode::None:
				default:
					{
					KG_ASSERT(false, "Invalid InputMode provided to InputMode serialization");
					break;
					}
				}

				out << YAML::EndMap; // InputActionBinding End
			}
			out << YAML::EndSeq; // End of CustomCalls OnUpdate Seq
		}

		{
			// ScriptClass OnUpdate
			out << YAML::Key << "ScriptClassOnUpdate" << YAML::Value;
			out << YAML::BeginSeq; // Start of CustomCalls OnUpdate Seq

			for (auto& [className, bindingList] : inputMode->m_ScriptClassOnUpdateBindings)
			{
				out << YAML::BeginMap; // Binding List Start

				out << YAML::Key << "ClassName" << YAML::Value << className;

				out << YAML::Key << "AllBindings" << YAML::Value;
				out << YAML::BeginSeq; // Start of All Bindings Seq

				for (auto& inputBinding : bindingList)
				{
					out << YAML::BeginMap; // InputActionBinding Start
					out << YAML::Key << "BindingType" << YAML::Value << Utility::InputActionTypeToString(inputBinding->GetActionType());
					out << YAML::Key << "FunctionBinding" << YAML::Value << inputBinding->GetFunctionBinding();

					switch (inputBinding->GetActionType())
					{
					case InputMode::KeyboardAction:
					{
						InputMode::KeyboardActionBinding* keyboardBinding = (InputMode::KeyboardActionBinding*)inputBinding.get();
						out << YAML::Key << "KeyBinding" << YAML::Value << keyboardBinding->GetKeyBinding();
						break;
					}
					case InputMode::None:
					default:
					{
						KG_ASSERT("Invalid InputMode provided to InputMode serialization");
						break;
					}
					}
					out << YAML::EndMap; // InputActionBinding End
				}

				out << YAML::EndSeq; // End of All Bindings Seq


				out << YAML::EndMap; // Binding List End
			}
			out << YAML::EndSeq; // End of CustomCalls OnUpdate Seq
		}

		{
			// CustomCalls OnKeyPressed
			out << YAML::Key << "CustomCallsOnKeyPressed" << YAML::Value;
			out << YAML::BeginSeq; // Start of CustomCalls OnKeyPressed Seq

			for (auto& inputBinding : inputMode->m_CustomCallsOnKeyPressedBindings)
			{
				out << YAML::BeginMap; // InputActionBinding Start

				out << YAML::Key << "BindingType" << YAML::Value << Utility::InputActionTypeToString(inputBinding->GetActionType());
				out << YAML::Key << "FunctionBinding" << YAML::Value << inputBinding->GetFunctionBinding();

				switch (inputBinding->GetActionType())
				{
				case InputMode::KeyboardAction:
				{
					InputMode::KeyboardActionBinding* keyboardBinding = (InputMode::KeyboardActionBinding*)inputBinding.get();
					out << YAML::Key << "KeyBinding" << YAML::Value << keyboardBinding->GetKeyBinding();
					break;
				}
				case InputMode::None:
				default:
				{
					KG_ASSERT("Invalid InputMode provided to InputMode serialization");
					break;
				}
				}

				out << YAML::EndMap; // InputActionBinding End
			}
			out << YAML::EndSeq; // End of CustomCalls OnKeyPressed Seq
		}

		{
			// ScriptClass OnKeyPressed
			out << YAML::Key << "ScriptClassOnKeyPressed" << YAML::Value;
			out << YAML::BeginSeq; // Start of CustomCalls OnKeyPressed Seq

			for (auto& [className, bindingList] : inputMode->m_ScriptClassOnKeyPressedBindings)
			{
				out << YAML::BeginMap; // Binding List Start

				out << YAML::Key << "ClassName" << YAML::Value << className;

				out << YAML::Key << "AllBindings" << YAML::Value;
				out << YAML::BeginSeq; // Start of All Bindings Seq

				for (auto& inputBinding : bindingList)
				{
					out << YAML::BeginMap; // InputActionBinding Start
					out << YAML::Key << "BindingType" << YAML::Value << Utility::InputActionTypeToString(inputBinding->GetActionType());
					out << YAML::Key << "FunctionBinding" << YAML::Value << inputBinding->GetFunctionBinding();

					switch (inputBinding->GetActionType())
					{
					case InputMode::KeyboardAction:
					{
						InputMode::KeyboardActionBinding* keyboardBinding = (InputMode::KeyboardActionBinding*)inputBinding.get();
						out << YAML::Key << "KeyBinding" << YAML::Value << keyboardBinding->GetKeyBinding();
						break;
					}
					case InputMode::None:
					default:
					{
						KG_ASSERT("Invalid InputMode provided to InputMode serialization");
						break;
					}
					}
					out << YAML::EndMap; // InputActionBinding End
				}

				out << YAML::EndSeq; // End of All Bindings Seq


				out << YAML::EndMap; // Binding List End
			}
			out << YAML::EndSeq; // End of CustomCalls OnKeyPressed Seq
		}


		out << YAML::EndMap; // Start of File Map

		std::ofstream fout(filepath);
		fout << out.c_str();
		KG_INFO("Successfully Serialized InputMode at {}", filepath);
	}

	bool AssetManager::CheckInputModeExists(const std::string& inputModeName)
	{
		// Create Checksum
		const std::string currentCheckSum = FileSystem::ChecksumFromString(inputModeName);

		if (currentCheckSum.empty())
		{
			KG_ERROR("Failed to generate checksum from file!");
			return {};
		}

		for (const auto& [handle, asset] : s_InputModeRegistry)
		{
			if (asset.Data.CheckSum == currentCheckSum)
			{
				KG_INFO("Attempt to instantiate duplicate font asset");
				return true;
			}
		}

		return false;
	}

	bool AssetManager::DeserializeInputMode(Ref<Kargono::InputMode> inputMode, const std::filesystem::path& filepath)
	{
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(filepath.string());
		}
		catch (YAML::ParserException e)
		{
			KG_ERROR("Failed to load .kgui file '{0}'\n     {1}", filepath, e.what());
			return false;
		}

		KG_INFO("Deserializing input mode");

		// Get Keyboard Polling!
		{
			
			auto keyboardPolling = data["KeyboardPolling"];
			if (keyboardPolling)
			{
				auto& keyboardPollingNew = inputMode->m_KeyboardPolling;
				for (auto binding : keyboardPolling)
				{
					uint16_t slot = (uint16_t)binding["Slot"].as<uint32_t>();
					uint16_t keyCode = (uint16_t)binding["KeyCode"].as<uint32_t>();
					keyboardPollingNew.insert({ slot, keyCode });
				}
			}
		}

		// CustomCalls OnUpdate
		{
			auto customOnUpdate = data["CustomCallsOnUpdate"];
			if (customOnUpdate)
			{
				auto& customOnUpdateNew = inputMode->m_CustomCallsOnUpdateBindings;
				for (auto binding : customOnUpdate)
				{
					InputMode::InputActionTypes bindingType = Utility::StringToInputActionType(binding["BindingType"].as<std::string>());
					Ref<InputMode::InputActionBinding> newActionBinding = nullptr;
					switch (bindingType)
					{
						case InputMode::KeyboardAction:
						{
							newActionBinding = CreateRef<InputMode::KeyboardActionBinding>();
							((InputMode::KeyboardActionBinding*)newActionBinding.get())->SetKeyBinding(binding["KeyBinding"].as<uint32_t>());
							break;
						}
						case InputMode::None:
						default:
						{
							KG_ASSERT(false, "Invalid bindingType while deserializing InputMode");
							break;
						}
					}

					newActionBinding->SetFunctionBinding(binding["FunctionBinding"].as<std::string>());
					customOnUpdateNew.push_back(newActionBinding);
				}
			}
		}

		// ScriptClass OnUpdate
		{
			auto classOnUpdate = data["ScriptClassOnUpdate"];
			if (classOnUpdate)
			{
				auto& classOnUpdateNew = inputMode->m_ScriptClassOnUpdateBindings;
				for (auto classDescription : classOnUpdate)
				{
					std::string className = classDescription["ClassName"].as<std::string>();

					auto allBindings = classDescription["AllBindings"];

					classOnUpdateNew.insert({ className, {} });
					if (allBindings)
					{
						for (auto binding : allBindings)
						{
							InputMode::InputActionTypes bindingType = Utility::StringToInputActionType(binding["BindingType"].as<std::string>());
							Ref<InputMode::InputActionBinding> newActionBinding = nullptr;
							switch (bindingType)
							{
							case InputMode::KeyboardAction:
							{
								newActionBinding = CreateRef<InputMode::KeyboardActionBinding>();
								((InputMode::KeyboardActionBinding*)newActionBinding.get())->SetKeyBinding(binding["KeyBinding"].as<uint32_t>());
								break;
							}
							case InputMode::None:
							default:
							{
								KG_ASSERT(false, "Invalid bindingType while deserializing InputMode");
								break;
							}
							}

							newActionBinding->SetFunctionBinding(binding["FunctionBinding"].as<std::string>());

							classOnUpdateNew.at(className).push_back(newActionBinding);
						}
					}
				}
			}
		}

		// CustomCalls OnKeyPressed
		{
			auto customOnKeyPressed = data["CustomCallsOnKeyPressed"];
			if (customOnKeyPressed)
			{
				auto& customOnKeyPressedNew = inputMode->m_CustomCallsOnKeyPressedBindings;
				for (auto binding : customOnKeyPressed)
				{
					InputMode::InputActionTypes bindingType = Utility::StringToInputActionType(binding["BindingType"].as<std::string>());
					Ref<InputMode::InputActionBinding> newActionBinding = nullptr;
					switch (bindingType)
					{
					case InputMode::KeyboardAction:
					{
						newActionBinding = CreateRef<InputMode::KeyboardActionBinding>();
						((InputMode::KeyboardActionBinding*)newActionBinding.get())->SetKeyBinding(binding["KeyBinding"].as<uint32_t>());
						break;
					}
					case InputMode::None:
					default:
					{
						KG_ASSERT(false, "Invalid bindingType while deserializing InputMode");
						break;
					}
					}

					newActionBinding->SetFunctionBinding(binding["FunctionBinding"].as<std::string>());
					customOnKeyPressedNew.push_back(newActionBinding);
				}
			}
		}

		// ScriptClass OnKeyPressed
		{
			auto classOnKeyPressed = data["ScriptClassOnKeyPressed"];
			if (classOnKeyPressed)
			{
				auto& classOnKeyPressedNew = inputMode->m_ScriptClassOnKeyPressedBindings;
				for (auto classDescription : classOnKeyPressed)
				{
					std::string className = classDescription["ClassName"].as<std::string>();

					auto allBindings = classDescription["AllBindings"];

					classOnKeyPressedNew.insert({ className, {} });
					if (allBindings)
					{
						for (auto binding : allBindings)
						{
							InputMode::InputActionTypes bindingType = Utility::StringToInputActionType(binding["BindingType"].as<std::string>());
							Ref<InputMode::InputActionBinding> newActionBinding = nullptr;
							switch (bindingType)
							{
							case InputMode::KeyboardAction:
							{
								newActionBinding = CreateRef<InputMode::KeyboardActionBinding>();
								((InputMode::KeyboardActionBinding*)newActionBinding.get())->SetKeyBinding(binding["KeyBinding"].as<uint32_t>());
								break;
							}
							case InputMode::None:
							default:
							{
								KG_ASSERT(false, "Invalid bindingType while deserializing InputMode");
								break;
							}
							}

							newActionBinding->SetFunctionBinding(binding["FunctionBinding"].as<std::string>());

							classOnKeyPressedNew.at(className).push_back(newActionBinding);
						}
					}
				}
			}
		}

		return true;

	}

	AssetHandle AssetManager::CreateNewInputMode(const std::string& inputModeName)
	{
		// Create Checksum
		const std::string currentCheckSum = FileSystem::ChecksumFromString(inputModeName);

		if (currentCheckSum.empty())
		{
			KG_ERROR("Failed to generate checksum from file!");
			return {};
		}

		// Compare currentChecksum to registered assets
		for (const auto& [handle, asset] : s_InputModeRegistry)
		{
			if (asset.Data.CheckSum == currentCheckSum)
			{
				KG_INFO("Attempt to instantiate duplicate input asset");
				return handle;
			}
		}

		// Create New Asset/Handle
		AssetHandle newHandle{};
		Assets::Asset newAsset{};
		newAsset.Handle = newHandle;

		// Create File
		CreateInputModeFile(inputModeName, newAsset);
		newAsset.Data.CheckSum = currentCheckSum;

		// Register New Asset and return handle.
		s_InputModeRegistry.insert({ newHandle, newAsset }); // Update Registry Map in-memory
		SerializeInputModeRegistry(); // Update Registry File on Disk

		return newHandle;
	}

	void AssetManager::SaveInputMode(AssetHandle inputModeHandle, Ref<Kargono::InputMode> inputMode)
	{
		if (!s_InputModeRegistry.contains(inputModeHandle))
		{
			KG_ERROR("Attempt to save inputMode that does not exist in registry");
			return;
		}
		Assets::Asset inputModeAsset = s_InputModeRegistry[inputModeHandle];
		SerializeInputMode(inputMode, (Projects::Project::GetAssetDirectory() / inputModeAsset.Data.IntermediateLocation).string());
	}

	std::filesystem::path AssetManager::GetInputModeLocation(const AssetHandle& handle)
	{
		if (!s_InputModeRegistry.contains(handle))
		{
			KG_ERROR("Attempt to save inputMode that does not exist in registry");
			return "";
		}
		return s_InputModeRegistry[handle].Data.IntermediateLocation;
	}

	Ref<Kargono::InputMode> AssetManager::GetInputMode(const AssetHandle& handle)
	{
		KG_ASSERT(Projects::Project::GetActive(), "There is no active project when retreiving inputMode!");

		if (s_InputModeRegistry.contains(handle))
		{
			auto asset = s_InputModeRegistry[handle];
			return InstantiateInputMode(asset);
		}

		KG_ERROR("No inputMode is associated with provided handle!");
		return nullptr;
	}
	std::tuple<AssetHandle, Ref<Kargono::InputMode>> AssetManager::GetInputMode(const std::filesystem::path& filepath)
	{
		KG_ASSERT(Projects::Project::GetActive(), "Attempt to use Project Field without active project!");

		std::filesystem::path inputModePath = filepath;

		if (filepath.is_absolute())
		{
			inputModePath = FileSystem::GetRelativePath(Projects::Project::GetAssetDirectory(), filepath);
		}

		for (auto& [assetHandle, asset] : s_InputModeRegistry)
		{
			if (asset.Data.IntermediateLocation.compare(inputModePath) == 0)
			{
				return std::make_tuple(assetHandle, InstantiateInputMode(asset));
			}
		}
		// Return empty inputMode if inputMode does not exist
		KG_WARN("No InputMode Associated with provided handle. Returned new empty inputMode");
		AssetHandle newHandle = CreateNewInputMode(filepath.stem().string());
		return std::make_tuple(newHandle, GetInputMode(newHandle));
	}

	Ref<Kargono::InputMode> AssetManager::InstantiateInputMode(const Assets::Asset& inputModeAsset)
	{
		Ref<Kargono::InputMode> newInputMode = CreateRef<Kargono::InputMode>();
		DeserializeInputMode(newInputMode, (Projects::Project::GetAssetDirectory() / inputModeAsset.Data.IntermediateLocation).string());
		return newInputMode;
	}


	void AssetManager::ClearInputModeRegistry()
	{
		s_InputModeRegistry.clear();
	}

	void AssetManager::CreateInputModeFile(const std::string& inputModeName, Assets::Asset& newAsset)
	{
		// Create Temporary InputMode
		Ref<Kargono::InputMode> temporaryInputMode = CreateRef<Kargono::InputMode>();

		// Save Binary Intermediate into File
		std::string inputModePath = "Input/" + inputModeName + ".kginput";
		std::filesystem::path intermediateFullPath = Projects::Project::GetAssetDirectory() / inputModePath;
		SerializeInputMode(temporaryInputMode, intermediateFullPath.string());

		// Load data into In-Memory Metadata object
		newAsset.Data.Type = Assets::AssetType::InputMode;
		newAsset.Data.IntermediateLocation = inputModePath;
		Ref<Assets::InputModeMetaData> metadata = CreateRef<Assets::InputModeMetaData>();
		newAsset.Data.SpecificFileData = metadata;
	}

	}

namespace Kargono::Utility
{
	void SerializeWrappedVariableData(Ref<WrappedVariable> variable, YAML::Emitter& out)
	{
		switch (variable->Type())
		{
			case WrappedVarType::Integer32:
			{
				out << YAML::Key << "Value" << YAML::Value << variable->GetWrappedValue<int32_t>();
				return;
			}
			case WrappedVarType::UInteger16: 
			{
				out << YAML::Key << "Value" << YAML::Value << variable->GetWrappedValue<uint16_t>();
				return;
			}
			case WrappedVarType::UInteger32: 
			{
				out << YAML::Key << "Value" << YAML::Value << variable->GetWrappedValue<uint32_t>();
				return;
			}
			case WrappedVarType::UInteger64:
			{
				out << YAML::Key << "Value" << YAML::Value << variable->GetWrappedValue<int64_t>();
				return;
			}
			case WrappedVarType::String:
			{
				out << YAML::Key << "Value" << YAML::Value << variable->GetWrappedValue<std::string>();
				return;
			}
			case WrappedVarType::Bool:
			{
				out << YAML::Key << "Value" << YAML::Value << variable->GetWrappedValue<bool>();
				return;
			}
			case WrappedVarType::Void:
			case WrappedVarType::None:
			{
				KG_CRITICAL("Use of Void or None when serializing value");
				return;
			}
		}
		KG_ASSERT(false, "Unknown Type of WrappedVariableType when serializing");
	}

	Ref<WrappedVariable> DeserializeWrappedVariableData(WrappedVarType type, YAML::detail::iterator_value& field)
	{
		switch (type)
		{
			case WrappedVarType::Integer32:
			{
				return CreateRef<WrappedInteger32>(field["Value"].as<int32_t>());
			}
			case WrappedVarType::UInteger16:
			{
				return CreateRef<WrappedUInteger16>(field["Value"].as<uint16_t>());
			}
			case WrappedVarType::UInteger32:
			{
				return CreateRef<WrappedUInteger32>(field["Value"].as<uint32_t>());
			}
			case WrappedVarType::UInteger64:
			{
				return CreateRef<WrappedUInteger64>(field["Value"].as<uint64_t>());
			}
			case WrappedVarType::String:
			{
				return CreateRef<WrappedString>(field["Value"].as<std::string>());
			}
			case WrappedVarType::Bool:
			{
				return CreateRef<WrappedBool>(field["Value"].as<bool>());
			}
			case WrappedVarType::Void:
			case WrappedVarType::None:
			{
				KG_CRITICAL("Use of Void or None when serializing value");
				return nullptr;
			}
		}
		KG_ASSERT(false, "Unknown Type of WrappedVariableType when deserializing");
		return nullptr;
	}
}

namespace Kargono::Assets
{
	std::unordered_map<AssetHandle, Assets::Asset> AssetManager::s_GameStateRegistry {};

	void AssetManager::DeserializeGameStateRegistry()
	{
		// Clear current registry and open registry in current project 
		s_GameStateRegistry.clear();
		KG_ASSERT(Projects::Project::GetActive(), "There is no currently loaded project to serialize from!");
		const auto& GameStateRegistryLocation = Projects::Project::GetAssetDirectory() / "GameState/GameStateRegistry.kgreg";

		if (!std::filesystem::exists(GameStateRegistryLocation))
		{
			KG_ERROR("No .kgregistry file exists in project path!");
			return;
		}
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(GameStateRegistryLocation.string());
		}
		catch (YAML::ParserException e)
		{
			KG_ERROR("Failed to load .kgstate file '{0}'\n     {1}", GameStateRegistryLocation.string(), e.what());
			return;
		}

		// Opening registry node 
		if (!data["Registry"]) { return; }

		std::string registryName = data["Registry"].as<std::string>();
		KG_INFO("Deserializing GameState Registry");

		// Opening all assets 
		auto assets = data["Assets"];
		if (assets)
		{
			for (auto asset : assets)
			{
				Assets::Asset newAsset{};
				newAsset.Handle = asset["AssetHandle"].as<uint64_t>();

				// Retrieving metadata for asset 
				auto metadata = asset["MetaData"];
				newAsset.Data.CheckSum = metadata["CheckSum"].as<std::string>();
				newAsset.Data.IntermediateLocation = metadata["IntermediateLocation"].as<std::string>();
				newAsset.Data.Type = Utility::StringToAssetType(metadata["AssetType"].as<std::string>());

				// Retrieving GameState specific metadata 
				if (newAsset.Data.Type == Assets::GameState)
				{
					Ref<Assets::GameStateMetaData> GameStateMetaData = CreateRef<Assets::GameStateMetaData>();
					GameStateMetaData->Name = metadata["Name"].as<std::string>();
					newAsset.Data.SpecificFileData = GameStateMetaData;
				}

				// Add asset to in memory registry 
				s_GameStateRegistry.insert({ newAsset.Handle, newAsset });
			}
		}
	}

	void AssetManager::SerializeGameStateRegistry()
	{
		KG_ASSERT(Projects::Project::GetActive(), "There is no currently loaded project to serialize to!");
		const auto& GameStateRegistryLocation = Projects::Project::GetAssetDirectory() / "GameState/GameStateRegistry.kgreg";
		YAML::Emitter out;

		out << YAML::BeginMap;
		out << YAML::Key << "Registry" << YAML::Value << "GameState";
		out << YAML::Key << "Assets" << YAML::Value << YAML::BeginSeq;

		// Asset
		for (auto& [handle, asset] : s_GameStateRegistry)
		{
			out << YAML::BeginMap; // Asset Map
			out << YAML::Key << "AssetHandle" << YAML::Value << static_cast<uint64_t>(handle);
			out << YAML::Key << "MetaData" << YAML::Value;
			out << YAML::BeginMap; // MetaData Map
			out << YAML::Key << "CheckSum" << YAML::Value << asset.Data.CheckSum;
			out << YAML::Key << "IntermediateLocation" << YAML::Value << asset.Data.IntermediateLocation.string();
			out << YAML::Key << "AssetType" << YAML::Value << Utility::AssetTypeToString(asset.Data.Type);
			if (asset.Data.Type == Assets::AssetType::GameState)
			{
				Assets::GameStateMetaData* metadata = static_cast<Assets::GameStateMetaData*>(asset.Data.SpecificFileData.get());

				out << YAML::Key << "Name" << YAML::Value << metadata->Name;
			}

			out << YAML::EndMap; // MetaData Map
			out << YAML::EndMap; // Asset Map
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;

		FileSystem::CreateNewDirectory(GameStateRegistryLocation.parent_path());

		std::ofstream fout(GameStateRegistryLocation);
		fout << out.c_str();
	}

	void AssetManager::SerializeGameState(Ref<Kargono::GameState> GameState, const std::filesystem::path& filepath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap; // Start of File Map

		out << YAML::Key << "Name" << YAML::Value << GameState->m_Name; // Output State Name

		out << YAML::Key << "Fields" << YAML::Value;
		out << YAML::BeginSeq; // Start Fields

		for (auto& [name, field] : GameState->m_Fields)
		{
			out << YAML::BeginMap; // Start Field

			out << YAML::Key << "Name" << YAML::Value << name; // Name/Map Key
			out << YAML::Key << "Type" << YAML::Value << Utility::WrappedVarTypeToString(field->Type()); // Field Type
			Utility::SerializeWrappedVariableData(field, out); // Field Value

			out << YAML::EndMap; // End Field
		}

		out << YAML::EndSeq; // End Fields

		out << YAML::EndMap; // End of File Map

		std::ofstream fout(filepath);
		fout << out.c_str();
		KG_INFO("Successfully Serialized GameState at {}", filepath);
	}

	bool AssetManager::CheckGameStateExists(const std::string& GameStateName)
	{
		// Create Checksum
		const std::string currentCheckSum = FileSystem::ChecksumFromString(GameStateName);

		if (currentCheckSum.empty())
		{
			KG_ERROR("Failed to generate checksum from file!");
			return {};
		}

		for (const auto& [handle, asset] : s_GameStateRegistry)
		{
			if (asset.Data.CheckSum == currentCheckSum)
			{
				KG_INFO("Attempt to instantiate duplicate font asset");
				return true;
			}
		}

		return false;
	}

	bool AssetManager::DeserializeGameState(Ref<Kargono::GameState> GameState, const std::filesystem::path& filepath)
	{
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(filepath.string());
		}
		catch (YAML::ParserException e)
		{
			KG_ERROR("Failed to load .kgui file '{0}'\n     {1}", filepath, e.what());
			return false;
		}

		KG_INFO("Deserializing game state");

		GameState->m_Name = data["Name"].as<std::string>();

		// Get Fields
		{
			auto fields = data["Fields"];
			if (fields)
			{
				auto& newFieldsMap = GameState->m_Fields;
				for (auto field : fields)
				{
					std::string fieldName = field["Name"].as<std::string>();
					WrappedVarType fieldType = Utility::StringToWrappedVarType(field["Type"].as<std::string>());
					Ref<WrappedVariable> wrappedVariable = Utility::DeserializeWrappedVariableData(fieldType, field);
					newFieldsMap.insert_or_assign(fieldName, wrappedVariable);
				}
			}
		}
		return true;

	}

	AssetHandle AssetManager::CreateNewGameState(const std::string& GameStateName)
	{
		// Create Checksum
		const std::string currentCheckSum = FileSystem::ChecksumFromString(GameStateName);

		if (currentCheckSum.empty())
		{
			KG_ERROR("Failed to generate checksum from file!");
			return {};
		}

		// Compare currentChecksum to registered assets
		for (const auto& [handle, asset] : s_GameStateRegistry)
		{
			if (asset.Data.CheckSum == currentCheckSum)
			{
				KG_INFO("Attempt to instantiate duplicate game state asset");
				return handle;
			}
		}

		// Create New Asset/Handle
		AssetHandle newHandle{};
		Assets::Asset newAsset{};
		newAsset.Handle = newHandle;

		// Create File
		CreateGameStateFile(GameStateName, newAsset);
		newAsset.Data.CheckSum = currentCheckSum;

		// Register New Asset and return handle.
		s_GameStateRegistry.insert({ newHandle, newAsset }); // Update Registry Map in-memory
		SerializeGameStateRegistry(); // Update Registry File on Disk

		return newHandle;
	}

	void AssetManager::SaveGameState(AssetHandle GameStateHandle, Ref<Kargono::GameState> GameState)
	{
		if (!s_GameStateRegistry.contains(GameStateHandle))
		{
			KG_ERROR("Attempt to save GameState that does not exist in registry");
			return;
		}
		Assets::Asset GameStateAsset = s_GameStateRegistry[GameStateHandle];
		SerializeGameState(GameState, (Projects::Project::GetAssetDirectory() / GameStateAsset.Data.IntermediateLocation).string());
	}

	void AssetManager::DeleteGameState(AssetHandle handle)
	{
		if (!s_GameStateRegistry.contains(handle))
		{
			KG_WARN("Failed to delete GameState in AssetManager");
			return;
		}

		FileSystem::DeleteSelectedFile(Projects::Project::GetAssetDirectory() /
			s_GameStateRegistry.at(handle).Data.IntermediateLocation);

		s_GameStateRegistry.erase(handle);

		SerializeGameStateRegistry();
	}

	std::filesystem::path AssetManager::GetGameStateLocation(const AssetHandle& handle)
	{
		if (!s_GameStateRegistry.contains(handle))
		{
			KG_ERROR("Attempt to save GameState that does not exist in registry");
			return "";
		}
		return s_GameStateRegistry[handle].Data.IntermediateLocation;
	}

	Ref<Kargono::GameState> AssetManager::GetGameState(const AssetHandle& handle)
	{
		KG_ASSERT(Projects::Project::GetActive(), "There is no active project when retreiving GameState!");

		if (s_GameStateRegistry.contains(handle))
		{
			auto asset = s_GameStateRegistry[handle];
			return InstantiateGameState(asset);
		}

		KG_ERROR("No GameState is associated with provided handle!");
		return nullptr;
	}
	std::tuple<AssetHandle, Ref<Kargono::GameState>> AssetManager::GetGameState(const std::filesystem::path& filepath)
	{
		KG_ASSERT(Projects::Project::GetActive(), "Attempt to use Project Field without active project!");

		std::filesystem::path GameStatePath = filepath;

		if (filepath.is_absolute())
		{
			GameStatePath = FileSystem::GetRelativePath(Projects::Project::GetAssetDirectory(), filepath);
		}

		for (auto& [assetHandle, asset] : s_GameStateRegistry)
		{
			if (asset.Data.IntermediateLocation.compare(GameStatePath) == 0)
			{
				return std::make_tuple(assetHandle, InstantiateGameState(asset));
			}
		}
		// Return empty GameState if GameState does not exist
		KG_WARN("No GameState Associated with provided handle. Returned new empty GameState");
		AssetHandle newHandle = CreateNewGameState(filepath.stem().string());
		return std::make_tuple(newHandle, GetGameState(newHandle));
	}

	Ref<Kargono::GameState> AssetManager::InstantiateGameState(const Assets::Asset& GameStateAsset)
	{
		Ref<Kargono::GameState> newGameState = CreateRef<Kargono::GameState>();
		DeserializeGameState(newGameState, (Projects::Project::GetAssetDirectory() / GameStateAsset.Data.IntermediateLocation).string());
		return newGameState;
	}


	void AssetManager::ClearGameStateRegistry()
	{
		s_GameStateRegistry.clear();
	}

	void AssetManager::CreateGameStateFile(const std::string& GameStateName, Assets::Asset& newAsset)
	{
		// Create Temporary GameState
		Ref<Kargono::GameState> temporaryGameState = CreateRef<Kargono::GameState>();
		temporaryGameState->SetName(GameStateName);

		// Save Binary Intermediate into File
		std::string GameStatePath = "GameState/" + GameStateName + ".kgstate";
		std::filesystem::path intermediateFullPath = Projects::Project::GetAssetDirectory() / GameStatePath;
		SerializeGameState(temporaryGameState, intermediateFullPath.string());

		// Load data into In-Memory Metadata object
		newAsset.Data.Type = Assets::AssetType::GameState;
		newAsset.Data.IntermediateLocation = GameStatePath;
		Ref<Assets::GameStateMetaData> metadata = CreateRef<Assets::GameStateMetaData>();
		metadata->Name = GameStateName;
		newAsset.Data.SpecificFileData = metadata;
	}

	std::unordered_map<AssetHandle, Assets::Asset> AssetManager::s_EntityClassRegistry {};

	void AssetManager::DeserializeEntityClassRegistry()
	{
		// Clear current registry and open registry in current project 
		s_EntityClassRegistry.clear();
		KG_ASSERT(Projects::Project::GetActive(), "There is no currently loaded project to serialize from!");
		const auto& EntityClassRegistryLocation = Projects::Project::GetAssetDirectory() / "EntityClass/EntityClassRegistry.kgreg";

		if (!std::filesystem::exists(EntityClassRegistryLocation))
		{
			KG_ERROR("No .kgregistry file exists in project path!");
			return;
		}
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(EntityClassRegistryLocation.string());
		}
		catch (YAML::ParserException e)
		{
			KG_ERROR("Failed to load .kgstate file '{0}'\n     {1}", EntityClassRegistryLocation.string(), e.what());
			return;
		}

		// Opening registry node 
		if (!data["Registry"]) { return; }

		std::string registryName = data["Registry"].as<std::string>();
		KG_INFO("Deserializing EntityClass Registry");

		// Opening all assets 
		auto assets = data["Assets"];
		if (assets)
		{
			for (auto asset : assets)
			{
				Assets::Asset newAsset{};
				newAsset.Handle = asset["AssetHandle"].as<uint64_t>();

				// Retrieving metadata for asset 
				auto metadata = asset["MetaData"];
				newAsset.Data.CheckSum = metadata["CheckSum"].as<std::string>();
				newAsset.Data.IntermediateLocation = metadata["IntermediateLocation"].as<std::string>();
				newAsset.Data.Type = Utility::StringToAssetType(metadata["AssetType"].as<std::string>());

				// Retrieving EntityClass specific metadata 
				if (newAsset.Data.Type == Assets::EntityClass)
				{
					Ref<Assets::EntityClassMetaData> EntityClassMetaData = CreateRef<Assets::EntityClassMetaData>();
					EntityClassMetaData->Name = metadata["Name"].as<std::string>();

					newAsset.Data.SpecificFileData = EntityClassMetaData;
				}

				// Add asset to in memory registry 
				s_EntityClassRegistry.insert({ newAsset.Handle, newAsset });
			}
		}
	}

	void AssetManager::SerializeEntityClassRegistry()
	{
		KG_ASSERT(Projects::Project::GetActive(), "There is no currently loaded project to serialize to!");
		const auto& EntityClassRegistryLocation = Projects::Project::GetAssetDirectory() / "EntityClass/EntityClassRegistry.kgreg";
		YAML::Emitter out;

		out << YAML::BeginMap;
		out << YAML::Key << "Registry" << YAML::Value << "EntityClass";
		out << YAML::Key << "Assets" << YAML::Value << YAML::BeginSeq;

		// Asset
		for (auto& [handle, asset] : s_EntityClassRegistry)
		{
			out << YAML::BeginMap; // Asset Map
			out << YAML::Key << "AssetHandle" << YAML::Value << static_cast<uint64_t>(handle);
			out << YAML::Key << "MetaData" << YAML::Value;
			out << YAML::BeginMap; // MetaData Map
			out << YAML::Key << "CheckSum" << YAML::Value << asset.Data.CheckSum;
			out << YAML::Key << "IntermediateLocation" << YAML::Value << asset.Data.IntermediateLocation.string();
			out << YAML::Key << "AssetType" << YAML::Value << Utility::AssetTypeToString(asset.Data.Type);
			if (asset.Data.Type == Assets::AssetType::EntityClass)
			{
				Assets::EntityClassMetaData* metadata = static_cast<Assets::EntityClassMetaData*>(asset.Data.SpecificFileData.get());

				out << YAML::Key << "Name" << YAML::Value << metadata->Name;
			}

			out << YAML::EndMap; // MetaData Map
			out << YAML::EndMap; // Asset Map
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;

		FileSystem::CreateNewDirectory(EntityClassRegistryLocation.parent_path());

		std::ofstream fout(EntityClassRegistryLocation);
		fout << out.c_str();
	}

	void AssetManager::SerializeEntityClass(Ref<Kargono::EntityClass> EntityClass, const std::filesystem::path& filepath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap; // Start of File Map

		out << YAML::Key << "Name" << YAML::Value << EntityClass->GetName(); // Output State Name
		out << YAML::Key << "FieldTypes" << YAML::Value;
		out << YAML::BeginSeq; // Start Fields

		for (auto& [name, field] : EntityClass->m_FieldTypes)
		{
			out << YAML::BeginMap; // Start Field

			out << YAML::Key << "Name" << YAML::Value << name; // Name/Map Key
			out << YAML::Key << "Type" << YAML::Value << Utility::WrappedVarTypeToString(field); // Field Type

			out << YAML::EndMap; // End Field
		}

		out << YAML::EndSeq; // End Fields

		out << YAML::Key << "OnPhysicsCollisionStart" << YAML::Value << 
			static_cast<uint64_t>(EntityClass->m_Scripts.OnPhysicsCollisionStartHandle);
		out << YAML::Key << "OnPhysicsCollisionEnd" << YAML::Value <<
			static_cast<uint64_t>(EntityClass->m_Scripts.OnPhysicsCollisionEndHandle);
		out << YAML::Key << "OnCreate" << YAML::Value <<
			static_cast<uint64_t>(EntityClass->m_Scripts.OnCreateHandle);
		out << YAML::Key << "OnUpdate" << YAML::Value <<
			static_cast<uint64_t>(EntityClass->m_Scripts.OnUpdateHandle);

		out << YAML::Key << "AllScripts" << YAML::Value;
		out << YAML::BeginSeq; // Start AllScripts

		for (auto& script : EntityClass->m_Scripts.AllClassScripts)
		{
			out << YAML::Value << static_cast<uint64_t>(script); // Script ID
		}

		out << YAML::EndSeq; // End AllScripts

		out << YAML::EndMap; // End of File Map

		std::ofstream fout(filepath);
		fout << out.c_str();
		KG_INFO("Successfully Serialized EntityClass at {}", filepath);
	}

	bool AssetManager::CheckEntityClassExists(const std::string& EntityClassName)
	{
		// Create Checksum
		const std::string currentCheckSum = FileSystem::ChecksumFromString(EntityClassName);

		if (currentCheckSum.empty())
		{
			KG_ERROR("Failed to generate checksum from file!");
			return {};
		}

		for (const auto& [handle, asset] : s_EntityClassRegistry)
		{
			if (asset.Data.CheckSum == currentCheckSum)
			{
				KG_INFO("Attempt to instantiate duplicate font asset");
				return true;
			}
		}

		return false;
	}

	bool AssetManager::DeserializeEntityClass(Ref<Kargono::EntityClass> EntityClass, const std::filesystem::path& filepath)
	{
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(filepath.string());
		}
		catch (YAML::ParserException e)
		{
			KG_ERROR("Failed to load .kgui file '{0}'\n     {1}", filepath, e.what());
			return false;
		}

		KG_INFO("Deserializing game state");

		EntityClass->m_Name = data["Name"].as<std::string>();

		// Get Fields
		{
			auto fields = data["FieldTypes"];
			if (fields)
			{
				auto& newFieldsMap = EntityClass->m_FieldTypes;
				for (auto field : fields)
				{
					std::string fieldName = field["Name"].as<std::string>();
					WrappedVarType fieldType = Utility::StringToWrappedVarType(field["Type"].as<std::string>());
					newFieldsMap.insert_or_assign(fieldName, fieldType);
				}
			}
		}

		// Get Function Slots
		{
			EntityScripts& scripts = EntityClass->m_Scripts;
			scripts.OnPhysicsCollisionStartHandle = 
				static_cast<Assets::AssetHandle>(data["OnPhysicsCollisionStart"].as<uint64_t>());
			if (scripts.OnPhysicsCollisionStartHandle != Assets::EmptyHandle)
			{
				scripts.OnPhysicsCollisionStart = 
					Assets::AssetManager::GetScript(scripts.OnPhysicsCollisionStartHandle).get();
			}
			scripts.OnPhysicsCollisionEndHandle =
				static_cast<Assets::AssetHandle>(data["OnPhysicsCollisionEnd"].as<uint64_t>());
			if (scripts.OnPhysicsCollisionEndHandle != Assets::EmptyHandle)
			{
				scripts.OnPhysicsCollisionEnd =
					Assets::AssetManager::GetScript(scripts.OnPhysicsCollisionEndHandle).get();
			}
			scripts.OnCreateHandle =
				static_cast<Assets::AssetHandle>(data["OnCreate"].as<uint64_t>());
			if (scripts.OnCreateHandle != Assets::EmptyHandle)
			{
				scripts.OnCreate =
					Assets::AssetManager::GetScript(scripts.OnCreateHandle).get();
			}
			scripts.OnUpdateHandle =
				static_cast<Assets::AssetHandle>(data["OnUpdate"].as<uint64_t>());
			if (scripts.OnUpdateHandle != Assets::EmptyHandle)
			{
				scripts.OnUpdate =
					Assets::AssetManager::GetScript(scripts.OnUpdateHandle).get();
			}
		}

		// Get Class Functions
		{
			auto allScripts = data["AllScripts"];
			if (allScripts)
			{
				std::set<Assets::AssetHandle>& classScripts  = EntityClass->m_Scripts.AllClassScripts;
				for (auto script : allScripts)
				{
					classScripts.insert(static_cast<Assets::AssetHandle>(script.as<uint64_t>()));
				}
			}
		}

		return true;

	}

	AssetHandle AssetManager::CreateNewEntityClass(const std::string& EntityClassName)
	{
		// Create Checksum
		const std::string currentCheckSum = FileSystem::ChecksumFromString(EntityClassName);

		if (currentCheckSum.empty())
		{
			KG_ERROR("Failed to generate checksum from file!");
			return {};
		}

		// Compare currentChecksum to registered assets
		for (const auto& [handle, asset] : s_EntityClassRegistry)
		{
			if (asset.Data.CheckSum == currentCheckSum)
			{
				KG_INFO("Attempt to instantiate duplicate game state asset");
				return handle;
			}
		}

		// Create New Asset/Handle
		AssetHandle newHandle{};
		Assets::Asset newAsset{};
		newAsset.Handle = newHandle;

		// Create File
		CreateEntityClassFile(EntityClassName, newAsset);
		newAsset.Data.CheckSum = currentCheckSum;

		// Register New Asset and return handle.
		s_EntityClassRegistry.insert({ newHandle, newAsset }); // Update Registry Map in-memory
		SerializeEntityClassRegistry(); // Update Registry File on Disk

		return newHandle;
	}

	void AssetManager::SaveEntityClass(AssetHandle EntityClassHandle, Ref<Kargono::EntityClass> EntityClass)
	{
		if (!s_EntityClassRegistry.contains(EntityClassHandle))
		{
			KG_ERROR("Attempt to save EntityClass that does not exist in registry");
			return;
		}
		Assets::Asset EntityClassAsset = s_EntityClassRegistry[EntityClassHandle];
		SerializeEntityClass(EntityClass, (Projects::Project::GetAssetDirectory() / EntityClassAsset.Data.IntermediateLocation).string());
	}

	void AssetManager::DeleteEntityClass(AssetHandle handle)
	{
		if (!s_EntityClassRegistry.contains(handle))
		{
			KG_WARN("Failed to delete EntityClass in AssetManager");
			return;
		}

		FileSystem::DeleteSelectedFile(Projects::Project::GetAssetDirectory() /
			s_EntityClassRegistry.at(handle).Data.IntermediateLocation);

		s_EntityClassRegistry.erase(handle);

		SerializeEntityClassRegistry();
	}

	std::filesystem::path AssetManager::GetEntityClassLocation(const AssetHandle& handle)
	{
		if (!s_EntityClassRegistry.contains(handle))
		{
			KG_ERROR("Attempt to save EntityClass that does not exist in registry");
			return "";
		}
		return s_EntityClassRegistry[handle].Data.IntermediateLocation;
	}

	Ref<Kargono::EntityClass> AssetManager::GetEntityClass(const AssetHandle& handle)
	{
		KG_ASSERT(Projects::Project::GetActive(), "There is no active project when retreiving EntityClass!");

		if (s_EntityClassRegistry.contains(handle))
		{
			auto asset = s_EntityClassRegistry[handle];
			return InstantiateEntityClass(asset);
		}

		KG_ERROR("No EntityClass is associated with provided handle!");
		return nullptr;
	}
	std::tuple<AssetHandle, Ref<Kargono::EntityClass>> AssetManager::GetEntityClass(const std::filesystem::path& filepath)
	{
		KG_ASSERT(Projects::Project::GetActive(), "Attempt to use Project Field without active project!");

		std::filesystem::path EntityClassPath = filepath;

		if (filepath.is_absolute())
		{
			EntityClassPath = FileSystem::GetRelativePath(Projects::Project::GetAssetDirectory(), filepath);
		}

		for (auto& [assetHandle, asset] : s_EntityClassRegistry)
		{
			if (asset.Data.IntermediateLocation.compare(EntityClassPath) == 0)
			{
				return std::make_tuple(assetHandle, InstantiateEntityClass(asset));
			}
		}
		// Return empty EntityClass if EntityClass does not exist
		KG_WARN("No EntityClass Associated with provided handle. Returned new empty EntityClass");
		AssetHandle newHandle = CreateNewEntityClass(filepath.stem().string());
		return std::make_tuple(newHandle, GetEntityClass(newHandle));
	}

	Ref<Kargono::EntityClass> AssetManager::InstantiateEntityClass(const Assets::Asset& EntityClassAsset)
	{
		Ref<Kargono::EntityClass> newEntityClass = CreateRef<Kargono::EntityClass>();
		DeserializeEntityClass(newEntityClass, (Projects::Project::GetAssetDirectory() / EntityClassAsset.Data.IntermediateLocation).string());
		return newEntityClass;
	}


	void AssetManager::ClearEntityClassRegistry()
	{
		s_EntityClassRegistry.clear();
	}

	void AssetManager::CreateEntityClassFile(const std::string& EntityClassName, Assets::Asset& newAsset)
	{
		// Create Temporary EntityClass
		Ref<Kargono::EntityClass> temporaryEntityClass = CreateRef<Kargono::EntityClass>();
		temporaryEntityClass->SetName(EntityClassName);

		// Save Binary Intermediate into File
		std::string EntityClassPath = "EntityClass/" + EntityClassName + ".kgclass";
		std::filesystem::path intermediateFullPath = Projects::Project::GetAssetDirectory() / EntityClassPath;
		SerializeEntityClass(temporaryEntityClass, intermediateFullPath.string());

		// Load data into In-Memory Metadata object
		newAsset.Data.Type = Assets::AssetType::EntityClass;
		newAsset.Data.IntermediateLocation = EntityClassPath;
		Ref<Assets::EntityClassMetaData> metadata = CreateRef<Assets::EntityClassMetaData>();
		metadata->Name = EntityClassName;
		newAsset.Data.SpecificFileData = metadata;
	}

	bool AssetManager::DeserializeServerVariables(Ref<Projects::Project> project, const std::filesystem::path& projectPath)
	{
		auto& config = project->m_Config;

		std::string filepath = (projectPath.parent_path() / "server_variables.env").string();

		YAML::Node data;
		try
		{
			data = YAML::LoadFile(filepath);
		}
		catch (YAML::ParserException e)
		{
			KG_ERROR("Failed to load server variables '{0}'\n     {1}", filepath, e.what());
			return false;
		}
		auto rootNode = data["ServerVariables"];
		if (!rootNode) { return false; }

		config.ServerIP = rootNode["ServerIP"].as<std::string>();
		config.ServerPort = static_cast<uint16_t>(rootNode["ServerPort"].as<uint32_t>());
		config.ServerLocation = rootNode["ServerLocation"].as<std::string>();
		config.SecretOne = rootNode["SecretOne"].as<uint64_t>();
		config.SecretTwo = rootNode["SecretTwo"].as<uint64_t>();
		config.SecretThree = rootNode["SecretThree"].as<uint64_t>();
		config.SecretFour = rootNode["SecretFour"].as<uint64_t>();

		return true;
	}


	Ref<Projects::Project> AssetManager::NewProject()
	{
		Projects::Project::s_ActiveProject = CreateRef<Projects::Project>();
		return Projects::Project::s_ActiveProject;
	}
	Ref<Projects::Project> AssetManager::OpenProject(const std::filesystem::path& path)
	{
		Ref<Projects::Project> project = CreateRef<Projects::Project>();
		if (Assets::AssetManager::DeserializeProject(project, path))
		{
			project->m_ProjectDirectory = path.parent_path();
			Projects::Project::s_ActiveProject = project;
			return Projects::Project::s_ActiveProject;
		}

		return nullptr;

	}
	bool AssetManager::SaveActiveProject(const std::filesystem::path& path)
	{
		if (Assets::AssetManager::SerializeProject(Projects::Project::s_ActiveProject, path))
		{
			Projects::Project::s_ActiveProject->m_ProjectDirectory = path.parent_path();
			return true;
		}
		return false;
	}

	bool AssetManager::SerializeProject(Ref<Projects::Project> project, const std::filesystem::path& filepath)
	{
		const auto& config = project->m_Config;
		YAML::Emitter out;

		{
			out << YAML::BeginMap; // Root
			out << YAML::Key << "Project" << YAML::Value;
			{
				out << YAML::BeginMap; // Project
				out << YAML::Key << "Name" << YAML::Value << config.Name;
				out << YAML::Key << "StartScene" << YAML::Value << config.StartScenePath.string();
				out << YAML::Key << "StartSceneHandle" << YAML::Value << static_cast<uint64_t>(config.StartSceneHandle);
				out << YAML::Key << "StartGameState" << YAML::Value << static_cast<uint64_t>(config.StartGameState);
				out << YAML::Key << "AssetDirectory" << YAML::Value << config.AssetDirectory.string();
				out << YAML::Key << "ScriptModulePath" << YAML::Value << config.ScriptModulePath.string();
				out << YAML::Key << "ScriptDLLPath" << YAML::Value << config.ScriptDLLPath.string();
				out << YAML::Key << "DefaultFullscreen" << YAML::Value << config.DefaultFullscreen;
				out << YAML::Key << "TargetResolution" << YAML::Value << Utility::ScreenResolutionToString(config.TargetResolution);
				out << YAML::Key << "OnRuntimeStart" << YAML::Value << static_cast<uint64_t>(config.OnRuntimeStart);
				out << YAML::Key << "OnUpdateUserCount" << YAML::Value << static_cast<uint64_t>(config.OnUpdateUserCount);
				out << YAML::Key << "OnApproveJoinSession" << YAML::Value << static_cast<uint64_t>(config.OnApproveJoinSession);
				out << YAML::Key << "OnUserLeftSession" << YAML::Value << static_cast<uint64_t>(config.OnUserLeftSession);
				out << YAML::Key << "OnCurrentSessionInit" << YAML::Value << static_cast<uint64_t>(config.OnCurrentSessionInit);
				out << YAML::Key << "OnConnectionTerminated" << YAML::Value << static_cast<uint64_t>(config.OnConnectionTerminated);
				out << YAML::Key << "OnUpdateSessionUserSlot" << YAML::Value << static_cast<uint64_t>(config.OnUpdateSessionUserSlot);
				out << YAML::Key << "OnStartSession" << YAML::Value << static_cast<uint64_t>(config.OnStartSession);
				out << YAML::Key << "OnSessionReadyCheckConfirmFunction" << YAML::Value << config.OnSessionReadyCheckConfirmFunction;
				out << YAML::Key << "OnReceiveSignalFunction" << YAML::Value << config.OnReceiveSignalFunction;
				out << YAML::Key << "AppIsNetworked" << YAML::Value << config.AppIsNetworked;

				if (config.AppTickGenerators.size() > 0)
				{
					out << YAML::Key << "AppTickGenerators" << YAML::BeginSeq;
					// Serialize App Tick Generators
					for (auto generatorValue : config.AppTickGenerators)
					{
						out << YAML::Value << generatorValue;
					}
					out << YAML::EndSeq;
				}

				out << YAML::EndMap; // Project
			}

			out << YAML::EndMap; // Root
		}

		std::ofstream fout(filepath);
		fout << out.c_str();

		return true;
	}

	bool AssetManager::DeserializeProject(Ref<Projects::Project> project, const std::filesystem::path& filepath)
	{
		auto& config = project->m_Config;

		YAML::Node data;
		try
		{
			data = YAML::LoadFile(filepath.string());
		}
		catch (YAML::ParserException e)
		{
			KG_ERROR("Failed to load project file '{0}'\n     {1}", filepath, e.what());
			return false;
		}
		auto projectNode = data["Project"];
		if (!projectNode) { return false; }

		config.Name = projectNode["Name"].as<std::string>();
		config.StartScenePath = projectNode["StartScene"].as<std::string>();
		config.StartSceneHandle = static_cast<AssetHandle>(projectNode["StartSceneHandle"].as<uint64_t>());
		config.StartGameState = static_cast<AssetHandle>(projectNode["StartGameState"].as<uint64_t>());
		config.AssetDirectory = projectNode["AssetDirectory"].as<std::string>();
		config.ScriptModulePath = projectNode["ScriptModulePath"].as<std::string>();
		config.ScriptDLLPath = projectNode["ScriptDLLPath"].as<std::string>();
		config.DefaultFullscreen = projectNode["DefaultFullscreen"].as<bool>();
		config.TargetResolution = Utility::StringToScreenResolution(projectNode["TargetResolution"].as<std::string>());
		config.OnRuntimeStart = static_cast<AssetHandle>(projectNode["OnRuntimeStart"].as<uint64_t>());
		config.OnUpdateUserCount = static_cast<AssetHandle>(projectNode["OnUpdateUserCount"].as<uint64_t>());
		config.OnApproveJoinSession = static_cast<AssetHandle>(projectNode["OnApproveJoinSession"].as<uint64_t>());
		config.OnUserLeftSession = static_cast<AssetHandle>(projectNode["OnUserLeftSession"].as<uint64_t>());
		config.OnCurrentSessionInit = static_cast<AssetHandle>(projectNode["OnCurrentSessionInit"].as<uint64_t>());
		config.OnConnectionTerminated = static_cast<AssetHandle>(projectNode["OnConnectionTerminated"].as<uint64_t>());
		config.OnUpdateSessionUserSlot = static_cast<AssetHandle>(projectNode["OnUpdateSessionUserSlot"].as<uint64_t>());
		config.OnStartSession = static_cast<AssetHandle>(projectNode["OnStartSession"].as<uint64_t>());
		config.OnSessionReadyCheckConfirmFunction = projectNode["OnSessionReadyCheckConfirmFunction"].as<std::string>();
		config.OnReceiveSignalFunction = projectNode["OnReceiveSignalFunction"].as<std::string>();
		config.AppIsNetworked = projectNode["AppIsNetworked"].as<bool>();

		auto tickGenerators = projectNode["AppTickGenerators"];

		if (tickGenerators)
		{
			for (auto generator : tickGenerators)
			{
				uint64_t value = generator.as<uint64_t>();
				config.AppTickGenerators.insert(value);
			}
		}

		DeserializeServerVariables(project, filepath);

		return true;
	}

}
