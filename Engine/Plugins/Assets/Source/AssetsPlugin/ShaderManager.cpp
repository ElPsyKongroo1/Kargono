#include "kgpch.h"

#include "AssetsPlugin/AssetService.h"
#include "AssetsPlugin/ShaderManager.h"
#include "RenderingPlugin/Shader.h"
#include "RenderingPlugin/ShaderBuilder.h"

#include "API/Serialization/yamlcppAPI.h"
#include "RenderingPlugin/ExternalAPI/VulkanAPI.h"
#include "API/Platform/gladAPI.h"

namespace Kargono::Utility
{
	static GLenum ShaderTypeFromString(std::string_view type)
	{
		if (type == "vertex")
			return GL_VERTEX_SHADER;
		if (type == "fragment" || type == "pixel")
			return GL_FRAGMENT_SHADER;

		KG_ERROR("Unknown shader type!");
		return 0;
	}

	/*static const char* ShaderTypeToString(GLenum stage)
	{
		switch (stage)
		{
		case GL_VERTEX_SHADER:   return "vertex";
		case GL_FRAGMENT_SHADER: return "fragment";
		}
		KG_ERROR("Invalid Shader Type!");
		return "";
	}*/

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

	static std::unordered_map<GLenum, std::string> PreProcess(std::string_view source)
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
			std::string_view type = source.substr(begin, eol - begin);
			KG_ASSERT(Utility::ShaderTypeFromString(type), "Invalid shader type specified");

			size_t nextLinePos = source.find_first_not_of("\r\n", eol); //Start of shader code after shader type declaration line
			KG_ASSERT(nextLinePos != std::string::npos, "Syntax error");
			pos = source.find(typeToken, nextLinePos); //Start of next shader type declaration line

			shaderSources[Utility::ShaderTypeFromString(type)] = (pos == std::string::npos) ? source.substr(nextLinePos) : source.substr(nextLinePos, pos - nextLinePos);
		}

		return shaderSources;
	}
#if !defined(KG_EXPORT_SERVER) && !defined(KG_EXPORT_RUNTIME)
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

	static void CompileBinaries(const Assets::AssetHandle& assetHandle, const std::unordered_map<GLenum, std::string>& shaderSources, std::unordered_map<GLenum, std::vector<uint32_t>>& openGLSPIRV)
	{
		//GLuint program = glCreateProgram(); // TODO: For some reason, program is not used here?
		glCreateProgram();

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
#endif
}



namespace Kargono::Assets
{
	AssetHandle ShaderManager::CreateNewShader(const Rendering::ShaderSpecification& shaderSpec)
	{
		// Create Checksum
		auto [shaderSource, bufferLayout, uniformList] = Rendering::ShaderBuilder::BuildShader(shaderSpec);
		std::string currentCheckSum = Utility::FileSystem::ChecksumFromString(shaderSource.c_str());

		// Ensure checksum is valid
		if (currentCheckSum.empty())
		{
			KG_WARN("Generated empty checksum from the string {}", shaderSource);
			return Assets::EmptyHandle;
		}

		// Ensure duplicate asset is not found in registry.
		for (const auto& [handle, asset] : m_AssetRegistry)
		{
			if (asset.Data.CheckSum == currentCheckSum)
			{
				KG_WARN("Attempt to instantiate duplicate {} asset. Returning existing asset.", m_AssetName);
				return handle;
			}
		}

		// Create New Asset/Handle
		AssetHandle newHandle{};
		Assets::AssetInfo newAsset{};
		newAsset.m_Handle = newHandle;
		newAsset.Data.Type = m_AssetType;
		newAsset.Data.CheckSum = currentCheckSum;
		newAsset.Data.IntermediateLocation = m_RegistryLocation.parent_path() / (std::string)newHandle;

		// Create intermediate and save intermediate to disk
		CreateShaderIntermediate(shaderSource, newAsset, shaderSpec, bufferLayout, uniformList);

		// Register New Asset and Create Shader
		m_AssetRegistry.insert({ newHandle, newAsset }); // Update Registry Map in-memory
		SerializeAssetRegistry(); // Update Registry File on Disk
		Ref<Kargono::Rendering::Shader> newShader = DeserializeAsset(newAsset, Projects::ProjectService::GetActiveIntermediateDirectory() / newAsset.Data.IntermediateLocation);
		m_AssetCache.insert({ newHandle, newShader });

		Ref<Events::ManageAsset> event = CreateRef<Events::ManageAsset>
		(
			newHandle, 
			AssetType::Shader, 
			Events::ManageAssetAction::Create
		);
		EngineService::SubmitToEventQueue(event);

		return newHandle;
	}

