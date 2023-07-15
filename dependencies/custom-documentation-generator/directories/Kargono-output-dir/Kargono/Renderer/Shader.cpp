/// @brief Include the precompiled header file for the Kargono game engine.
#include "Kargono/kgpch.h"
/// @brief Include the header file for the Shader class in the Renderer module of the Kargono game engine.
#include "Kargono/Renderer/Shader.h"

/// @brief Include the glad library for OpenGL functionality.
#include <glad/glad.h>

/// @brief Include the header file for the Core module of the Kargono game engine.
#include "Kargono/Core.h"
/// @brief Include the header file for the Log class in the Kargono game engine.
#include "Kargono/Log.h"

namespace Kargono
{
/// @brief Constructor for the Shader class that takes two string parameters as the vertex source and the fragment source.
	Shader::Shader(const std::string& vertexSrc, const std::string& fragmentSrc)
	{
		// Create an empty vertex shader handle
/// @brief Create an empty vertex shader handle.
		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

		// Send the vertex shader source code to GL
		// Note that std::string's .c_str is NULL character terminated.
		const GLchar* source = vertexSrc.c_str();
		glShaderSource(vertexShader, 1, &source, 0);

		// Compile the vertex shader
/// @brief Compile the vertex shader.
		glCompileShader(vertexShader);

		GLint isCompiled = 0;
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiled);
/// @brief Check if the vertex shader compilation failed.
		if (isCompiled == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);

			// The maxLength includes the NULL character
			std::vector<GLchar> infoLog(maxLength);
			glGetShaderInfoLog(vertexShader, maxLength, &maxLength, &infoLog[0]);

			// We don't need the shader anymore.
/// @brief Delete the vertex shader after compilation.
			glDeleteShader(vertexShader);

			// Use the infoLog as you see fit.
/// @brief Log the error message if the vertex shader compilation failed.
			KG_CORE_ERROR("{0}", infoLog.data());
/// @brief Assert if the vertex shader compilation failed.
			KG_CORE_ASSERT(false, "Vertex Shader Compilation Failure!");
			return;
		}

		// Create an empty fragment shader handle
/// @brief Create an empty fragment shader handle.
		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

		// Send the fragment shader source code to GL
		// Note that std::string's .c_str is NULL character terminated.
		source = fragmentSrc.c_str();
		glShaderSource(fragmentShader, 1, &source, 0);

		// Compile the fragment shader
/// @brief Compile the fragment shader.
		glCompileShader(fragmentShader);

		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompiled);
		if (isCompiled == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);

			// The maxLength includes the NULL character
			std::vector<GLchar> infoLog(maxLength);
			glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, &infoLog[0]);

			// We don't need the shader anymore.
/// @brief Delete the fragment shader after compilation.
			glDeleteShader(fragmentShader);
			// Either of them. Don't leak shaders.
			glDeleteShader(vertexShader);

			// Use the infoLog as you see fit.
			KG_CORE_ERROR("{0}", infoLog.data());
			KG_CORE_ASSERT(false, "Fragment Shader Compilation Failure!");
			return;
		}

		// Vertex and fragment shaders are successfully compiled.
		// Now time to link them together into a program.
		// Get a program object.
/// @brief Create a program object for the Shader class.
		m_RendererID = glCreateProgram();

		// Attach our shaders to our program
		glAttachShader(m_RendererID, vertexShader);
		glAttachShader(m_RendererID, fragmentShader);

		// Link our program
/// @brief Link the shaders and the program into a complete shader program.
		glLinkProgram(m_RendererID);

		// Note the different functions here: glGetProgram* instead of glGetShader*.
		GLint isLinked = 0;
		glGetProgramiv(m_RendererID, GL_LINK_STATUS, (int*)&isLinked);
/// @brief Check if the shader program linking failed.
		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetProgramiv(m_RendererID, GL_INFO_LOG_LENGTH, &maxLength);

			// The maxLength includes the NULL character
			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(m_RendererID, maxLength, &maxLength, &infoLog[0]);

			// We don't need the program anymore.
			glDeleteProgram(m_RendererID);
			// Don't leak shaders either.
			glDeleteShader(vertexShader);
			glDeleteShader(fragmentShader);

			// Use the infoLog as you see fit.
			KG_CORE_ERROR("{0}", infoLog.data());
			KG_CORE_ASSERT(false, "Shader Link Failure!");
			return;
			// In this simple program, we'll just leave
			return;
		}

		// Always detach shaders after a successful link.
		glDetachShader(m_RendererID, vertexShader);
		glDetachShader(m_RendererID, fragmentShader);
	}
/// @brief Destructor for the Shader class.
	Shader::~Shader()
	{
		glDeleteProgram(m_RendererID);
	}
/// @brief Bind the shader for rendering.
	void Shader::Bind() const
	{
		glUseProgram(m_RendererID);
	}
/// @brief Unbind the shader after rendering.
	void Shader::Unbind() const
	{
		glUseProgram(0);
	}
}
