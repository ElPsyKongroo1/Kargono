#pragma once
#include "../Includes.h"

/*============================================================================================================================================================================================
 * Shader Struct that represents indvidual shader object
 *============================================================================================================================================================================================*/

struct GLShader
{
public:
	enum ShaderType { LIGHTING, NOLIGHTING, LIGHTSOURCE };
public:
	unsigned int shaderID;
	ShaderType type;

public:
	GLShader() {};
	GLShader(ShaderType shaderType, const char* vertexPath, const char* fragmentPath);
	~GLShader() 
	{
		type = NOLIGHTING;
		shaderID = -1;
	}
public:
	void loadSource(const char* vertexPath, const char* fragmentPath);
	void checkCompileErrors(std::string type);
public:
	void useProgram();
	void setBool(const std::string& name, bool value) const;
	void setInt(const std::string& name, int value) const;
	void setFloat(const std::string& name, float value) const;
	void setVec3(const std::string& name, glm::vec3& value) const;
	void setVec4(const std::string& name, glm::vec4& value) const;
	void setMatrix4f(const std::string& name, glm::mat4& matrix);
};

/*============================================================================================================================================================================================
 * Shaders class that manages GLShader objects
 *============================================================================================================================================================================================*/

class ShaderManager
{
public:
	GLShader* defaultShader;
	GLShader* lightingShader;
	GLShader* lightSourceShader;
public: 
	ShaderManager() {};
public:
	void CreateShaders();
	void DestroyShaders();
	
};