	std::tuple<AssetHandle, Ref<Kargono::Rendering::Shader>> ShaderManager::GetShader(const Rendering::ShaderSpecification& shaderSpec)
	{
		KG_ASSERT(Projects::ProjectService::GetActive(), "There is no active project when retrieving shader!");

		for (const auto& [assetHandle, shaderRef] : m_AssetCache)
		{
			if (shaderRef->GetSpecification() == shaderSpec)
			{
				return std::make_tuple(assetHandle, shaderRef);
			}
		}

		for (auto& [assetHandle, asset] : m_AssetRegistry)
		{
			Assets::ShaderMetaData metadata = *static_cast<Assets::ShaderMetaData*>(asset.Data.SpecificFileData.get());
			if (metadata.ShaderSpec == shaderSpec)
			{
				Ref<Kargono::Rendering::Shader> newShader = DeserializeAsset(asset, 
					Projects::ProjectService::GetActiveIntermediateDirectory() / asset.Data.IntermediateLocation);
				m_AssetCache.insert({ asset.m_Handle, newShader });
				return std::make_tuple(assetHandle, newShader);
			}
		}

		// Create New Shader if none exists already
		AssetHandle newShaderHandle = CreateNewShader(shaderSpec);
		return std::make_tuple(newShaderHandle, GetAsset(newShaderHandle));

	}

