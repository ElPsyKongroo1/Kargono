/// @brief Includes the precompiled headers
#include "kgpch.h"

/// @brief Includes the Timer class from Kargono
#include "Kargono/Core/Timer.h"
/// @brief Includes the FileSystem utilities from Kargono
#include "Kargono/Core/FileSystem.h"
/// @brief Includes the openGL shader implementation
#include "API/OpenGL/OpenGLShader.h"

#include <glad/glad.h>
#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>


namespace API::OpenGL
{

/// @brief Constructor. @details Initializes the shader object with given name and shader binaries. @params name The name of the shader. @params shaderBinaries Map that contains shader binaries.
	OpenGLShader::OpenGLShader(const std::string& name, const std::unordered_map<GLenum, std::vector<uint32_t>>& shaderBinaries)
		: m_Name(name)
	{
		CreateProgram(shaderBinaries);
	}

/// @brief Destructor. @details Unloads this shader and remove it from the renderer.
	OpenGLShader::~OpenGLShader()
	{
		glDeleteProgram(m_RendererID);
	}


/// @brief Creates a shader program using shader binaries in SPIR-V format. @param openGLSPIRV Map that contains openGL SPIR-V binaries.
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
			KG_CORE_ERROR("Shader linking failed :\n{0}", infoLog.data());

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

/// @brief Binds current shader for rendering.
	void OpenGLShader::Bind() const
	{
		glUseProgram(m_RendererID);
	}

/// @brief Unbinds the shader.
	void OpenGLShader::Unbind() const
	{
		glUseProgram(0);
	}

/// @brief Sets an integer uniform value. @param name Name of the uniform. @param value The integer value to set.
	void OpenGLShader::SetIntUniform(const std::string& name, int value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1i(location, value);
	}

/// @brief Sets an integer array uniform value. @param name Name of the uniform. @param values The array of integers to set. @param count The number of elements in the array.
	void OpenGLShader::SetIntArrayUniform(const std::string& name, int* values, uint32_t count)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1iv(location, count, values);
	}

/// @brief Sets a floating point uniform value. @param name Name of the uniform. @param value The floating point value to set.
	void OpenGLShader::SetFloatUniform(const std::string& name, float value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1f(location, value);
	}

/// @brief Sets a 2D vector as a float uniform value. @param name Name of the uniform. @param value The 2D vector to set.
	void OpenGLShader::SetFloat2Uniform(const std::string& name, const Kargono::Math::vec2& value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform2f(location, value.x, value.y);
	}

/// @brief Sets a 3D vector as a float uniform value. @param name Name of the uniform. @param value The 3D vector to set.
	void OpenGLShader::SetFloat3Uniform(const std::string& name, const Kargono::Math::vec3& value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform3f(location, value.x, value.y, value.z);
	}

/// @brief Sets a 4D vector as a float uniform value. @param name Name of the uniform. @param value The 4D vector to set.
	void OpenGLShader::SetFloat4Uniform(const std::string& name, const Kargono::Math::vec4& value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform4f(location, value.x, value.y, value.z, value.w);
	}

/// @brief Sets a 3x3 matrix as a float uniform value. @param name Name of the uniform. @param value The 3x3 matrix to set.
	void OpenGLShader::SetMat3Uniform(const std::string& name, const Kargono::Math::mat3& value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(value));
	}

/// @brief Sets a 4x4 matrix as a float uniform value. @param name Name of the uniform. @param value The 4x4 matrix to set.
	void OpenGLShader::SetMat4Uniform(const std::string& name, const Kargono::Math::mat4& value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
	}

}
