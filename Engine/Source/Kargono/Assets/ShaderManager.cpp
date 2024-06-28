#include "kgpch.h"

#include "Kargono/Assets/AssetManager.h"
#include "Kargono/Projects/Project.h"
#include "Kargono/Utility/FileSystem.h"
#include "Kargono/Rendering/Shader.h"
#include "Kargono/Rendering/ShaderBuilder.h"

#include "API/Serialization/yamlcppAPI.h"
#include "API/RenderingAPI/VulkanAPI.h"
#include "API/Platform/gladAPI.h"

namespace Kargono::Utility
{
	static GLenum ShaderTypeFromString(const std::string& type)
	{
		if (type == "vertex")
			return GL_VERTEX_SHADER;
		if (type == "fragment" || type == "pixel")
			return GL_FRAGMENT_SHADER;

		KG_ERROR("Unknown shader type!");
		return 0;
	}

	static std::string ShaderTypeToString(GLenum stage)
	{
		switch (stage)
		{
		case GL_VERTEX_SHADER:   return "vertex";
		case GL_FRAGMENT_SHADER: return "fragment";
		}
		KG_ERROR("Invalid Shader Type!");
		return "";
	}

	static shaderc_shader_kind GLShaderStageToShaderC(GLenum stage)
	{
		switch (stage)
		{
		case GL_VERTEX_SHADER:   return shaderc_glsl_vertex_shader;
		case GL_FRAGMENT_SHADER: return shaderc_glsl_fragment_shader;
		}
		KG_ERROR("Invalid Shader Type!");
		return (shaderc_shader_kind)0;
	}

	static const char* GLShaderStageToString(GLenum stage)
	{
		switch (stage)
		{
		case GL_VERTEX_SHADER:   return "GL_VERTEX_SHADER";
		case GL_FRAGMENT_SHADER: return "GL_FRAGMENT_SHADER";
		}
		KG_ERROR("Invalid Shader Type!");
		return nullptr;
	}

	static const char* ShaderBinaryFileExtension(uint32_t stage)
	{
		switch (stage)
		{
		case GL_VERTEX_SHADER:    return ".kgshadervert";
		case GL_FRAGMENT_SHADER:  return ".kgshaderfrag";
		}
		KG_ERROR("Invalid Shader Extension Type");
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

				KG_CRITICAL(module.GetErrorMessage());
				KG_CRITICAL("Here are the shaders: ");
				for (auto& [enumName, text] : shaderSources)
				{
					KG_CRITICAL(text);
				}
				KG_CRITICAL("Failure ");
			}
			// Add Newly Compiled Spirv to m_OpenGLSPIRV
			shaderData[stage] = std::vector<uint32_t>(module.cbegin(), module.cend());
		}
	}
}

namespace Kargono::Assets
{

	std::unordered_map<AssetHandle, Assets::Asset> AssetManager::s_ShaderRegistry {};
	std::unordered_map<AssetHandle, Ref<Kargono::Rendering::Shader>> AssetManager::s_Shaders {};

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

					KG_ASSERT(sizeof(uint8_t) * 20 == sizeof(Rendering::ShaderSpecification), "Please Update Deserialization and Serialization. Incorrect size of input data in Shader Deserializer!")
					{
						// InputBufferLayout Section
						auto inputBufferLayout = metadata["InputBufferLayout"];
						auto elementList = inputBufferLayout["Elements"];
						for (const auto& element : elementList)
						{
							shaderMetaData->InputLayout.AddBufferElement(Rendering::InputBufferElement(
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
							shaderMetaData->UniformList.AddBufferElement(Rendering::UniformElement(
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

		Utility::FileSystem::CreateNewDirectory(shaderRegistryLocation.parent_path());

		std::ofstream fout(shaderRegistryLocation);
		fout << out.c_str();
	}

	AssetHandle AssetManager::CreateNewShader(const Rendering::ShaderSpecification& shaderSpec)
	{
		// Create Checksum
		auto [shaderSource, bufferLayout, uniformList] = Rendering::ShaderBuilder::BuildShader(shaderSpec);
		std::string currentCheckSum = Utility::FileSystem::ChecksumFromString(shaderSource);

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
		Ref<Kargono::Rendering::Shader> newShader = InstantiateShaderIntoMemory(newAsset);
		s_Shaders.insert({ newHandle, newShader });

		return newHandle;
	}

	Ref<Kargono::Rendering::Shader> AssetManager::GetShader(const AssetHandle& handle)
	{
		KG_ASSERT(Projects::Project::GetActive(), "There is no active project when retrieving shader!");

		if (s_Shaders.contains(handle)) { return s_Shaders[handle]; }

		if (s_ShaderRegistry.contains(handle))
		{
			auto asset = s_ShaderRegistry[handle];

			Ref<Kargono::Rendering::Shader> newShader = InstantiateShaderIntoMemory(asset);
			s_Shaders.insert({ asset.Handle, newShader });
			return newShader;
		}

		//KG_CORE_INFO("No Shader is associated with provided asset handle!");
		return nullptr;
	}

	std::tuple<AssetHandle, Ref<Kargono::Rendering::Shader>> AssetManager::GetShader(const Rendering::ShaderSpecification& shaderSpec)
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
				Ref<Kargono::Rendering::Shader> newShader = InstantiateShaderIntoMemory(asset);
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

	Ref<Kargono::Rendering::Shader> AssetManager::InstantiateShaderIntoMemory(Assets::Asset& asset)
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

		Ref<Kargono::Rendering::Shader> newShader = Rendering::Shader::Create(static_cast<std::string>(asset.Handle), openGLSPIRV);
		newShader->SetSpecification(metadata.ShaderSpec);
		newShader->SetInputLayout(metadata.InputLayout);
		newShader->SetUniformList(metadata.UniformList);
		openGLSPIRV.clear();
		return newShader;
	}


	void AssetManager::CreateShaderIntermediate(const Rendering::ShaderSource& shaderSource, Assets::Asset& newAsset, const Rendering::ShaderSpecification& shaderSpec,
		const Rendering::InputBufferLayout& inputLayout, const Rendering::UniformBufferList& uniformLayout)
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

			Utility::FileSystem::CreateNewDirectory(intermediateFullPath.parent_path());
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
		Utility::FileSystem::WriteFileString(debugPath, debugString);
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
