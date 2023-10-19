#include "kgpch.h"

#include "Kargono/Assets/AssetManager.h"

#include "Kargono/Core/FileSystem.h"
#include "Kargono/Core/Timer.h"
#include "Kargono/Project/Project.h"
#include "Kargono/Renderer/Shader.h"
#include "API/Serialization/SerializationAPI.h"


#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>

namespace Kargono
{
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
		KG_CORE_TRACE("Deserializing Registry");

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
					shaderMetaData->ShaderSpec.AddTexture = metadata["AddTexture"].as<bool>();
					shaderMetaData->ShaderSpec.RenderType = Shape::StringToRenderingType(metadata["RenderType"].as<std::string>());

					KG_CORE_ASSERT(sizeof(uint8_t) * 12 == sizeof(Shader::ShaderSpecification), "Please Update Deserialization and Serialization. Incorrect size of input data in Shader Deserializer!")
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
				out << YAML::Key << "AddTexture" << YAML::Value << metadata->ShaderSpec.AddTexture;
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
				return std::make_tuple(assetHandle, newShader );
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
		CompileBinaries(newAsset.Handle , shaderSources, openGLSPIRV);

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

}