	void ShaderManager::CreateShaderIntermediate(const Rendering::ShaderSource& shaderSource, Assets::AssetInfo& newAsset, const Rendering::ShaderSpecification& shaderSpec,
		const Rendering::InputBufferLayout& inputLayout, const Rendering::UniformBufferList& uniformLayout)
	{
#if defined(KG_EXPORT_SERVER) || defined(KG_EXPORT_RUNTIME)
		KG_ERROR("Attempt to create/compile new shader during runtime!");
#endif
		// Create Shader Binary
		auto shaderSources = Utility::PreProcess(shaderSource);

		std::unordered_map<GLenum, std::vector<uint32_t>> openGLSPIRV;
#if !defined(KG_EXPORT_SERVER) && !defined(KG_EXPORT_RUNTIME)
		Utility::CompileBinaries(newAsset.m_Handle, shaderSources, openGLSPIRV);
#endif

		// Save binary intermediates for all shader stages!
		for (const auto& [stage, source] : openGLSPIRV)
		{
			std::string intermediatePathWithExtension = newAsset.Data.IntermediateLocation.string() + Utility::ShaderBinaryFileExtension(stage);
			std::filesystem::path intermediateFullPath = Projects::ProjectService::GetActiveIntermediateDirectory() / intermediatePathWithExtension;

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

		// File Location
		std::string debugString = shaderSource;
		std::filesystem::path shaderTextFile = Projects::ProjectService::GetActiveIntermediateDirectory() / (newAsset.Data.IntermediateLocation.string() + ".source");
		Utility::FileSystem::WriteFileString(shaderTextFile, debugString);

		// Load In-Memory Metadata Object
		newAsset.Data.FileLocation = shaderTextFile;
		Ref<Assets::ShaderMetaData> metadata = CreateRef<Assets::ShaderMetaData>();
		metadata->ShaderSpec = shaderSpec;
		metadata->InputLayout = inputLayout;
		metadata->UniformList = uniformLayout;
		newAsset.Data.SpecificFileData = metadata;
	}


	Ref<Rendering::Shader> Assets::ShaderManager::DeserializeAsset(Assets::AssetInfo& asset, const std::filesystem::path& assetPath)
	{
		UNREFERENCED_PARAMETER(assetPath);

		Assets::ShaderMetaData metadata = *asset.Data.GetSpecificMetaData<ShaderMetaData>();
		std::unordered_map<GLenum, std::vector<uint32_t>> openGLSPIRV;
		std::filesystem::path intermediatePath = Projects::ProjectService::GetActiveIntermediateDirectory() / asset.Data.IntermediateLocation;
		static std::array<std::string, 2> stageTypes = { "vertex", "fragment" };

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

		Ref<Kargono::Rendering::Shader> newShader = Rendering::Shader::Create(static_cast<std::string>(asset.m_Handle), openGLSPIRV);
		newShader->SetSpecification(metadata.ShaderSpec);
		newShader->SetInputLayout(metadata.InputLayout);
		newShader->SetUniformList(metadata.UniformList);
		openGLSPIRV.clear();
		return newShader;
	}
	void Assets::ShaderManager::SerializeAssetSpecificMetadata(YAML::Emitter& serializer, Assets::AssetInfo& currentAsset)
	{
		// ShaderSpecification Section
		Assets::ShaderMetaData* metadata = static_cast<Assets::ShaderMetaData*>(currentAsset.Data.SpecificFileData.get());
		serializer << YAML::Key << "ColorInputType" << YAML::Value << Utility::ColorInputTypeToString(metadata->ShaderSpec.ColorInput);
		serializer << YAML::Key << "AddProjectionMatrix" << YAML::Value << metadata->ShaderSpec.AddProjectionMatrix;
		serializer << YAML::Key << "AddEntityID" << YAML::Value << metadata->ShaderSpec.AddEntityID;
		serializer << YAML::Key << "AddCircleShape" << YAML::Value << metadata->ShaderSpec.AddCircleShape;
		serializer << YAML::Key << "TextureInput" << YAML::Value << Utility::TextureInputTypeToString(metadata->ShaderSpec.TextureInput);
		serializer << YAML::Key << "DrawOutline" << YAML::Value << metadata->ShaderSpec.DrawOutline;
		serializer << YAML::Key << "RenderType" << YAML::Value << Utility::RenderingTypeToString(metadata->ShaderSpec.RenderType);

		// InputBufferLayout Section
		serializer << YAML::Key << "InputBufferLayout" << YAML::Value << YAML::BeginMap; // Input Buffer Layout Map
		serializer << YAML::Key << "Elements" << YAML::Value << YAML::BeginSeq;
		for (const auto& element : metadata->InputLayout.GetElements())
		{
			serializer << YAML::BeginMap; // Input Element Map
			serializer << YAML::Key << "Name" << YAML::Value << element.Name;
			serializer << YAML::Key << "Type" << YAML::Value << Utility::InputDataTypeToString(element.Type);
			serializer << YAML::EndMap; // Input Element Map
		}
		serializer << YAML::EndSeq;
		serializer << YAML::EndMap; // Input Buffer Layout Map

		// UniformBufferList Section
		serializer << YAML::Key << "UniformBufferList" << YAML::Value << YAML::BeginMap; // Uniform Buffer Layout Map
		serializer << YAML::Key << "Elements" << YAML::Value << YAML::BeginSeq;
		for (const auto& element : metadata->UniformList.GetElements())
		{
			serializer << YAML::BeginMap; // Uniform Element Map
			serializer << YAML::Key << "Name" << YAML::Value << element.Name;
			serializer << YAML::Key << "Type" << YAML::Value << Utility::UniformDataTypeToString(element.Type);
			serializer << YAML::EndMap; // Uniform Element Map
		}
		serializer << YAML::EndSeq;
		serializer << YAML::EndMap; // Uniform Buffer Layout Map
	}
	void Assets::ShaderManager::DeserializeAssetSpecificMetadata(YAML::Node& metadataNode, Assets::AssetInfo& currentAsset)
	{
		Ref<Assets::ShaderMetaData> shaderMetaData = CreateRef<Assets::ShaderMetaData>();

		// ShaderSpecification Section
		shaderMetaData->ShaderSpec.ColorInput = Utility::StringToColorInputType(metadataNode["ColorInputType"].as<std::string>());
		shaderMetaData->ShaderSpec.AddProjectionMatrix = metadataNode["AddProjectionMatrix"].as<bool>();
		shaderMetaData->ShaderSpec.AddEntityID = metadataNode["AddEntityID"].as<bool>();
		shaderMetaData->ShaderSpec.AddCircleShape = metadataNode["AddCircleShape"].as<bool>();
		shaderMetaData->ShaderSpec.TextureInput = Utility::StringToTextureInputType(metadataNode["TextureInput"].as<std::string>());
		shaderMetaData->ShaderSpec.DrawOutline = metadataNode["DrawOutline"].as<bool>();
		shaderMetaData->ShaderSpec.RenderType = Utility::StringToRenderingType(metadataNode["RenderType"].as<std::string>());

		static_assert(sizeof(uint8_t) * 20 == sizeof(Rendering::ShaderSpecification));

		{
			// InputBufferLayout Section
			auto inputBufferLayout = metadataNode["InputBufferLayout"];
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
			auto uniformBufferList = metadataNode["UniformBufferList"];
			auto elementList = uniformBufferList["Elements"];
			for (const auto& element : elementList)
			{
				shaderMetaData->UniformList.AddBufferElement(Rendering::UniformElement(
					Utility::StringToUniformDataType(element["Type"].as<std::string>()),
					element["Name"].as<std::string>()
				));
			}
		}
		currentAsset.Data.SpecificFileData = shaderMetaData;
	}
}
