#include "kgpch.h"

#include "Modules/Rendering/Assets/Shader.h"
#include "Modules/Rendering/RenderingService.h"
#include "Modules/Scenes/Assets/Scene.h"
#include "Modules/FileSystem/FileSystem.h"
#include "Modules/Rendering/Components/ShapeComponent.h"

#include "Modules/Rendering/ExternalAPI/VulkanAPI.h"
#include "API/Platform/gladAPI.h"

namespace Kargono::Utility
{
	static GLenum ShaderTypeFromExtension(std::string_view type)
	{
		if (type == ".kgshadervert")
		{
			return GL_VERTEX_SHADER;
		}
		if (type == ".kgshaderfrag")
		{
			return GL_FRAGMENT_SHADER;
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

namespace Kargono::Rendering
{
	void RendererInputSpec::ClearData()
	{
		m_Shader = nullptr;
		m_Texture = nullptr;
		if (m_ShapeComponent)
		{
			m_ShapeComponent->m_Texture = nullptr;
			m_ShapeComponent->m_Shader = nullptr;
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

	void Shader::Serialize(void* context)
	{

	}
	void Shader::Deserialize(void* context)
	{
		KG_ASSERT(context, "Context cannot be null");

		// Get asset context
		Assets::DeserializeAssetContext& assetContext = *(Assets::DeserializeAssetContext*)context;
		KG_ASSERT(assetContext.m_AssetMetadata);
		Assets::Metadata* metadata{ assetContext.m_AssetMetadata };
		KG_ASSERT(metadata);

		ShaderMetaData* shaderMetadata = metadata->GetSpecificMetaData<ShaderMetaData>();
		KG_ASSERT(shaderMetadata);

		std::unordered_map<GLenum, std::vector<uint32_t>> openGLSPIRV;
		std::array<FixedBufStr16, 2> shaderStageExtensions { k_IntermediateExtensions[1], k_IntermediateExtensions[2] };

		for (const FixedBufStr16& extension : shaderStageExtensions)
		{
			const std::filesystem::path fullPath = metadata->GetAssetFullIntermediatePath<Shader>(extension.StringView());
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
		m_VertexArray = VertexArray::Create();

		auto quadVertexBuffer = VertexBuffer::Create(k_MaxVertexBufferSize);
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
