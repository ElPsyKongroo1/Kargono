#include "Shaders.h"


/*============================================================================================================================================================================================
 *============================================================================================================================================================================================
 * GLShader Class
 *============================================================================================================================================================================================
 *============================================================================================================================================================================================*/

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
 * External Functionality
 *============================================================================================================================================================================================*/
