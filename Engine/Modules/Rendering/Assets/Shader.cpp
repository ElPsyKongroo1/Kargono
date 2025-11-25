#include "kgpch.h"

#include "Modules/Rendering/Assets/Shader.h"
#include "Modules/Rendering/RenderingService.h"
#include "Modules/Scenes/Assets/Scene.h"
#include "Modules/FileSystem/FileSystem.h"
#include "Modules/Rendering/Components/ShapeComponent.h"
#include "Modules/Rendering/ShaderBuilder.h"
#include "Modules/Assets/Managers/ShaderManager.h"

#include "Modules/Rendering/ExternalAPI/VulkanAPI.h"
#include "API/Platform/gladAPI.h"

namespace Kargono::Utility
{
	static GLenum ShaderTypeFromExtension(std::string_view type)
	{
		if (type == Rendering::Shader::k_IntermediateExtensions[1].StringView())
		{
			return GL_FRAGMENT_SHADER;
		}
		if (type == Rendering::Shader::k_IntermediateExtensions[2].StringView())
		{
			return GL_VERTEX_SHADER;
		}

		KG_ERROR("Unknown shader extension!");
		return 0;
	}

	static GLenum ShaderTypeFromString(std::string_view type)
	{
		if (type == "vertex")
		{
			return GL_VERTEX_SHADER;
		}
		if (type == "fragment" || "pixel")
		{
			return GL_FRAGMENT_SHADER;
		}

		KG_ERROR("Unknown shader type!");
		return 0;
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
		case GL_FRAGMENT_SHADER:  return Rendering::Shader::k_IntermediateExtensions[1].CString();
		case GL_VERTEX_SHADER:    return Rendering::Shader::k_IntermediateExtensions[2].CString();
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

namespace Kargono::Rendering
{
	void RendererInputSpec::ClearData()
	{
		m_Shader.Reset();
		m_Texture.Reset();
		if (m_ShapeComponent)
		{
			m_ShapeComponent->m_Texture.Reset();
			m_ShapeComponent->m_Shader.Reset();
		}
		m_CurrentDrawBuffer = nullptr;
	}

	ShaderSpecification::ShaderSpecification(ColorInputType colorInput, TextureInputType textureInput,
		bool addCircle, bool addProjection, bool addEntityID,
		RenderingType renderType, bool drawOutline)
		: m_ColorInput(colorInput), m_TextureInput(textureInput), m_AddCircleShape(addCircle), 
		m_AddProjectionMatrix(addProjection), m_AddEntityID(addEntityID), 
		m_RenderType(renderType), m_DrawOutline(drawOutline)
	{
	}

	void ShaderMetaData::Serialize(void* context)
	{
		// Get asset context
		KG_ASSERT(context, "Context cannot be null");
		Assets::SerializeMetaDataContext<Shader>& metadataContext = *(Assets::SerializeMetaDataContext<Shader>*)context;

		// Get context fields
		YAML::Emitter& emitter = *metadataContext.m_Serializer;

		// Shader specification section
		emitter << YAML::Key << "ColorInputType" << YAML::Value << Utility::ColorInputTypeToString(m_ShaderSpec.m_ColorInput);
		emitter << YAML::Key << "AddProjectionMatrix" << YAML::Value << m_ShaderSpec.m_AddProjectionMatrix;
		emitter << YAML::Key << "AddEntityID" << YAML::Value << m_ShaderSpec.m_AddEntityID;
		emitter << YAML::Key << "AddCircleShape" << YAML::Value << m_ShaderSpec.m_AddCircleShape;
		emitter << YAML::Key << "TextureInput" << YAML::Value << Utility::TextureInputTypeToString(m_ShaderSpec.m_TextureInput);
		emitter << YAML::Key << "DrawOutline" << YAML::Value << m_ShaderSpec.m_DrawOutline;
		emitter << YAML::Key << "RenderType" << YAML::Value << Utility::RenderingTypeToString(m_ShaderSpec.m_RenderType);

		// Input buffer layout section
		emitter << YAML::Key << "InputBufferLayout" << YAML::Value << YAML::BeginMap; // Input Buffer Layout Map
		emitter << YAML::Key << "Elements" << YAML::Value << YAML::BeginSeq;
		for (const auto& element : m_InputLayout.GetElements())
		{
			emitter << YAML::BeginMap; // Input Element Map
			emitter << YAML::Key << "Name" << YAML::Value << element.m_Name;
			emitter << YAML::Key << "Type" << YAML::Value << Utility::InputDataTypeToString(element.m_Type);
			emitter << YAML::EndMap; // Input Element Map
		}
		emitter << YAML::EndSeq;
		emitter << YAML::EndMap; // Input Buffer Layout Map

		// Uniform buffer list section
		emitter << YAML::Key << "UniformBufferList" << YAML::Value << YAML::BeginMap; // Uniform Buffer Layout Map
		emitter << YAML::Key << "Elements" << YAML::Value << YAML::BeginSeq;
		for (const auto& element : m_UniformList.GetElements())
		{
			emitter << YAML::BeginMap; // Uniform Element Map
			emitter << YAML::Key << "Name" << YAML::Value << element.m_Name;
			emitter << YAML::Key << "Type" << YAML::Value << Utility::UniformDataTypeToString(element.m_Type);
			emitter << YAML::EndMap; // Uniform Element Map
		}
		emitter << YAML::EndSeq;
		emitter << YAML::EndMap; // Uniform Buffer Layout Map
	}
	void ShaderMetaData::Deserialize(void* context)
	{
		// Get asset context
		KG_ASSERT(context, "Context cannot be null");
		Assets::DeserializeMetaDataContext<Shader>& assetContext = *(Assets::DeserializeMetaDataContext<Shader>*)context;

		// Get context fields
		YAML::Node& metadataNode = *assetContext.m_Node;

		// Shader specification section
		m_ShaderSpec.m_ColorInput = Utility::StringToColorInputType(metadataNode["ColorInputType"].as<std::string>());
		m_ShaderSpec.m_AddProjectionMatrix = metadataNode["AddProjectionMatrix"].as<bool>();
		m_ShaderSpec.m_AddEntityID = metadataNode["AddEntityID"].as<bool>();
		m_ShaderSpec.m_AddCircleShape = metadataNode["AddCircleShape"].as<bool>();
		m_ShaderSpec.m_TextureInput = Utility::StringToTextureInputType(metadataNode["TextureInput"].as<std::string>());
		m_ShaderSpec.m_DrawOutline = metadataNode["DrawOutline"].as<bool>();
		m_ShaderSpec.m_RenderType = Utility::StringToRenderingType(metadataNode["RenderType"].as<std::string>());

		static_assert(sizeof(uint8_t) * 20 == sizeof(Rendering::ShaderSpecification));

		// Input buffer layout section
		{
			YAML::Node inputBufferLayout = metadataNode["InputBufferLayout"];
			YAML::Node elementList = inputBufferLayout["Elements"];
			for (const auto& element : elementList)
			{
				m_InputLayout.AddBufferElement(Rendering::InputBufferElement(
					Utility::StringToInputDataType(element["Type"].as<std::string>()),
					element["Name"].as<std::string>()
				));
			}
		}
		// Uniform buffer layout section
		{
			YAML::Node uniformBufferList = metadataNode["UniformBufferList"];
			YAML::Node elementList = uniformBufferList["Elements"];
			for (const YAML::Node& element : elementList)
			{
				m_UniformList.AddBufferElement(Rendering::UniformElement(
					Utility::StringToUniformDataType(element["Type"].as<std::string>()),
					element["Name"].as<std::string>()
				));
			}
		}
	}

	void Shader::Serialize(void* context)
	{
		KG_ERROR("Serialize not implemented!");
	}
	void Shader::Deserialize(void* context)
	{
		KG_ASSERT(context, "Context cannot be null");

		// Get asset context
		Assets::DeserializeAssetContext<Shader>& assetContext = *(Assets::DeserializeAssetContext<Shader>*)context;
		KG_ASSERT(assetContext.m_AssetMetadata);
		Assets::Metadata<Shader>* metadata{ assetContext.m_AssetMetadata };
		KG_ASSERT(metadata);

		ShaderMetaData* shaderMetadata = metadata->GetSpecificMetaData();
		KG_ASSERT(shaderMetadata);

		std::unordered_map<GLenum, std::vector<uint32_t>> openGLSPIRV;
		std::array<FixedBufStr16, 2> shaderStageExtensions { k_IntermediateExtensions[1], k_IntermediateExtensions[2] };

		for (const FixedBufStr16& extension : shaderStageExtensions)
		{
			const std::filesystem::path fullPath = metadata->GetAssetFullIntermediatePath(extension.StringView());
			std::ifstream in(fullPath, std::ios::in | std::ios::binary);

			if (in.is_open())
			{
				in.seekg(0, std::ios::end);
				auto size = in.tellg();
				in.seekg(0, std::ios::beg);

				auto& data = openGLSPIRV[Utility::ShaderTypeFromExtension(extension.StringView())];
				data.resize(size / sizeof(uint32_t));
				in.read((char*)data.data(), size);
			}
		}
		RegisterShader(openGLSPIRV);
		m_Name = static_cast<std::string>(metadata->m_Handle);
		SetSpecification(shaderMetadata->m_ShaderSpec);
		SetInputLayout(shaderMetadata->m_InputLayout);
		SetUniformList(shaderMetadata->m_UniformList);
		openGLSPIRV.clear();
	}

	Utility::SHA256Hash Shader::GetHashFromSpec(const ShaderSpecification& spec)
	{
		auto [shaderSource, bufferLayout, uniformList] = ShaderBuilder::BuildShader(spec);
		return Utility::FileSystem::SHA256HashFromString(shaderSource.c_str());
	}

	bool Shader::GetAssetFromSpec(Assets::Metadata<Shader>& metadata, const ShaderSpecification& querySpec)
	{
		Assets::AssetReference<Shader> currentRef = Assets::s_ShaderManager.GetAssetByHandle(metadata.m_Handle);
		Shader& currentShader = currentRef.GetAsset();

		if (currentShader.GetSpecification() == querySpec)
		{
			return true;
		}

		Projects::ProjectPaths& projectPaths{ Projects::ProjectService::GetActiveContext().GetProjectPaths() };

		ShaderMetaData* shaderMetadata = metadata.GetSpecificMetaData();
		if (shaderMetadata->m_ShaderSpec == querySpec)
		{
			return true;
		}

		return false;
	}

	void Rendering::Shader::CreateFromSpec(Assets::Metadata<Shader>& metadata, const ShaderSpecification& spec)
	{
		// Build the shader source from the specification
		auto [shaderSource, bufferLayout, uniformList] = ShaderBuilder::BuildShader(spec);

		// Get the intermediate paths
		const std::filesystem::path shaderSourcePath = 
			metadata.GetAssetFullIntermediatePath(k_IntermediateExtensions[0].StringView());

		// Compile shader(s) to SPIR-V
#if defined(KG_EXPORT_SERVER) || defined(KG_EXPORT_RUNTIME)
		KG_ERROR("Attempt to create/compile new shader during runtime!");
#endif
		auto shaderSources = Utility::PreProcess(shaderSource);

		std::unordered_map<GLenum, std::vector<uint32_t>> openGLSPIRV;
#if !defined(KG_EXPORT_SERVER) && !defined(KG_EXPORT_RUNTIME)
		Utility::CompileBinaries(metadata.m_Handle, shaderSources, openGLSPIRV);
#endif
		Projects::ProjectPaths& projectPaths { Projects::ProjectService::GetActiveContext().GetProjectPaths() };
		Utility::FileSystem::CreateNewDirectory(shaderSourcePath.parent_path());

		// Write out shader binary intermediates for all shader stages
		for (const auto& [stage, source] : openGLSPIRV)
		{
			const std::filesystem::path intermediateFullPath =
				metadata.GetAssetFullIntermediatePath(Utility::ShaderBinaryFileExtension(stage));
			std::ofstream out(intermediateFullPath, std::ios::out | std::ios::binary);
			if (out.is_open())
			{
				auto& data = source;
				out.write((char*)data.data(), data.size() * sizeof(uint32_t));
				out.flush();
				out.close();
			}
		}

		// Write out shader source file for debugging
		std::string debugString = shaderSource;
		Utility::FileSystem::WriteFileString(shaderSourcePath, debugString);

		// Load in-memory metadata object
		ShaderMetaData* shaderMetadata = metadata.GetSpecificMetaData();
		shaderMetadata->m_ShaderSpec = spec;
		shaderMetadata->m_InputLayout = bufferLayout;
		shaderMetadata->m_UniformList = uniformList;
	}

	void Shader::RegisterShader(const std::unordered_map<GLenum, std::vector<uint32_t>>& openGLSPIRV)
	{
		GLuint program = glCreateProgram();

		std::vector<GLuint> shaderIDs;
		for (auto&& [stage, spirv] : openGLSPIRV)
		{
			GLuint shaderID = shaderIDs.emplace_back(glCreateShader(stage));
			glShaderBinary(1, &shaderID, GL_SHADER_BINARY_FORMAT_SPIR_V, spirv.data(), static_cast<GLsizei>(spirv.size() * sizeof(uint32_t)));
			glSpecializeShader(shaderID, "main", 0, nullptr, nullptr);
			glAttachShader(program, shaderID);
		}

		glLinkProgram(program);

		GLint isLinked;
		glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, infoLog.data());
			KG_ERROR("Shader linking failed :\n{0}", infoLog.data());

			glDeleteProgram(program);

			for (auto id : shaderIDs)
				glDeleteShader(id);
		}

		for (auto id : shaderIDs)
		{
			glDetachShader(program, id);
			glDeleteShader(id);
		}

		m_RendererID = program;
		m_Registered = true;
	}

