#include "Shaders.h"

/*============================================================================================================================================================================================
 *============================================================================================================================================================================================
 * ShaderManager Class
 *============================================================================================================================================================================================
 *============================================================================================================================================================================================*/

 /*============================================================================================================================================================================================
  * Instantiate/Delete GLShader objects
  *============================================================================================================================================================================================*/

void ShaderManager::CreateShaders()
{
	CreateDefaultShader(defaultShader);
	CreateLightingShader(lightingShader);
	CreateLightSourceShader(lightSourceShader);
}

/*============================================================================================================================================================================================
 * Initializes GLShader Structs with specified values
 *============================================================================================================================================================================================*/

void ShaderManager::CreateDefaultShader(GLShader& shader)
{
	shader.type = GLShader::NOLIGHTING;
	loadSource("Resources/Shaders/DefaultShader/shader.vs", "Resources/Shaders/DefaultShader/shader.fs", shader);
}

void ShaderManager::CreateLightingShader(GLShader& shader)
{
	shader.type = GLShader::LIGHTING;
	loadSource("Resources/Shaders/Lighting/shader.vs", "Resources/Shaders/Lighting/shader.fs", shader);
}
void ShaderManager::CreateLightSourceShader(GLShader& shader)
{
	shader.type = GLShader::LIGHTSOURCE;
	loadSource("Resources/Shaders/LightSource/shader.vs", "Resources/Shaders/LightSource/shader.fs", shader);
}

/*============================================================================================================================================================================================
 * Internal Functionality
 *============================================================================================================================================================================================*/

void ShaderManager::loadSource(const char* vertexPath, const char* fragmentPath, GLShader& shader)
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
	checkCompileErrors(vertexShader, "VERTEX");
	unsigned int fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fShaderCode, NULL);
	glCompileShader(fragmentShader);
	checkCompileErrors(fragmentShader, "FRAGMENT");
	shader.shaderID = glCreateProgram();
	glAttachShader(shader.shaderID, vertexShader);
	glAttachShader(shader.shaderID, fragmentShader);
	glLinkProgram(shader.shaderID);
	checkCompileErrors(shader.shaderID, "PROGRAM");

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	// Register texture units with current shader
	glUseProgram(shader.shaderID);
	glUseProgram(0);
}

void ShaderManager::checkCompileErrors(unsigned int shader, std::string type)
{
	int success;
	char infoLog[1024];
	if (type != "PROGRAM")
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
	else
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
}
