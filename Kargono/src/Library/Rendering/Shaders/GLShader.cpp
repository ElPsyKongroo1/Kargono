#include "Kargono/kgpch.h"
#include "Shaders.h"


/*============================================================================================================================================================================================
 *============================================================================================================================================================================================
 * GLShader Class
 *============================================================================================================================================================================================
 *============================================================================================================================================================================================*/

GLShader::GLShader(ShaderType shaderType, const char* vertexPath, const char* fragmentPath) 
{
	type = shaderType;
	loadSource(vertexPath, fragmentPath);

}

 /*============================================================================================================================================================================================
  * Use current Shader
  *============================================================================================================================================================================================*/
void GLShader::useProgram()
{
	glUseProgram(shaderID);
}

/*============================================================================================================================================================================================
 * Set Uniform data for Shader
 *============================================================================================================================================================================================*/

void GLShader::setBool(const std::string& name, bool value) const
{
	GLuint attributeLocation = glGetUniformLocation(shaderID, name.c_str());
	if (attributeLocation == -1)
	{
		std::cout << "ERROR::UNIFORM::BOOLEAN_LOCATION_NOT_FOUND: " << name << std::endl;
		throw std::runtime_error("Check Logs!");
	}
	glUniform1i(attributeLocation, (int)value);
}

void GLShader::setInt(const std::string& name, int value) const
{
	GLuint attributeLocation = glGetUniformLocation(shaderID, name.c_str());
	if (attributeLocation == -1)
	{
		std::cout << "ERROR::UNIFORM::INTEGER_LOCATION_NOT_FOUND: " << name << std::endl;
		throw std::runtime_error("Check Logs!");
	}
	glUniform1i(attributeLocation, value);
}

void GLShader::setFloat(const std::string& name, float value) const
{
	GLuint attributeLocation = glGetUniformLocation(shaderID, name.c_str());
	if (attributeLocation == -1)
	{
		std::cout << "ERROR::UNIFORM::ONE_FLOAT_LOCATION_NOT_FOUND: " << name << std::endl;
		throw std::runtime_error("Check Logs!");
	}
	glUniform1f(attributeLocation, value);
}

void GLShader::setVec3(const std::string& name, glm::vec3& value) const
{
	GLuint attributeLocation = glGetUniformLocation(shaderID, name.c_str());
	if (attributeLocation == -1)
	{
		std::cout << "ERROR::UNIFORM::THREE_FLOAT_LOCATION_NOT_FOUND: " << name << std::endl;
		throw std::runtime_error("Check Logs!");
	}
	glUniform3fv(attributeLocation, 1, &value[0]);
}

void GLShader::setVec4(const std::string& name, glm::vec4& value) const
{
	GLuint attributeLocation = glGetUniformLocation(shaderID, name.c_str());
	if (attributeLocation == -1)
	{
		std::cout << "ERROR::UNIFORM::FOUR_FLOAT_LOCATION_NOT_FOUND: " << name << std::endl;
		throw std::runtime_error("Check Logs!");
	}
	glUniform4fv(attributeLocation, 1, &value[0]);
}

void GLShader::setMatrix4f(const std::string& name, glm::mat4& matrix)
{
	GLuint attributeLocation = glGetUniformLocation(shaderID, name.c_str());
	if (attributeLocation == -1)
	{
		std::cout << "ERROR::UNIFORM::MATRIX_4F_LOCATION_NOT_FOUND: " << name << std::endl;
		throw std::runtime_error("Check Logs!");
	}
	glUniformMatrix4fv(attributeLocation, 1, GL_FALSE, glm::value_ptr(matrix));
}

/*============================================================================================================================================================================================
 * Internal Functionality
 *============================================================================================================================================================================================*/

void GLShader::loadSource(const char* vertexPath, const char* fragmentPath)
{
	// Load in shader data from file
	std::string vertexCode;
	std::string fragmentCode;
	std::ifstream vShaderFile;
	std::ifstream fShaderFile;
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		vShaderFile.open(vertexPath);
		fShaderFile.open(fragmentPath);
		std::stringstream vShaderStream, fShaderStream;
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();
		vShaderFile.close();
		fShaderFile.close();
		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
	}
	catch (std::ifstream::failure& e)
	{
		std::cout << "ERROR::SHADER:::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
	}
	const char* vShaderCode = vertexCode.c_str();
	const char* fShaderCode = fragmentCode.c_str();

	// Set up and link ShaderProgram
	unsigned int vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vShaderCode, NULL);
	glCompileShader(vertexShader);
	checkCompileErrors("VERTEX");
	unsigned int fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fShaderCode, NULL);
	glCompileShader(fragmentShader);
	checkCompileErrors("FRAGMENT");
	shaderID = glCreateProgram();
	glAttachShader(shaderID, vertexShader);
	glAttachShader(shaderID, fragmentShader);
	glLinkProgram(shaderID);
	checkCompileErrors("PROGRAM");

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	// Register texture units with current shader
	glUseProgram(shaderID);
	glUseProgram(0);
}

void GLShader::checkCompileErrors(std::string type)
{
	int success;
	char infoLog[1024];
	if (type != "PROGRAM")
	{
		glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shaderID, 1024, NULL, infoLog);
			std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
	else
	{
		glGetProgramiv(shaderID, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(shaderID, 1024, NULL, infoLog);
			std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
}