	void Shader::DeregisterShader()
	{
		glDeleteProgram(m_RendererID);
		m_Registered = false;
	}

	void Shader::Bind() const
	{
		glUseProgram(m_RendererID);
	}

	void Shader::Unbind() const
	{
		glUseProgram(0);
	}

	void Shader::SetIntUniform(const char* name, int value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name);
		glUniform1i(location, value);
	}

	void Shader::SetIntArrayUniform(const char* name, int* values, uint32_t count)
	{
		GLint location = glGetUniformLocation(m_RendererID, name);
		glUniform1iv(location, count, values);
	}

	void Shader::SetFloatUniform(const char* name, float value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name);
		glUniform1f(location, value);
	}

	void Shader::SetFloat2Uniform(const char* name, const Kargono::Math::vec2& value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name);
		glUniform2f(location, value.x, value.y);
	}

	void Shader::SetFloat3Uniform(const char* name, const Kargono::Math::vec3& value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name);
		glUniform3f(location, value.x, value.y, value.z);
	}

	void Shader::SetFloat4Uniform(const char* name, const Kargono::Math::vec4& value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name);
		glUniform4f(location, value.x, value.y, value.z, value.w);
	}

	void Shader::SetMat3Uniform(const char* name, const Kargono::Math::mat3& value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name);
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(value));
	}

	void Shader::SetMat4Uniform(const char* name, const Kargono::Math::mat4& value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name);
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
	}


	void Shader::SetSpecification(const ShaderSpecification& shaderSpec)
	{
		m_ShaderSpecification = shaderSpec;
		FillRenderFunctionList();
	}

	void Shader::SetInputLayout(const InputBufferLayout& shaderInputLayout)
	{
		m_InputBufferLayout = shaderInputLayout;
		m_VertexArray->RegisterArray();

		Ref<VertexBuffer> quadVertexBuffer;
		quadVertexBuffer->SetLayout(m_InputBufferLayout);
		m_VertexArray->AddVertexBuffer(quadVertexBuffer);
	}

	void Shader::FillRenderFunctionList()
	{
		if (m_ShaderSpecification.m_RenderType == RenderingType::DrawLine)
		{
			m_DrawFunctions.push_back(RenderingService::DrawBufferLine);
		}
		if (m_ShaderSpecification.m_RenderType == RenderingType::DrawPoint)
		{
			m_DrawFunctions.push_back(RenderingService::DrawBufferPoints);
		}

		if (m_ShaderSpecification.m_RenderType == RenderingType::DrawLine ||
			m_ShaderSpecification.m_RenderType == RenderingType::DrawPoint ||
			m_ShaderSpecification.m_TextureInput == TextureInputType::TextTexture)
		{
			m_FillDataPerVertex.push_back(RenderingService::FillWorldPositionNoTransform);
		}
		else
		{
			m_FillDataPerVertex.push_back(RenderingService::FillWorldPosition);
		}

		if (m_ShaderSpecification.m_AddEntityID)
		{
			m_FillDataInScene.push_back(RenderingService::FillEntityID);
		}

		if (m_ShaderSpecification.m_TextureInput == TextureInputType::ColorTexture)
		{
			m_FillDataPerObject.push_back(RenderingService::FillTextureIndex);
			m_FillDataPerVertex.push_back(RenderingService::FillTextureCoordinate);
			m_SubmitUniforms.push_back(RenderingService::FillTextureUniform);
		}

		if (m_ShaderSpecification.m_TextureInput == TextureInputType::TextTexture)
		{

			m_FillDataPerObject.push_back(RenderingService::FillTextureAtlas);
			m_FillDataPerVertex.push_back(RenderingService::FillTextureCoordinate);
			m_SubmitUniforms.push_back(RenderingService::FillTextureUniform);
		}

		if (m_ShaderSpecification.m_ColorInput == ColorInputType::VertexColor)
		{
			m_FillDataPerVertex.push_back(RenderingService::FillVertexColor);
		}

		if (m_ShaderSpecification.m_AddCircleShape)
		{
			m_FillDataPerVertex.push_back(RenderingService::FillLocalPosition);
		}

		if (m_ShaderSpecification.m_RenderType == RenderingType::DrawIndex)
		{
			m_FillDataPerObject.push_back(RenderingService::FillIndicesData);
			m_DrawFunctions.push_back(RenderingService::DrawBufferIndices);
		}

		if (m_ShaderSpecification.m_RenderType == RenderingType::DrawTriangle)
		{
			m_DrawFunctions.push_back(RenderingService::DrawBufferTriangles);
		}

		static_assert(sizeof(ShaderSpecification) == sizeof(uint8_t) * 20);
	}
}
