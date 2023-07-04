#include "Kargono/kgpch.h"
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
	defaultShader = new GLShader(GLShader::NOLIGHTING, "Resources/Default-Resources/Shaders/DefaultShader/shader.vs", "Resources/Default-Resources/Shaders/DefaultShader/shader.fs");
	lightingShader = new GLShader(GLShader::LIGHTING, "Resources/Default-Resources/Shaders/Lighting/shader.vs", "Resources/Default-Resources/Shaders/Lighting/shader.fs");
	lightSourceShader = new GLShader(GLShader::LIGHTSOURCE, "Resources/Default-Resources/Shaders/LightSource/shader.vs", "Resources/Default-Resources/Shaders/LightSource/shader.fs");
}

void ShaderManager::DestroyShaders() 
{
	delete defaultShader;
	delete lightingShader;
	delete lightSourceShader;
}


