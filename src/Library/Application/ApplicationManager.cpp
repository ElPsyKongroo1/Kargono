#include "Application.h"
#include "../Library.h"

/*============================================================================================================================================================================================
 *============================================================================================================================================================================================
 * Application Manager Class
 *============================================================================================================================================================================================
 *============================================================================================================================================================================================*/

 /*============================================================================================================================================================================================
  * Instantiate/Delete Application objects
  *============================================================================================================================================================================================*/
void ApplicationManager::CreateApplications()
{
	CreateDefaultApplication(defaultApplication);
	CreateApplication2D(application2D);

}

void ApplicationManager::DestroyApplications()
{
	DestroyApplication(defaultApplication);
	DestroyApplication(application2D);

}

void ApplicationManager::DestroyApplication(Application& application)
{
	application.programName = "";
	int GLFWVersionSize = sizeof(application.GLFWVersion) / sizeof(int);
	for (int i = 0; i < GLFWVersionSize; i++)
	{
		application.GLFWVersion[i] = 0;
	}
	application.screenDimension = glm::vec3();
	application.backgroundColor = glm::vec3();

	application.allSimpleObjects.clear();
	application.allLightSources.clear();
	application.currentInput = nullptr;
	application.currentCamera = nullptr;
	application.defaultMesh = nullptr;
	application.defaultModel = nullptr;
	application.defaultShader = nullptr;
}

/*============================================================================================================================================================================================
 * Initializes Application Objects with specified values
 *============================================================================================================================================================================================*/

void ApplicationManager::CreateDefaultApplication(Application& application)
{
	application.programName = "3D Renderer";
	application.GLFWVersion[0] = 4;
	application.GLFWVersion[1] = 6;
	application.screenDimension = glm::vec2(1920.0f, 1080.0f);
	application.backgroundColor = glm::vec3(0.0f, 0.0f, 0.0f);

	application.allSimpleObjects = std::vector<Object>();
	application.allLightSources = std::vector<LightSource*>();
	application.defaultInput = &Resources::inputManager.input3D;
	application.currentInput = application.defaultInput;
	application.currentCamera = &Resources::cameraManager.flyCamera;
	application.defaultModel = &Resources::modelManager.simpleBackpack;
	application.defaultMesh = &Resources::meshManager.cubeMesh;
	application.defaultShader = &Resources::shaderManager.lightingShader;
	application.currentWindow = nullptr;

}

void ApplicationManager::CreateApplication2D(Application& application)
{
	application.programName = "2D Renderer";
	application.GLFWVersion[0] = 4;
	application.GLFWVersion[1] = 6;
	application.screenDimension = glm::vec2(1920.0f, 1080.0f);
	application.backgroundColor = glm::vec3(0.0f, 0.11f, 0.1f);

	application.allSimpleObjects = std::vector<Object>();
	application.allLightSources = std::vector<LightSource*>();
	application.defaultInput = &Resources::inputManager.input2D;
	application.currentInput = application.defaultInput;
	application.defaultModel = &Resources::modelManager.simpleBackpack;
	application.currentCamera = &Resources::cameraManager.static2DCamera;
	application.defaultMesh = &Resources::meshManager.cubeMesh;
	application.defaultShader = &Resources::shaderManager.lightingShader;
	application.currentWindow = nullptr;

}