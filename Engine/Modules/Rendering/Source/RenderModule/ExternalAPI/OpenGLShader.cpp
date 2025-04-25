#include "kgpch.h"

#include "Kargono/Utility/Timers.h"
#include "FileSystemModule/FileSystem.h"

#include "RenderModule/ExternalAPI/OpenGLShader.h"
#include "API/Platform/gladAPI.h"

#ifdef KG_RENDERER_OPENGL

namespace API::RenderingAPI
{

	OpenGLShader::OpenGLShader(std::string_view name, const std::unordered_map<GLenum, std::vector<uint32_t>>& shaderBinaries)
		: m_Name(name)
	{
		CreateProgram(shaderBinaries);
	}

	OpenGLShader::~OpenGLShader()
	{
		glDeleteProgram(m_RendererID);
	}


	void OpenGLShader::CreateProgram(const std::unordered_map<GLenum, std::vector<uint32_t>>& openGLSPIRV)
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
	}

	void OpenGLShader::Bind() const
	{
		glUseProgram(m_RendererID);
	}

	void OpenGLShader::Unbind() const
	{
		glUseProgram(0);
	}

	void OpenGLShader::SetIntUniform(const char* name, int value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name);
		glUniform1i(location, value);
	}

	void OpenGLShader::SetIntArrayUniform(const char* name, int* values, uint32_t count)
	{
		GLint location = glGetUniformLocation(m_RendererID, name);
		glUniform1iv(location, count, values);
	}

	void OpenGLShader::SetFloatUniform(const char* name, float value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name);
		glUniform1f(location, value);
	}

	void OpenGLShader::SetFloat2Uniform(const char* name, const Kargono::Math::vec2& value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name);
		glUniform2f(location, value.x, value.y);
	}

	void OpenGLShader::SetFloat3Uniform(const char* name, const Kargono::Math::vec3& value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name);
		glUniform3f(location, value.x, value.y, value.z);
	}

	void OpenGLShader::SetFloat4Uniform(const char* name, const Kargono::Math::vec4& value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name);
		glUniform4f(location, value.x, value.y, value.z, value.w);
	}

	void OpenGLShader::SetMat3Uniform(const char* name, const Kargono::Math::mat3& value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name);
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(value));
	}

	void OpenGLShader::SetMat4Uniform(const char* name, const Kargono::Math::mat4& value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name);
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
	}

}

#endif
